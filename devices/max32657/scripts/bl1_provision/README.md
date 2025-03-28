BL1 Provision
=============
This folder includes provisioning fw and script that activate MAX32657 SecureBoot ROM.
After SecureBoot ROM has been enabled MCUBoot image must be signed to it be validated by SecureBoot ROM.
The .elf file in this folder does not include user public key, the enable_secureboot.py script
take user certificate, decode public key and update .pubkey section in .elf file
then activate secureboot by using JLinkScript.

Open an terminal application on the PC and connect to the EV kit's console UART at 115200, 8-N-1.
Then run below command to load and execute fw:

`python enable_secureboot.py -c ../../keys/bl1_dummy.pem`


Note:
    User shall load final application images before provision device.
    The final images can loaded during device provisioning, in that case
    put your image in the JLinkScript file to it be written during device provision.

    User can create your certificate by openssl:

    `openssl ecparam -out <MY_CERT_FILE.pem> -genkey -name prime256v1`


Expected Output
===============

The Console UART of the device will output these messages:

```
**** MAX32657 Secure Boot ROM Provisioning FW v1.0.0 ****
date: 'Mar 28 2025'
time: '12:01:01'

USN:
05 00 AB CD EF 01 00 01 02 AB CD F6 AE
BBREG0 (@ 0x50006C30) Status: 0x00000000
BBREG1 (@ 0x50006C34) Status: 0x00000000
Warm Boot: Disabled
CRK:
B2 D7 E4 FA 58 20 50 DE CA 1E E4 34 8F 8F 60 7F
25 05 A9 91 CA 25 8F 7E 9E 82 A3 13 DB 54 95 E5
78 1C 2F F7 9E 35 30 23 87 DF 42 F3 58 2F 2D 17
36 8B A9 E5 9E 6E 3B B2 CB D7 41 09 C3 8A 60 81

CRK Written!

Locks left = 4, Unlocks left = 4, Debug port locked = 0
Debug port is Unlocked, attempting Lock.
Debug port is now Locked.

Locks left = 3, Unlocks left = 4, Debug port locked = 1
Debug port is currently Locked.
Will now permanently freeze the state.
Debug port is Locked Permanently.

Secure boot enabled.
```
