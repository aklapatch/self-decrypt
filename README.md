# self-decrypt
Hopefully this will be a program that makes self-decrypting archives

## General idea
I want to be able to send an encrypted payload that is password
protected. The program should prompt for a password when cliced on, and
then if the password is correct, then it will extract the payload. The
payload should be AES-256 encrypted, and verified with ECC P384 if possible.

### Crypto plans
- Use libsodium (MSYS2 x86_64 package version)
- Authenticate the password before decrypting stuff.
- Authenticate the decrypted stuff somehow (maybe ECC?)

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
