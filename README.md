# Perseverance Space Rover Firmware

# Building

To build the Rover firmware, you'll need the ESP-IDF. You can find the getting
started guide [here](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/).

The production firmware is built using IDF
version [5.1.1](https://github.com/espressif/esp-idf/releases/tag/v5.1.1).

In the root directory of the project:

**To build the firmware** run ```idf.py  build```

**To upload the firmware to the device** run ```idf.py -p <PORT> flash```.
Replace `<PORT>` with the port the Rover is attached to, for ex. ```COM6``` or
```/dev/ttyACM0```.


# Restoring the stock firmware

To restore the stock firmware, you can download the prebuilt binary on
the [releases page](https://github.com/CircuitMess/Perse_Rover-Firmware/releases) of this repository
and flash it manually using esptool:

```shell
esptool -c esp32s3 -b 921600 -p <PORT> write_flash 0 Rover-Firmware.bin
```

Alternatively, you can also do so using [CircuitBlocks](https://code.circuitmess.com/) by
logging in, clicking the "Restore Firmware" button in the top-right corner, and following the
on-screen instructions.