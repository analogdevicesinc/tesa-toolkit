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
#
#-------------------------------------------------------------------------------

import argparse
import ecdsa
import hashlib


def sign_ecdsa(infile, outfile, certfile):
	with open(infile, 'rb') as f:
		bin_file = f.read()

	with open(certfile, 'r') as f:
		cert_data = f.read()

	sk = ecdsa.SigningKey.from_pem(cert_data, hashfunc=hashlib.sha256)
	sig = sk.sign(bin_file, hashfunc=hashlib.sha256)

	#vk = sk.verifying_key
	#vk.verify(sig, bin_file, hashfunc=hashlib.sha256)

	print("\nGenerated Signature:")
	print(sig.hex())

	with open(outfile, 'wb') as f:
		f.write(bin_file)
		f.write(sig)


if __name__ == '__main__':

	parser = argparse.ArgumentParser()

	parser.add_argument("--input_file", help="the image to process", required=True)
	parser.add_argument("--sign_key_file", help="signing key file", required=False)
	parser.add_argument("--img_output_file", help="image output file", required=True)
	args = parser.parse_args()

	print("Signing:")
	print(f"Input File:  {args.input_file}")
	print(f"Certificate: {args.sign_key_file}")
	print(f"Output File: {args.img_output_file}")

	sign_ecdsa(args.input_file, args.img_output_file, args.sign_key_file)

	print("\nSignature Generation Succeeded")
