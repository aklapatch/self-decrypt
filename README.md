# self-decrypt
Hopefully this will be a program that makes self-decrypting archives

## Buiding
1. Install [MSYS2](https://www.msys2.org/)
2. Open the MSYS2 MINGW64 shell
3. Install these libraries (gcc and libsodium, and make)
```
pacman -S make mingw-w64-x86_64-gcc mingw-w64-x86_64-libsodium

```
4. Clone this repo
```
git clone --depth=1 https://github.com/aklapatch/self-decrypt
```
5. Build everything (The IUP toolkit is included in the repo. that will get built too).
```
cd self-decrypt && make
```
The `self-decrypt.exe` executable should be in repo's root directory when the build is finished.

## File Format Plans
The self-decrypting archive format:
```
| decryption program |
|--------------------|
| encrypted payload  |
|--------------------|
| payload length u64 |
|--------------------|
| payload name       |
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
decryption program and set up the payload name and sentinel.
