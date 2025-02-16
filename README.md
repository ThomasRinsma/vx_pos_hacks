
Instructions and scripts to help you compile and run custom code on certain Verifone devices.

## The vulnerability

Verifone VX POS devices normally require applications to be signed, and employ a secure boot process to ensure this. As found and documented by the POSWorld authors (link down below), the bootloader of VX and MX devices (called SBI) contains a vulnerability (CVE-2019-14715) that allows us to break secure boot and execute our own code in kernel mode. I exploit this to invoke some kernel logic that marks an executable file (called `H.OUT`, which we upload before) as authenticated, allowing us to run our own code.


### SBI

The vulnerability (feature?) is that when a certain key combination is held during boot (1+5+9), the SBI bootloader enters a special mode which accepts a download over RS232 through the XDL protocol. The same protocol used by the OS's download mode.

When a file with the right header is transmitted, the entire file's contents are copied to an arbitrary address in memory (specified in one of the header fields), without any signature verification. This allows us to overwrite SBI itself while it is running, and take full control.

### Bootstrapping the exploit

The challenge is that to bootstrap this process, we ideally need to know where in SBI the vulnerable `memcpy` occurs, such that we can copy our payload right after it, or at least somewhere in the control flow that occurs afterwards. Or alternatively, we can overwrite a bigger part of the loaded SBI code, including the currently running code. With this latter approach we don't have to worry about making sure that the payload's header executes as NOPs.

Either approach is easy if we have a dumped SBI binary, but what if we don't?

### Prompt mode

Well, it turns out (again, discovered by the POSWorld authors) that SBI contains a hidden "prompt" function which gives a shell over RS232 with various commands, including "DUMP" with which we can dump NAND flash contents, including the stored SBI code.

From the POSWorld paper (figure 51), we can see that at `0x189e48`, a patch is inserted that jumps to `prompt`, with the bytestring `03f021fe`. They do this with a full modified SBI image, but we can instead write just that instruction by setting the target address just right. Hence, a payload for `03_04` would be something like:
```
00000000: 8fc3 9ba1 0800 0000 0100 0000 0000 0000  ................
00000010: 0400 0000 0300 0000 189e 1800 0000 0000  ................
00000020: 0000 0000 0000 0000 0000 0000 0000 0000  ................
00000030: 0000 0000 0000 0000 0000 0000 03f0 21fe  ..............!.
00000040: 0000 0000                                ....
```

For `03_10`, it turns out that the prompt function moved a bit, but the memcpy stayed (roughly) in the same area as in `03_04`, so we only need to change the jump target slightly:
```
00000000: 8fc3 9ba1 0800 0000 0100 0000 0000 0000  ................
00000010: 0400 0000 0300 0000 189e 1800 0000 0000  ................
00000020: 0000 0000 0000 0000 0000 0000 0000 0000  ................
00000030: 0000 0000 0000 0000 0000 0000 03f0 e6ff  ................
00000040: 0000 0000                                ....
```
To figure this out I just did some rough "binary searching" of the jump target, where if you overshoot, you'll sometimes see some stray prints from various prompt functionality, whereas if you undershoot, nothing or unrelated text is printed. Tip: use radare2 for easy assembling:
```sh
> rasm2 -a arm -b 16 'bl 0x3c46' # 03_04
03f021fe
> rasm2 -a arm -b 16 'bl 0x3fd0' # 03_10
03f0e6ff
```

These two "prompt mode" payloads are stored in this repo under `toolchain/payloads`. Use it as follows (e.g. for `03_04`):

1. Unplug power of device first
2. `python toolchain/xdl.py -v toolchain/payloads/sbi_prompt_03_04.bin`
3. Now power the device while holding 1+5+9. The upload will succeed in a second or so.
4. Assuming the payload worked, use a tty client to talk to the prompt function: `picocom /dev/ttyUSB0 -b 115200`
5. Press enter, and you should see `====>`. Try e.g. `help`.

Future work: An idea for a general payload to dump any vulnerable SBI version would be to overwrite everything from a very low address (so including the `memcpy` implementation) with a giant nopsled, followed by a custom NAND-dumper. This should work on all vulnerable SBI versions and would not rely on the existence of `prompt`.

### Getting further control

Now that we've dumped SBI, we can easily make new payloads by making changes wherever we want, and uploading the entire patched SBI image exactly on top of the running one.

What I did to start with, is add a jump to `prompt` later in the boot process, just after SBI validates and decrypts Verix OS. I also slightly modified the `prompt` logic to add RAM dumping and writing commands. This allowed me to more easily debug the boot process and figure out the exact patches needed.

In the end, the exploit has several stages, confusingly involving both thumb (SBI) and ARM (Verix) code.
The file `toolchain/create_bypass_auth_sbi.py` applies these patches to a dumped SBI binary:

- Execute normally: load and authenticate the VXOS kernel image
- Insert call to `patch1()` before jumping into the loaded kernel @ `0x40000000`
- `patch1()`: insert a call to `patch2()` at `0x40000XXX` in the decompression routine, after decompression and before the jump into the decompressed code
- `patch2()`: insert a call to `auth_and_generate_sig_file()` (`4000ecd8` at QT000500) at the end of `initialize()`

This all assumes a file called `H.OUT` is on the device's storage. After booting using the above exploit, `H.OUT` will permamently have a flag set allowing it to be executed. See the "Toolchain" section below for info on how to build your own `H.OUT`. Game-of-life is given as an example.

More info about this exploit is discussed in [my talk on MCH2022](https://th0mas.nl/2022/07/18/porting-doom-to-a-payment-terminal/). I also show Doom and my own Gameboy emulator running.

## Running the exploit

This exploit needs to be customized for each combination of SBI version and OS version.
Also, your device may not be vulnerable at all, if it has a new enough bootloader version. The patch was apparently released somewhere in 2020.

The following pre-made payloads are provided at the moment:

- vx820:
	- OS: `QT000500`
	- SBI: `03_04` and `03_10`
- e355:
	- OS: `QTE50301`
	- SBI: `03_10`

To see your device's versions, navigate to "Software Versions" in the system menu.
SBI also specifies its version and build date over serial, when booted into download mode (see below).

### Notes on the e355

The e355 does not have an obvious RS232 port, but only a visible micro-USB connector.
Turns out, the USB data lines are used as 5V UART TX/RX when no power is applied.

```
DATA- -> TX from the device
DATA+ -> RX from the device
```

So instead of RS232, use an FTDI or a similar UART-to-USB adapter.

### Getting the device into a usable state

1. When booted and running an app, press 7+OK at the same time
2. Enter the system mode password (default **166831** or **2012166831**)

If the app blocks system mode you can wipe the device by triggering the tamper mode.
Just unscrew the back and put it back together. Note: this wipes the currently installed application(s).
You can then clear the "TAMPER MODE" as follows in the system mode menu :

1. Go to "Key Management / SOP"
2. Go to "IPP Key Load" (and enter the system mode password again if needed)
	- On the e355 I needed to enter the key `1004444` here, YMMV.
3. Press the "2" key.

Now go to "Edit Parameters" and set `*SMDL` to `1`. This will enable auto download mode upon every boot. This uses `/DEV/COM1`, the same as used by the bootloader.

### "Toolchain"

This "toolchain" is quite ad-hoc and developer-unfriendly but it may help you get started.
See `programs/` for sample projects.
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


## See also

- [POSWorld: Vulnerabilities within Ingenico Telium 2 and Verifone VX and MX series Point of Sales terminals](https://drive.google.com/file/d/1GqiFrt_61lqXXPjuj7O4PJx4tV7uaVWD/view)
- [My initial blogpost and talk on MCH2022](https://th0mas.nl/2022/07/18/porting-doom-to-a-payment-terminal/)
