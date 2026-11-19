"""Check the complete payload artifact set and encrypted binary integrity."""

from pathlib import Path

from ltchiptool.soc.bk72xx.util import BekenBinary
from ltchiptool.util.crc16 import CRC16


def require(condition, message):
    if not condition:
        raise ValueError(message)


def main():
    site = Path("site")
    build = Path("build")
    stagers = {
        "bk7231t": ("standard",),
        "bk7231n": ("standard", "xor", "ip"),
    }
    expected = set()
    bk = BekenBinary("510fb093a3cbeadc5993a17ec7adeb03")
    crc = CRC16(CRC16.CMS)

    # Published CRC-16/CMS check value, independent of the generated payloads.
    require(crc.calc(b"123456789") == 0xAEE7, "CRC-16/CMS check failed")
    for cpu, modes in stagers.items():
        base = f"{cpu}-stager-v1"
        raw = (build / f"{base}.bin").read_bytes()
        require(0 < len(raw) <= 56, f"Invalid stager code size: {base}")
        expected.add(f"{base}.lst")
        addr = 0x1C5AC0 if cpu == "bk7231t" else 0x1B5AC0
        padded = raw.ljust(56, b"\xaa")
        encrypted = b"".join(bk.crypt(addr, padded))
        for mode in modes:
            name = f"{base}-{mode}.bin"
            expected.add(name)
            data = (site / name).read_bytes()
            require(len(data) == 62, f"Invalid stager size: {name}")
            require(b"\x00" not in data, f"Null byte in stager: {name}")
            require(data[:2] == b"11", f"Invalid stager prefix: {name}")
            blocks = data[2:]
            if mode == "xor":
                key = b"\x55\xaa\x5a\x55\xa5\x4d\x63\x7f"
                blocks = bytes(b ^ key[(0x6C + i) % 8] for i, b in enumerate(blocks))
            require(blocks[:32] == encrypted[:32], f"First stager block differs: {name}")
            require(blocks[34:58] == encrypted[32:], f"Last stager block differs: {name}")
            require(int.from_bytes(blocks[32:34], "big") == crc.calc(blocks[:32]), f"Invalid stager CRC: {name}")
            suffix, final_crc = {
                "standard": (b"\x00\x00\x00\xff\xff\xff", 0xFFFF),
                "ip": (b"\x00\x00\x00\x00\x31\x30", 0x2E30),
                "xor": (b"\x55\xaa\x5a\xff\xff\xff", 0xFFFF),
            }[mode]
            require(crc.calc(blocks[34:] + suffix) == final_crc, f"Invalid final stager CRC: {name}")

    for cpu in stagers:
        base = f"{cpu}-proper-v1"
        expected.update((f"{base}.bin", f"{base}.lst"))
        raw = (build / f"{base}.bin").read_bytes()
        data = (site / f"{base}.bin").read_bytes()
        require(bool(raw), f"Empty proper payload: {base}")
        require(len(data) == ((len(raw) + 31) // 32) * 34, f"Invalid proper size: {base}")
        # uncrc checks every block before decrypting and comparing against GCC's output.
        decrypted = b"".join(bk.crypt(0x120000, bk.uncrc(data)))
        require(decrypted[:len(raw)] == raw, f"Proper payload round trip failed: {base}")

    actual = {p.name for p in site.iterdir() if p.suffix in (".bin", ".lst")}
    require(actual == expected, f"Unexpected artifact set: missing={expected - actual}, extra={actual - expected}")
    for name in expected:
        require((site / name).stat().st_size > 0, f"Empty artifact: {name}")
    print("Verified 6 payload binaries, 4 disassemblies, CRCs and encryption round trips.")


if __name__ == "__main__":
    main()
