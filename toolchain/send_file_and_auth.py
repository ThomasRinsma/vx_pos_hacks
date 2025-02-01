#!/usr/bin/python

import sys
from xdl import XDL
import time
import argparse
import os

payload_dir = os.path.dirname(os.path.realpath(__file__)) + "/payloads"


parser = argparse.ArgumentParser()
parser.add_argument('--device', '-d', type=str, help='TTY device', default='/dev/ttyUSB0')
parser.add_argument('--os_version', '-o', type=str, help='OS version')
parser.add_argument('--sbi_version', '-s', type=str, help='SBI version')
parser.add_argument('--file', '-f', type=str, help='file', default='H.OUT')
args = parser.parse_args()

print(f"[*] Trying to upload, reboot device now...")

xdl = XDL(port=args.device)

# VFI,VX820,XDL,F,,,QT000500,1000,32768,72286424
os_info_str = xdl.connect()

xdl.set_config_var("*GO", os.path.basename(args.file))
xdl.set_config_var("*SMDL", "1") # should already be set, but useful for later
xdl.send_file(args.file)
xdl.stop()

print("\n[*] Shut the device down, press enter, then hold 1+5+9 while powering up...")
input()

xdl.connect()
xdl.send_file(f"{payload_dir}/sbi_auth_{args.os_version}_{args.sbi_version}.bin")
xdl.stop()

# press enter after a sec
time.sleep(1)
xdl.ser.write(b"\r\n")

print("\n[*] Done. Now reboot the device again.")
