import serial
import time
import struct
import sys
import os
import math
import argparse

class XDL:
	def __init__(self, port="/dev/ttyUSB0", baud=115200, log=True):
		self.ser = serial.Serial(port, baud, timeout=0)
		self.connected = False
		self.log = log
		self.__log(f"[*] Opened {port}:{baud}")

	def __log(self, s):
		if self.log:
			print(s)


	# Calculated checksum from (after) 02, up to (including 03)
	def __checksum(buf, check=0x00):
		for x in buf:
			x ^= check >> 8
			x ^= x >> 4
			x ^= x >> 2
			x ^= x >> 1
			check = ((check << 8) & 0xFFFF) ^ ((x << 0xf) & 0xFFFF) ^ ((x << 2) & 0xFFFF) ^ (x & 0xFFFF)
		return check

	def __crc16_buypass(data):
		table = [ 
			0x0000, 0x8005, 0x800f, 0x000a, 0x801b, 0x001e, 0x0014, 0x8011,
			0x8033, 0x0036, 0x003c, 0x8039, 0x0028, 0x802d, 0x8027, 0x0022,
			0x8063, 0x0066, 0x006c, 0x8069, 0x0078, 0x807d, 0x8077, 0x0072,
			0x0050, 0x8055, 0x805f, 0x005a, 0x804b, 0x004e, 0x0044, 0x8041,
			0x80c3, 0x00c6, 0x00cc, 0x80c9, 0x00d8, 0x80dd, 0x80d7, 0x00d2,
			0x00f0, 0x80f5, 0x80ff, 0x00fa, 0x80eb, 0x00ee, 0x00e4, 0x80e1,
			0x00a0, 0x80a5, 0x80af, 0x00aa, 0x80bb, 0x00be, 0x00b4, 0x80b1,
			0x8093, 0x0096, 0x009c, 0x8099, 0x0088, 0x808d, 0x8087, 0x0082,
			0x8183, 0x0186, 0x018c, 0x8189, 0x0198, 0x819d, 0x8197, 0x0192,
			0x01b0, 0x81b5, 0x81bf, 0x01ba, 0x81ab, 0x01ae, 0x01a4, 0x81a1,
			0x01e0, 0x81e5, 0x81ef, 0x01ea, 0x81fb, 0x01fe, 0x01f4, 0x81f1,
			0x81d3, 0x01d6, 0x01dc, 0x81d9, 0x01c8, 0x81cd, 0x81c7, 0x01c2,
			0x0140, 0x8145, 0x814f, 0x014a, 0x815b, 0x015e, 0x0154, 0x8151,
			0x8173, 0x0176, 0x017c, 0x8179, 0x0168, 0x816d, 0x8167, 0x0162,
			0x8123, 0x0126, 0x012c, 0x8129, 0x0138, 0x813d, 0x8137, 0x0132,
			0x0110, 0x8115, 0x811f, 0x011a, 0x810b, 0x010e, 0x0104, 0x8101,
			0x8303, 0x0306, 0x030c, 0x8309, 0x0318, 0x831d, 0x8317, 0x0312,
			0x0330, 0x8335, 0x833f, 0x033a, 0x832b, 0x032e, 0x0324, 0x8321,
			0x0360, 0x8365, 0x836f, 0x036a, 0x837b, 0x037e, 0x0374, 0x8371,
			0x8353, 0x0356, 0x035c, 0x8359, 0x0348, 0x834d, 0x8347, 0x0342,
			0x03c0, 0x83c5, 0x83cf, 0x03ca, 0x83db, 0x03de, 0x03d4, 0x83d1,
			0x83f3, 0x03f6, 0x03fc, 0x83f9, 0x03e8, 0x83ed, 0x83e7, 0x03e2,
			0x83a3, 0x03a6, 0x03ac, 0x83a9, 0x03b8, 0x83bd, 0x83b7, 0x03b2,
			0x0390, 0x8395, 0x839f, 0x039a, 0x838b, 0x038e, 0x0384, 0x8381,
			0x0280, 0x8285, 0x828f, 0x028a, 0x829b, 0x029e, 0x0294, 0x8291,
			0x82b3, 0x02b6, 0x02bc, 0x82b9, 0x02a8, 0x82ad, 0x82a7, 0x02a2,
			0x82e3, 0x02e6, 0x02ec, 0x82e9, 0x02f8, 0x82fd, 0x82f7, 0x02f2,
			0x02d0, 0x82d5, 0x82df, 0x02da, 0x82cb, 0x02ce, 0x02c4, 0x82c1,
			0x8243, 0x0246, 0x024c, 0x8249, 0x0258, 0x825d, 0x8257, 0x0252,
			0x0270, 0x8275, 0x827f, 0x027a, 0x826b, 0x026e, 0x0264, 0x8261,
			0x0220, 0x8225, 0x822f, 0x022a, 0x823b, 0x023e, 0x0234, 0x8231,
			0x8213, 0x0216, 0x021c, 0x8219, 0x0208, 0x820d, 0x8207, 0x0202,
		]
		
		crc = 0x0000
		for byte in data:
			crc = (crc << 8) ^ table[(crc >> 8) ^ byte]
			crc &= 0xFFFF
		return crc

	# basically re-implementing pySerial's blocking read(), but with protocol awareness
	# (our underlying `self.ser` is non-blocking (timeout=0))
	def __get_response(self, timeout=0.2):
		time_start = time.time()

		# Keep trying to get a complete command/response until timeout
		command_buffer = bytearray()
		in_command = False
		while (time.time() - time_start) < timeout:
			input_data = self.ser.read(1)

			if len(input_data) == 0:
				continue

			if not in_command:
				# Not in a command, we'll accept a 0x05 or 0x06 (and ignore the rest).
				if input_data[0] in [0x06, 0x05]:
					return input_data

				# Start handling a command
				if input_data[0] == 0x02:
					in_command = True
					command_buffer += input_data

				# Anything else: keep waiting for 0x02 or timeout
				continue 
			else:
				command_buffer += input_data

				# Check if we're complete (got 0x03 + 2 checksum bytes)
				if len(command_buffer) >= 4 and command_buffer[-3] == 0x03:
					# TODO: verify checksum
					# check_actual = struct.pack(">H", XDL.__checksum(incoming[1:-2]))
					# check_received = incoming[-2:]
					return command_buffer



	def __send_command(self, cmd, exclude_checksum=False):
		# print(f"Sending: {cmd}")

		if exclude_checksum:
			footer = b''
		else:
			footer = (
				b'\x03' + # start of checksum
				struct.pack(">H", XDL.__checksum(cmd + b'\x03')) # checksum
			)

		self.ser.write(
			b"\x02" + # start of data
			cmd + 
			footer
		)

		ret = self.__get_response()

		# print(f"Received: {ret}")

		return ret

	# Waits for message from terminal, initiating Download mode
	def connect(self, repeat_delay=0.05):
		if self.connected:
			self.__log("[-] Already connected, can't connect.")
			return False

		i = 0 # for the visual
		while True:
			i += 1

			time.sleep(repeat_delay)

			if self.log:
				spinner = ["-", "\\", "|", "/"][i % 4]
				sys.stdout.write("\r[" + spinner + "] Trying to connect...")
				sys.stdout.flush()

			# host "SYN"
			self.ser.write(b'\x05')

			# Check for a reply
			if self.ser.in_waiting > 0:
				response = self.__get_response()

				if not response:
					continue

				if response[1:4] != b"VFI":
					self.__log(f"[-] Invalid connection response: {response}")
					return False
			
				self.connected = True
				self.connection_string = response[1:-3].decode('ascii')
				self.__log(f"\n[+] Connected: {self.connection_string}")

				# Got a proper answer: reply with "ACK"
				self.ser.write(b"\x06")
				break

		return self.connection_string

	def message(self, msg):
		if not self.connected:
			self.__log("[i] Can't send message, connect first.")
			return False

		# useful characters:
		# \r   -> starts first line at column 0
		# \f   -> starts at top of screen
		# \x00 -> checkerboard square
		# \x7f -> black square
		# \x10 -> arrow characters (until \x19)
		# \x8? -> some more drawing chars in the extended part

		# Message needs to start with a '-' to prevent "printer not open" error
		# hack: "-\rhello", "-\fhello" or "-\bhello"

		if type(msg) is str:
			msg_bytes = msg.encode("ascii")
			self.__send_command(b"M" + msg_bytes)
		elif type(msg) is bytes:
			self.__send_command(b"M" + msg)

	def set_config_var(self, var, val):
		if not self.connected:
			self.__log("[i] Can't set config var, connect first.")
			return False

		self.__log(f"[+] Setting config var: {var}={val}")

		var_bytes = var.encode("ascii")
		val_bytes = val.encode("ascii")
		self.__send_command(b"L" + var_bytes + b"\x1c" + val_bytes)



	def send_file(self, filename, code=True):
		if not self.connected:
			self.__log("[i] Can't send file, connect first.")
			return False

		# Open file and get size
		fd = open(filename, "rb")
		file_size = os.fstat(fd.fileno()).st_size
		basename = os.path.basename(filename)

		self.__log(f"[+] Sending '{filename}' ({basename}) as " + ("code" if code else "data"))

		# Start: announcing date and file name
		self.__send_command(
			b'O' + 
			(b'C' if code else b'D') + 
			b'000000C300000000' + # unknown
			b'20220102112233' + # timestamp TODO (YYYYMMDDHHmmss)
			b'X'*8 + # unknown
			basename.encode('ascii')
		)

		self.message("-\f") # clear screen

		# # size max 1000 (0x03e8)
		# b'\x02W' + BE SIZE (2b) + AMOUNT ALREADY SENT (4b) + b'<contents>' + b'\xfcR'

		# Send file in 1000-byte chunks
		already_sent = 0
		i = 0
		while True:
			chunk = fd.read(1000)
			chunk_len = len(chunk)

			# empty at EOF
			if chunk_len == 0:
				break

			# Display progress on device
			progress_bar = b"\xff" * math.ceil((already_sent / file_size) * 12)
			progress_bar += [b"-", b"\\", b"|", b"/"][i % 4]
			progress_bar += (13 - len(progress_bar)) * b"_"
			self.message(b"-\b[" + progress_bar + b"]")

			if self.log:
				progress_percent = math.ceil((already_sent / file_size) * 100.0) 
				sys.stdout.write(f"\r[+] Progress: {progress_percent}%")
				sys.stdout.flush()

			cmd = (
				b'W' +
				struct.pack(">H", chunk_len) +
				struct.pack(">I", already_sent) +
				chunk
			)

			# CRC-16/BUYPASS, exclude 03 and normal checksum.
			crc = XDL.__crc16_buypass(cmd)
			
			self.__send_command(
				b'\x02' +
				cmd +
				struct.pack(">H", crc),
				exclude_checksum=True
			)

			already_sent += chunk_len
			i += 1

		# Indicate end of file
		self.__send_command(b'C')

		if self.log:
			sys.stdout.write("\n")

	def stop(self):
		if not self.connected:
			self.__log("[i] Can't stop, connect first.")
			return False

		self.__send_command(b"S")
		# self.ser.write(b"\x04") # not sure if this is needed

		self.__log("[+] Disconnected")

		self.connected = False


def main():
	parser = argparse.ArgumentParser(description='XDL client')
	parser.add_argument('--verbose', '-v', action='count', default=0, help='enable logging')
	parser.add_argument('--device', '-d', type=str, help='TTY device', default='/dev/ttyUSB0')
	parser.add_argument('--baud', '-b', type=int, help='baud rate', default=115200)
	parser.add_argument('-c', metavar='VAR=VAL', type=str, action='append', help='config var and value')
	parser.add_argument('files', metavar='FILE', type=str, nargs='+', help='files to send')
	args = parser.parse_args()

	# TODO: improve validation and error checking

	xdl = XDL(port=args.device, log=(args.verbose > 0))
	xdl.connect()

	if args.c:
		for config_var_val in args.c:
			if '=' not in config_var_val:
				print(f"Error: invalid config var: {config_var_val}. Quitting.")
				return

			config_var = config_var_val[:config_var_val.index('=')]
			config_val = config_var_val[config_var_val.index('=')+1:]

			xdl.set_config_var(config_var, config_val)

	for file_name in args.files:
		# TODO: determine code/data type. For now we assume code.
		xdl.send_file(file_name)

	xdl.stop()


if __name__ == "__main__":
	main()



