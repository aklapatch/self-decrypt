# self-decrypt
Hopefully this will be a program that makes self-decrypting archives

## General idea
I want to be able to send an encrypted payload that is password
protected. The program should prompt for a password when cliced on, and
then if the password is correct, then it will extract the payload. The
payload should be AES-256 encrypted, and verified with ECC P384 if possible.

### Crypto plans
- Use Argon2id for password hashing.
- Use tomcrypt for ChaCha20Poly1305 and ecc sign+verify

First, ECC verify the password hash.
The password hash will be split into a key, IV, and a MAC addition. 
The whole password hash will also be used for authdata.
There will be some user provided randomness (from the interval of how many times they push a ui button or something, fed into chacha20), that will be used to create an ECC private key.
A paired public key will be made from that private key. That key pair
will be used to sign the payload MAC. Part of the payload MAC will come
from ChaCha20Poly1305, and the other 24 bytes will come from part of the password hash.

### File Format Plans
The self-decrypting archive format:
```
| decryption program |
|--------------------|
| encrypted payload  |
|--------------------|
| Payload signature  |
|--------------------|
| password signature |
|--------------------|
| ECC P384 Pub key   |
|--------------------|
| payload length u64 |
|--------------------|
| sentinel value     |
```
The decryption will open itself as a file, ask for a password, then try
to decrypt the value.

The encrypting program format:
```
| encryption program           |
|------------------------------|
| decryption program           |
|------------------------------|
| decrypt program length (u64) |
|------------------------------|
| sentinel value               |
```
The encryption program will append the encrypted payload to the
decryption program.
