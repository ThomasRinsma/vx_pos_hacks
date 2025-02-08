
Instructions and scripts to help you compile and run custom code on certain Verifone devices.

## Supported devices

This technique relies on a bootloader (SBI) exploit and needs to be customized for each model.
This means that your device may not be vulnerable if it has a new enough bootloader version.
Additionally, payloads have to be customized for each combination of {model, SBI version, OS version}. The following are provided at the moment:

- vx820:
	- OS: `QT000500`
	- SBI: `03_04` and `03_10`
- e355:
	- OS: `QTE50301`
	- SBI: `03_10`

To see your device's versions, navigate to "Software Versions" in the system menu.

## Instructions

### Getting the device into a usable state

1. When booted and running an app, press 7+OK at the same time
2. Enter the system mode password (default **166831** or **2012166831**)

If the app blocks system mode you can wipe the device by triggering the tamper mode.
Just unscrew the back and put it back together. Note: this wipes the currently installed application(s).
You can then clear the "TAMPER MODE" as follows in the system mode menu :

1. Go to "Key Management / SOP"
2. Go to "IPP Key Load" (and enter the system mode password again if needed)
3. Press the "2" key.

Now go to "Edit Parameters" and set `*SMDL` to `1`. This will enable auto download mode upon every boot. This uses `/DEV/COM1`, the same as used by the bootloader.


### "Toolchain"

This "toolchain" is quite ad-hoc and developer-unfriendly but it may help you get started.
See `toolchain/example` for sample projects.
The best way to get started for now is just to copy one of them.

Compiling and uploading an example (game of life):

1. Install the `arm-none-eabi` toolchain and `python-pyserial`.
2. `cd programs/gol`
3. Customize variables in `Makefile` according to your situation (device and TTY)
4. `make`
5. `make upload`

Known problems/limitations:

- Only `.text` and `.rodata` segments work for now
- Only a subset of libc is implemented (see `programs/*/lib.c`)
- Getting floats to work is a pain, I believe I hacked something together with a softfloat library but YMMV.

## Technical details

TODO (link to further info on a future blog post)

## See also

- [POSWorld: Vulnerabilities within Ingenico Telium 2 and Verifone VX and MX series Point of Sales terminals](https://drive.google.com/file/d/1GqiFrt_61lqXXPjuj7O4PJx4tV7uaVWD/view)
- [My initial blogpost and talk on MCH2022](https://th0mas.nl/2022/07/18/porting-doom-to-a-payment-terminal/)
