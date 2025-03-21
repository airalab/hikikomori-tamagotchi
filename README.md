# Hikikomory Tamagotchi Dev Kit

This repository contains several firmware examples for the Hikikomory Tamagotchi Dev Kit, demonstrating how the ESP device can communicate with the Robonomics Network. The available examples include:

1. [Example 1](./example1_wifi/README.md) - Connect to WiFi
2. [Example 2](./example2_datalog/README.md) - Send Datalog Transaction
3. [Example 3](./example3_press_counter/README.md) - Button Press Counter

You can either flash the pre-built binaries provided in the builds folder within each example or customize the firmware by building it from source.

## Flashing the Prebuilt Binaries

To flash your Hikikomory ESP32C6 device, use the [Web Flasher Tool](https://espressif.github.io/esptool-js/).

1. **Connect the Device**  
   Plug the Hikikomory device into your computer via USB and click the `Connect` button.  

   ![esptool1](./images/esptool1.png)

2. **Select the Device**  
   Choose the ESP device from the list. It should appear similar to the example below.  

   ![esptool2](./images/esptool2.png)

3. **Upload Firmware**  
   Upload the files from the [builds](./builds) folder. For each file, set the `Flash Address` as shown in the image below. Once done, press `Program` and wait for the flashing process to complete.  

   ![esptool3](./images/esptool3.png)

## Build from Source

### Requirements

* ESP-IDF v5.1.4 (installation instrutions are [here](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html#installation))

### Build

Clone the repository with submodules

```bash
git clone --recurse-submodules https://github.com/airalab/hikikomori-tamagotchi.git
```

Go to the example directory

```bash
cd cd hikikomori-tamagotchi/example1_wifi
```
Connect the device to computer via USB and run this command (For Linux and Mac) to build and flash firmware

```bash
idf.py -p <PORT> build flash
```
### Usefull commands

To see the logs run

```bash
idf.py -p <PORT> monitor
```

You can erase the flash (to delete saved credentials for example) with this command

```bash
idf.py -p <PORT> erase-flash
```