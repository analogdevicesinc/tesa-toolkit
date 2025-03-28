#-------------------------------------------------------------------------------
# Copyright (C) 2025 Analog Devices, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#-------------------------------------------------------------------------------
import sys
import os
import argparse
import base64
from elftools.elf.elffile import ELFFile


def convert_pem_to_der(cert_pem):
	with open(cert_pem, 'r') as f:
		cert_data = f.read()
	
	start = "-----BEGIN EC PRIVATE KEY-----"
	end = "-----END EC PRIVATE KEY-----"
	
	key_base64 = cert_data.split(start)[1].split(end)[0]
	key_bytes = base64.b64decode(key_base64)
	return key_bytes


def update_section_in_elf(cert, elf_file, section_name):
	with open(elf_file, 'r+b') as f:
		elf = ELFFile(f)

		# Check if the section exists
		section = elf.get_section_by_name(section_name)
		if not section:
			print(f"Section {section_name} not found!")
			return

		#print(' '.join(f'{b:02x}' for b in section.data()))
		if section.header['sh_size'] != 64:
			print('Section size is not correct')
			return

		key_bytes = convert_pem_to_der(cert)
		pub_offset = 57
		pub_key = key_bytes[pub_offset:(pub_offset+64)]

		# Update section
		f.seek(section.header['sh_offset'])
		f.write(pub_key)


def bl1_provision():
	# Execute JLink Script
	JLinkExe = "JLink" if os.name == "nt" else "JLinkExe"
	os.system(JLinkExe + " -device MAX32657 -if swd -speed 2000 -autoconnect 1 -CommanderScript JLinkScript")


if __name__ == '__main__':
	parser = argparse.ArgumentParser(description='Parameters need to be passed')

	parser.add_argument("-c", "--cert", dest="cert_file", help="Cerfitifcate FILE", metavar="FILE")

	args = parser.parse_args()

	if args.cert_file == None:
		print("Usage error, please specifcy certificate file.")
		print(args)
		sys.exit(1)

	print("\n\033[91mWARNING:\033[0m")
	print("This script will enable Secure Boot mode")
	print("which will write your public key in the OTP and turn off debug interface")
	print("")
	print("After that device will not be reprogrammed!")
	print("Be sure you write your final images on the device.\n")

	response = input("Do you want to continue? (Y/N): ").strip().upper()
	if response not in ['Y', "YES"]:
		print("Operation aborted.")
		sys.exit(0)

	print("\n-------------------------")
	elf_file = "bl1_provision.elf"
	update_section_in_elf(args.cert_file, elf_file, '.pubkey')
	print(".pubkey section updated")
 
	print("\n-------------------------")
	bl1_provision()
	print("bl1 provision done.")
