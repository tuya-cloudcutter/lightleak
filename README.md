# Cloudcutter Lightleak

This is a somewhat universal way of exploiting [a vulnerability in Tuya Smart IoT products](https://rb9.nl/posts/2022-03-29-light-jailbreaking-exploiting-tuya-iot-devices/). Ideally, it doesn't require to have firmware dumps (and "device profiles") prior to executing.

A detailed writeup about how this works will probably be here at some point.

## How to use it

Lightleak is used together with the [Cloudcutter Android](https://github.com/tuya-cloudcutter/cloudcutter-android) application. Currently, there's no other way to use it.

Additionally, you need a device (ESP32/ESP8266/BK7231 with Tasamota/ESPHome OR RTL8710B with LibreTuya) to serve as a dummy Wi-Fi Access Point - hereinafter referred to as `CustomAP device`. This will be configured and used by the Android app throughout the process. The [`platformio-custom-ap`](https://github.com/tuya-cloudcutter/lightleak/tree/master/platformio-custom-ap) directory contains a PlatformIO project that can be compiled on any of the platforms mentioned above. You need to download this code, build it, and upload to your device of choice - please note that the device you choose WILL NOT be OTA flashable after this. It's advised to get a cheap ESP32 chipset with USB before proceeding.  

### Compiling & Installing CustomAP firmware

The easiest way to compile the CustomAP firmware is to install visual studio code and install the PlatformIO extension. You can find full instructions here: https://platformio.org/platformio-ide

Once installed, open the IDE and select the chip you want to build the custom firmware for from the IDE menu - it should automatically happen. Once, built, the firmware will appear in your local directory, such as 'lightleak/platformio-custom-ap/.pio/build/esp8266/firmware.bin` 

If you do not have an idle ESP chipset, you can temporarily change an ESPHome device by configuring the webserver component:

```
web_server:
  port: 80
```

This will then allow you to manually upload the CustomAP firmware. 

Once installed, check that the LightleakIdle AP is visible before proceeding but do not connect to it. 

###

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
- It may be helpful to "Forget" any local wifi networks. Your phone may try to join those while running the app, causing unexepected failures. You can add them back afterwards. 
- Open the app. Select one of the Lightleak profiles (depending on the CPU of your device). If you don't know the CPU you have, you can try all the profiles, one by one. Trying an incompatible profile will not brick the device; it will just freeze and reboot after ~60 seconds.
- Read the message about choosing device state.
	- If it's the first time exploiting a particular device, choose `Unconfigured`. Other options are too hard to explain here, so just don't use them please.
- It will first connect to and configure the CustomAP device.
- **Note 1:** Android (10+) will probably ask you when first connecting to a network. There will be a dialog message with the network name. You can just click the name or press `Connect`.
- **Note 2:** if the entire process fails at any point, please reset/unplug the CustomAP device before trying it again
- **Note 3:** If you are having issues joining the Smart Device AP during the process, check if the wifi network shows "Access point is temporarily full" - if so, cycle the smart device. If it starts in slow-blink, the app should proceed. Otherwise start over. 
- Look carefully at the message window at the bottom, and do what it says. It will tell you when it's time to reboot the target device into AP mode.
- If the process completes successfully, another window will open. Currently, it allows you to dump flash contents of the device.
- Pressing `Read flash` should download the entire 2 MiB of flash, saving it to the directory of choice. It should take around 30-40 seconds.
- What to do next:
	- Grab the dump binary, using any decent file manager (like [X-plore File Manager](https://play.google.com/store/apps/details?id=com.lonelycatgames.Xplore)). It will be located somewhere in `Internal Storage/Android/data/io.github.cloudcutter/`. Alternatively you can connect the smartphone to your PC to get the file.
	- Either [post the file to tuya-cloudcutter issues](https://github.com/tuya-cloudcutter/tuya-cloudcutter/issues), or:
	- if you're feeling brave enough, [create the profile yourself](https://github.com/tuya-cloudcutter/tuya-cloudcutter/tree/main/profile-building).
	- After that's done, you can use tuya-cloudcutter to detach the device from cloud, or [flash custom firmware](https://github.com/tuya-cloudcutter/tuya-cloudcutter/blob/main/INSTRUCTIONS.md#flashing-custom-firmware).


## Restoring CustomAP device

TODO
