Dump Device Info
================
This folder includes utility fw and script that help for debugging device.
The fw includes function to dump and erase flash content.

Open an terminal application on the PC and connect to the EV kit's console UART at 115200, 8-N-1.
Then run below command to load and execute fw:

`python load_and_exec.py`


Expected Output
===============

The Console UART of the device will output these messages:

```
**** MAX32657 Secure Boot ROM Provisioning FW v1.0.0 ****
date: 'Mar 27 2025'
time: '14:14:43'

USN:
05 00 AB CD EF 01 00 01 02 AB CD F6 AE

Test Menu
----------------------------------------
1  - Print BL2 Provision Configurations
2  - Dump Device Info Block
3  - Dump User Info Block
4  - Erase User Info Block
5  - Mass Erase FLC

Please select:
```
