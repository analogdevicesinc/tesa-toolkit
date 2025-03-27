Keys Folder
===========

This folder includes test certificate and script file (that extract public & private key from certificate).

- bl1_dummy.pem: Test certifcate that can be used for development purpose
- bl1_dummy.key: Includes pub & priv key of bl1_dummy.pem
- extract_bl1_key.py: Script to extract pub & priv key from certificate

To create your own private key, run:

`openssl ecparam -out <MY_CERT_FILE.pem> -genkey -name prime256v1`

Incase of need public & private key can be extract key from your ceritifate by:

`python extract_bl1_key.py -c <MY_CERT_FILE.pem> -o <KEY_OUT_FILE>`
