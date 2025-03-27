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
import argparse
import base64


def convert_pem_to_der(cert_pem):
    with open(cert_pem, 'r') as f:
        cert_data = f.read()
    
    start = "-----BEGIN EC PRIVATE KEY-----"
    end = "-----END EC PRIVATE KEY-----"
    
    key_base64 = cert_data.split(start)[1].split(end)[0]
    key_bytes = base64.b64decode(key_base64)
    return key_bytes

def extract_key(cert, out_file):
    key_bytes = convert_pem_to_der(cert)

    priv_offset = 7
    pub_offset = 57

    priv = "".join([f"{b:02x}" for b in key_bytes[priv_offset:(priv_offset+32)]])
    pub  = "".join([f"{b:02x}" for b in key_bytes[pub_offset:(pub_offset+32)]]) + "\n"
    pub += "".join([f"{b:02x}" for b in key_bytes[(pub_offset+32):(pub_offset+64)]])

    with open(out_file, 'w') as f:
        f.write(priv + "\n")
        f.write(pub + "\n")
        f.write("#1st line private key\n")
        f.write("#2nd, 3rd lines public key (x,y)\n")


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Parameter need to be passed')

    parser.add_argument("-c", "--cert", dest="cert_file",  help="Cerfitifcate FILE", metavar="FILE")
    parser.add_argument("-o", "--out-file", dest="out_file",  help="Output FILE", metavar="FILE")
      
    args = parser.parse_args()

    if (args.out_file == None) or (args.cert_file == None):
        print("Usage error!")
        print(args)
        sys.exit(1)

    extract_key(args.cert_file, args.out_file)
    print(f"Key written into {args.out_file} file")
