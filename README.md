# Cloudcutter Lightleak

This is a somewhat universal way of exploiting [a vulnerability in Tuya Smart IoT products](https://rb9.nl/posts/2022-03-29-light-jailbreaking-exploiting-tuya-iot-devices/) mostly effecting Tuya devices from Pre FEb-2022 firmware. Further, lightleak only has payloads to target Bekin BK7231T and BK7231N chipset based devices.  ESP Devices and others are not currently supported. Ideally, it doesn't require to have firmware dumps (and "device profiles") prior to executing.  The goal of lightleak is to acquire a complete firmware dump that can then be submitted up to cloudcutter so we can develop a full exploit profile for a previously unknown configuration or you can try to do so yourself (details below).

A detailed writeup about how this works will probably be here at some point.

## Who this is NOT for

- Non Bekin Chipsets: Again, you must have a Bekin 7231T/N device for it to even be a possibility.  Check the known [NON-bekin firmware page](https://github.com/tuya-cloudcutter/tuya-cloudcutter/wiki/Known-non%E2%80%90Beken-firmware-versions), if yours is on that page there is a high likelihood it is not Bekin and not exploitable by lightleak.  You can also put your target device in AP pairing mode (slow blink) and then get the BSSID of the device using `nmcli dev wifi list` or `iw wlan0 scan` and checking that against: https://maclookup.app/.  If it comes back unknown try subtracting 0x02 from the first octet (ie E2:98:06:05:06:32 -> E0:98:06:05:06:32).  If it comes back anything other than `Tuya Smart Inc` then it is likely not a bekin chipset.

- Exploited Devices: Already exploitable device/firmware versions, if your target device is already in the [known working database](https://github.com/tuya-cloudcutter/tuya-cloudcutter.github.io/tree/master/devices).  you can likely jump right to cutting or flashing it using tuya-cloudcutter directly.  While lightleak would likely work on the device there is little reason to use lightleak as the work is already done.  Note, if your device is listed but not the specific firmware you may need lightleak to generate the dump (assuming it is still an exploitable chipset).

- Patched Firmware: This only works on devices that were compiled against firmware pre Feb-2022 and earlier.  If yours has newer firmware it cannot be exploited with lightleak.  Please check the [known patched page](https://github.com/tuya-cloudcutter/tuya-cloudcutter/wiki/Known-Patched-Firmware) as if it is on there it will _not_ be exploitable.

## Requirements

- You must have a spare device able to run custom firmware (ie ESP32/ESP8266/BK7231 or RTL8710B with LibreTiny).  We will be temporarily flashing Lightleak firmware onto it to assist in our exploiting of the target device.  Once done you are welcome to flash back to whatever firmware you like.

- An android phone - This is an android application you need an android phone and the knowledge to install custom APK's onto it

- A potentially vulnerable device - Again, see above for who this is not for, you are welcome to try this on any devices (and it is unlikely to cause any bricking) but won't be successful unless it is running a vulnerable firmware on the right Bekin chipsets.


## Preparing for First Use

### Android Device Configuration
Lightleak is used together with the [Cloudcutter Android](https://github.com/tuya-cloudcutter/cloudcutter-android) application. Currently, there's no other way to use it.  Download the APK from releases (or compile it yourself) and install it on your device (you should have the "Cloud Cutter" app show up. See [Install the app](https://github.com/tuya-cloudcutter/cloudcutter-android/blob/master/README.md) for details.

Permissions must be correctly configured:

- Go to `App Info` (Android settings), choose `Permissions` and grant the `Location` permission (it's required for Wi-Fi connecting and scanning). **The app won't ask for it yet, so you have to grant it manually.**
- Before running the process, ensure you have both Wi-Fi and Location enabled (otherwise it'll just fail silently, waiting forever and timing out).


###

### Custom AP Device Setup
You need a device (ESP32/ESP8266/BK7231 or RTL8710B with LibreTiny) to serve as a dummy Wi-Fi Access Point - hereinafter referred to as `CustomAP device` (ie a device previously exploited by cloudcutter). This will be configured and used by the Android app throughout the process. You need to flash the lightleak firmware temporarily onto the device for the app to use it, you can likely find a pre-compiled firmware you can directly flash onto it (via serial or OTA) in the [releases section](https://github.com/tuya-cloudcutter/lightleak/releases).


### Target device prep

**Important:** The target device must be completely unconfigured.  If the target device was used before, or paired to **any** Wi-Fi network using the official app (Tuya Smart/SmartLife or any 3rd party derivative) you MUST wipe it first.  If you are not sure (ie it was acquired second hand) even if can't connect to the device currently we highly recommend wiping the device. You may need to put it back in pairing mode, pair it with the Tuya app, and then wipe it:

- Open the Tuya app.
- Choose the target device in the app (make sure it's online).
- Click the pencil icon in the top-right corner.
- Choose `Remove Device` and click `Disconnect and wipe data`.
- The device should start blinking *quickly* and disappear from the official app.

---

## Performing Lightleak Dump

1. Plug in the CustomAP device and make sure it works (a `LightleakIdle` Wi-Fi network should be available - scan with your smartphone or laptop). Do NOT connect to the network manually.  Note: You must restart/reboot (cut the power wait a second and turn it back on) the CustomAP device any time you go to test a profile.

2. Open the app. Select one of the Lightleak payload profiles (depending on the CPU/configuration of your device). If you don't know the CPU you have, you can try all the profiles, one by one. Trying an incompatible profile will not brick the device; it will just freeze and reboot after ~60 seconds.  NOTE: Only lightleak profiles will work, classic profiles show up but cannot be used.

3. The app will ask you about the device state, you should always be starting from scratch and choosing `Unconfigured` trying to shortcut from one of the partial states is not recommended as even if it seems to work the device may not end up in the proper state to dump the firmware.

4. The app will next go through several steps, pay close attention to any instructions on screen.

- It will first connect to and configure the CustomAP device, make sure the device was fully rebooted prior to this step each time.

- **Note:** Android (10+) will probably ask you when first connecting to a network. There will be a dialog message with the network name. You can just click the name or press `Connect`.

- Look carefully at the message window at the bottom, and do what it says. It will tell you when it's time to reboot the target device into AP mode (slow blink mode).  This can usually be accomplished by either:
    - Toggling it off and on again 6 times, with around 1 second in between each toggle. If it's a light bulb, it will blink slowly. If it blinks quickly, power cycle it 3 more times.
    - Long pressing the power/reset button on the device until it starts fast-blinking, then releasing, and then holding the power/reset button again until the device starts slow-blinking.


5. If the process completes successfully, another window will open. Currently, it allows you to dump flash contents of the device.
- Pressing `Read flash` should download the entire 2 MiB of flash, saving it to the directory of choice. It should take around 30-40 seconds.  If it says "Can't Receive Packets" it may have been a temporary failure or the device may not be compatible with lightleak (including using the wrong chipset).  See "Failure or Trying Another Profile" below for next steps.

6. What to do next:
	- Grab the dump binary, using any decent file manager (like [X-plore File Manager](https://play.google.com/store/apps/details?id=com.lonelycatgames.Xplore)). It will be located somewhere in `Internal Storage/Android/data/io.github.cloudcutter/`. Alternatively you can connect the smartphone to your PC to get the file.
	- Either [post the file to tuya-cloudcutter issues](https://github.com/tuya-cloudcutter/tuya-cloudcutter/issues), or:
	- if you're feeling brave enough, [create the profile yourself](https://github.com/tuya-cloudcutter/tuya-cloudcutter/tree/main/profile-building).
	- After that's done, you can use tuya-cloudcutter to detach the device from cloud, or [flash custom firmware](https://github.com/tuya-cloudcutter/tuya-cloudcutter/blob/main/INSTRUCTIONS.md#flashing-custom-firmware).


## Failure or Trying Another Profile
- If the entire process fails at any point or you want to try another payload profile you need to start from scratch for the best chance of success.  Note, just because you get to the "Read Flash" stage it does not mean that lightleak will work on the device or it is of the correct chipset/firmware revision.  To try again:

    1. Reset/restart the CustomAP device (you don't need to re-flash the firmware just power cycle it)
	2. Restart the target device into fast blink pairing mode (as long as you made sure it was wiped initially you do not need to re-wipe it before running lightleak again just restart it)
	3. Go back to the start of the app try another profile (or same one again if it seems it may have been intermittent).



## Manual Hotspot Firmware Compiling

The CustomAP device runs a custom lightleak firmware we use as part of the process. While you can likely use the pre-compiled firmware versions found in the [releases section](https://github.com/tuya-cloudcutter/lightleak/releases).  The [`platformio-custom-ap`](https://github.com/tuya-cloudcutter/lightleak/tree/master/platformio-custom-ap) directory contains a PlatformIO project that can be compiled on any of the platforms mentioned above. You need to download this code, build it, and upload to your device of choice.
