# Copyright (c) Kuba Szczodrzyński 2022-09-23.

import sys
from subprocess import PIPE, Popen

from ltchiptool.soc.bk72xx.util import BekenBinary
from ltchiptool.util import CRC16
from ltchiptool.util.intbin import inttobe16

if len(sys.argv) != 4:
    print(f"usage: {sys.argv[0]} <mode> <input> <output>")
    exit(2)

mode = sys.argv[1]
bk = BekenBinary("510fb093a3cbeadc5993a17ec7adeb03")
xor_key = b"\x55\xaa\x5a\x55\xa5\x4d\x63\x7f"


def find_null(data, source):
    try:
        i = data.index(b"\x00")
        print(f"Found null byte at {hex(i)}: ")
        print(source.hex(" ", -1))
        print(" " * ((i * 3) + 1) + "^")
        exit(1)
    except ValueError:
        pass


def cmd(*args: str) -> str:
    p = Popen(args, stdout=PIPE)
    p.wait()
    return p.stdout.read().decode().strip()


def xor_encode(var, offs):
    key_offs = ((offs - 1) % 8) + 1
    key = xor_key[key_offs:] + xor_key[:key_offs]
    return bytes(a ^ key[i % 8] for i, a in enumerate(var))


with open(sys.argv[2], "rb") as f:
    code = f.read()

if len(code) > 56:
    print(f"Code too long! {len(code)} bytes > 56 bytes")
    exit(1)

print(f"Code length: {len(code)} bytes")

print(code.hex(" ", -1))

while len(code) < 56:
    code += b"\xaa"

blocks: list[dict] = []

if mode == "bk7231t-standard":
    addr = 0x1C5AC0
    blocks += [
        dict(
            addr=addr + 0x00,
            code=code[0:32],  # 32 bytes
            pre_block=b"11",
        ),
        dict(
            addr=addr + 0x20,
            code=code[32:56],  # 24 bytes
            post_code=b"\x00\x00\x00\xff\xff\xff",
            # reveng -m crc-16/cms -i 0 -v 000000ffffffffff
            # (passwd. last 2 bytes + null term. + rest of block + expected CRC)
            revcrc=0x7ADF,
        ),
    ]
elif mode == "bk7231n-ip":
    # BK7231N
    addr = 0x1B5AC0
    blocks += [
        dict(
            addr=addr + 0x00,
            code=code[0:32],  # 32 bytes
            pre_block=b"11",
        ),
        dict(
            addr=addr + 0x20,
            code=code[32:56],  # 24 bytes
            post_code=b"\x00\x00\x00\x00\x31\x30",  # beginning of IP (10.0.0.x)
            # reveng -m crc-16/cms -i 0 -v 0000000031302e30
            # (passwd. last 2 bytes + null term. + rest of block + expected CRC)
            revcrc=0x33B3,
        ),
    ]
elif mode == "bk7231n-xor":
    # BK7231N
    addr = 0x1B5AC0
    blocks += [
        dict(
            addr=addr + 0x00,
            code=code[0:32],  # 32 bytes
            pre_block=b"11",
            xor=True,
            xor_offs=0x6C,
        ),
        dict(
            addr=addr + 0x20,
            code=code[32:56],  # 24 bytes
            post_code=b"\x55\xaa\x5a\xff\xff\xff",
            # reveng -m crc-16/cms -i 0 -v 55aa5affffffffff
            # (passwd. last 2 bytes + null term. + rest of block + expected CRC) - XOR-encoded
            revcrc=0x67EC,
            xor=True,
            xor_offs=0x8E,
        ),
    ]
else:
    print(f"Unknown mode: {mode}")
    exit(1)

data = b""

for i, block in enumerate(blocks):
    xor = block.get("xor", False)

    # encrypt code and find null bytes
    block["crypt"] = b"".join(bk.crypt(block["addr"], block["code"]))
    find_null(block["crypt"], block["code"])

    # add pre-block padding
    data += block.get("pre_block", b"")
    # add pre-code padding
    block["crypt"] = block.get("pre_code", b"") + block["crypt"]

    # calculate "fix" to spoof CRC
    if "revcrc" in block:
        crc = CRC16.CMS.calc(block["crypt"])
        fix = inttobe16(crc ^ block["revcrc"])
        print(f"Block {i} fix-CRC: {fix.hex()}")
        block["crypt"] += fix
        find_null(fix, fix)

    # store entire block
    block_strip = block["crypt"]
    if xor:
        block_strip = xor_encode(block_strip, block["xor_offs"])
    data += block_strip
    # add post-code padding
    block["crypt"] = block["crypt"] + block.get("post_code", b"")
    # add post-block padding
    data += block.get("post_block", b"")

    # calculate actual CRC
    block["crc"] = inttobe16(CRC16.CMS.calc(block["crypt"]))
    print(f"Block {i} CRC: {block['crc'].hex()}")
    find_null(block["crc"], block["crc"])
    # store block CRC for full blocks
    if len(block_strip) == 32:
        if xor:
            block["crc"] = xor_encode(block["crc"], block["xor_offs"] + 0x20)
            print(f"Block {i} CRC (XOR): {block['crc'].hex()}")
        data += block["crc"]

print(f"Output ({len(data)} bytes):")
print(data.hex(" ", -1))
find_null(data, data)

with open(sys.argv[3], "wb") as f:
    f.write(data)
