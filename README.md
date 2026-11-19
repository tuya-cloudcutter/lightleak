# Cloudcutter Lightleak

This is a somewhat universal way of exploiting [a vulnerability in Tuya Smart IoT products](https://rb9.nl/posts/2022-03-29-light-jailbreaking-exploiting-tuya-iot-devices/). Ideally, it doesn't require to have firmware dumps (and "device profiles") prior to executing.

A detailed writeup about how this works is available: [read it here](https://kuba.szczodrzynski.pl/posts/writeups/cloudcutter-lightleak/).

## Builds in this fork

Both workflows run on relevant pushes to `master`, pull requests, and manual dispatches in [GitHub Actions](https://github.com/seb5594/lightleak/actions).

- **Build payload binaries** provides the `lightleak-bins` artifact: four stager variants, two proper payloads, four disassemblies, and `SHA256SUMS`. The build checks the complete artifact set, stager CRCs, and proper payload decryption against the compiler output. Fork builds do not require the upstream project's Pages SSH key.
- **Push (CustomAP)** builds ESP32-C3, ESP32, ESP8266, Sonoff Basic, BK7231T, BK7231N, and RTL8710BN. Successful `master` builds also publish the firmware to this fork's [releases](https://github.com/seb5594/lightleak/releases). Pull requests only build and upload artifacts.

For local payload builds, install Python 3.10+, the GNU Arm Embedded toolchain (CI uses `7-2017-q4`), and `python -m pip install -r requirements-build.txt`. Run these commands from the repository root in Bash:

```sh
./build-stager.sh bk7231t 0x1C5AC0 standard
./build-stager.sh bk7231n 0x1B5AC0 standard
./build-stager.sh bk7231n 0x1B5AC0 xor
./build-stager.sh bk7231n 0x1B5AC0 ip
./build-proper.sh bk7231-proper-v1 bk7231t-proper-v1 BK7231T
./build-proper.sh bk7231-proper-v1 bk7231n-proper-v1 BK7231N
python tests/verify-payloads.py
```

## How to use it

Lightleak is used together with the [Cloudcutter Android](https://github.com/tuya-cloudcutter/cloudcutter-android) application. Currently, there's no other way to use it.

Additionally, you need a device (ESP32/ESP8266/BK7231 or RTL8710B with LibreTiny) to serve as a dummy Wi-Fi Access Point - hereinafter referred to as `CustomAP device`. This will be configured and used by the Android app throughout the process. The [`platformio-custom-ap`](https://github.com/tuya-cloudcutter/lightleak/tree/master/platformio-custom-ap) directory contains a PlatformIO project that can be compiled on any of the platforms mentioned above. You need to download this code, build it, and upload to your device of choice.

Plug in the CustomAP device and make sure it works (a `LightleakIdle` Wi-Fi network should be available - scan with your smartphone or laptop). Do not connect to the network manually.

**Important:** if the target device was used before, or paired to **any** Wi-Fi network using the official app (Tuya Smart/SmartLife):
- Open the Tuya app.
- Choose the target device in the app (make sure it's online).
- Click the pencil icon in the top-right corner.
- Choose `Remove Device` and click `Disconnect and wipe data`.
- The device should start blinking *quickly* and disappear from the official app.

---

- [Install the app](https://github.com/tuya-cloudcutter/cloudcutter-android/blob/master/README.md).
- Go to `App Info` (Android settings), choose `Permissions` and grant the `Location` permission (it's required for Wi-Fi connecting and scanning). **The app won't ask for it yet, so you have to grant it manually.**
- Before running the process, ensure you have both Wi-Fi and Location enabled (otherwise it'll just fail silently, waiting forever and timing out).
- Open the app. Select one of the Lightleak profiles (depending on the CPU of your device). If you don't know the CPU you have, you can try all the profiles, one by one. Trying an incompatible profile will not brick the device; it will just freeze and reboot after ~60 seconds.
- Read the message about choosing device state.
	- If it's the first time exploiting a particular device, choose `Unconfigured`. Other options are too hard to explain here, so just don't use them please.
- It will first connect to and configure the CustomAP device.
- **Note 1:** Android (10+) will probably ask you when first connecting to a network. There will be a dialog message with the network name. You can just click the name or press `Connect`.
- **Note 2:** if the entire process fails at any point, please reset the CustomAP device before trying it again.
- Look carefully at the message window at the bottom, and do what it says. It will tell you when it's time to reboot the target device into AP mode.
- If the process completes successfully, another window will open. Currently, it allows you to dump flash contents of the device.
- Pressing `Read flash` should download the entire 2 MiB of flash, saving it to the directory of choice. It should take around 30-40 seconds.
- What to do next:
	- Grab the dump binary, using any decent file manager (like [X-plore File Manager](https://play.google.com/store/apps/details?id=com.lonelycatgames.Xplore)). It will be located somewhere in `Internal Storage/Android/data/io.github.cloudcutter/`. Alternatively you can connect the smartphone to your PC to get the file.
	- Either [post the file to tuya-cloudcutter issues](https://github.com/tuya-cloudcutter/tuya-cloudcutter/issues), or:
	- if you're feeling brave enough, [create the profile yourself](https://github.com/tuya-cloudcutter/tuya-cloudcutter/tree/main/profile-building).
	- After that's done, you can use tuya-cloudcutter to detach the device from cloud, or [flash custom firmware](https://github.com/tuya-cloudcutter/tuya-cloudcutter/blob/main/INSTRUCTIONS.md#flashing-custom-firmware).
