#include<stdio.h>
#include<string.h>
#include<stddef.h>
#include<sodium.h>
#include <iup.h>
#include "util.h"

static Ihandle *pw_text= NULL, *decrypt_button = NULL;

static char self_name[4096] = {0};

int decrypt_cb(Ihandle *handle){
  
  // hash the password.
  char pwd[4096] = {0};
  strncpy(pwd, IupGetAttribute(pw_text, "VALUE"), sizeof(pwd));
  pwd[sizeof(pwd) - 1] = '\0';
  size_t pwd_len = strlen(pwd);
  if (pwd_len == 0){
    IupMessagef("Error", "Please enter a longer password.");
    return IUP_DEFAULT;
  }

  // hash the password
  uint8_t hash[crypto_box_SEEDBYTES] = {0};
  int hash_rc = crypto_pwhash(hash, sizeof(hash), pwd, pwd_len, SALT, crypto_pwhash_OPSLIMIT_INTERACTIVE, crypto_pwhash_MEMLIMIT_INTERACTIVE, crypto_pwhash_ALG_DEFAULT);
  if (hash_rc != 0){
    IupMessagef("Error", "Failed to hash password rc=%d", hash_rc);
    return IUP_DEFAULT;
  }

  // hash again for the nonce bytes
  uint8_t nonce[crypto_box_NONCEBYTES] = {0};
  hash_rc = crypto_pwhash(nonce, sizeof(nonce), pwd, pwd_len, SALT, crypto_pwhash_OPSLIMIT_INTERACTIVE, crypto_pwhash_MEMLIMIT_INTERACTIVE, crypto_pwhash_ALG_DEFAULT);
  if (hash_rc != 0){
    IupMessagef("Error", "Failed to hash password for nonce rc=%d", hash_rc);
    return IUP_DEFAULT;
  }

  // generate public and secret keys from the password hash
  uint8_t sk[crypto_box_SECRETKEYBYTES] = {0}, pk[crypto_box_PUBLICKEYBYTES ] = {0};
  int rc = crypto_box_seed_keypair(pk, sk, hash);
  // TODO: zero hash
  if (rc != 0){
    IupMessagef("Error", "Failed to Generate key pair! rc=%d", rc);
    return IUP_DEFAULT;
  }

  // pre-generate key to encrypt the file.
  uint8_t result_k[crypto_box_BEFORENMBYTES] = {0};
  rc = crypto_box_beforenm(result_k, pk, sk);
  // TODO: set pk and sk to 0 here
  if (rc != 0){
    IupMessagef("Error", "Failed to pre-compute key! rc=%d", rc);
    return IUP_DEFAULT;
  }

  // get file len
  FILE *self = fopen(self_name, "rb");
  if (!self){
    IupMessagef("Error", "Failed Opening %s for extraction!", self_name);
    return IUP_DEFAULT;
  }

  //TODO zero out all keys.
  uint64_t file_size = 0;
  int64_t offset = - (int64_t)(SENTINEL_LEN  + sizeof(file_size));
  if (fseek(self, offset, SEEK_END) != 0){
	IupMessagef("Error", "Failed to seek in %s", self_name);
	goto closein;
  }

  if (fread(&file_size, 1, sizeof(file_size), self) != sizeof(file_size)){
	IupMessagef("Error", "Failed to read %s", self_name);
	goto closein;
  }

  //seek to the beginning of the stored file (not the .exe file)
  offset -= file_size;
  if (fseek(self, offset , SEEK_END) != 0){
	IupMessagef("Error", "Failed to seek in %s", self_name);
	goto closein;
  }
  
  const char *out_path = "decrypted.txt";
  FILE *fout = fopen(out_path, "wb");
  if (fout == NULL){
	IupMessagef("Error", "Failed to open output file!");
	goto closein;
  }

  uint64_t in1_size = 0, out_size = 0;
  // read the file and encrypt it, dump it to the output file.
  uint8_t file_buf[FREAD_BYTES] = {0};
  size_t buf_size = sizeof(file_buf);
  // leave space for the maclen so we can encrypt in place
  for (size_t bytes_left = file_size, bytes_read = 0; bytes_left > 0; bytes_left -= bytes_read){
    size_t read_size = bytes_left > buf_size ? buf_size : bytes_left;
    bytes_read = fread(file_buf, 1, read_size, self);
    in1_size += bytes_read;

    // encrypt the stuff
    rc = crypto_box_open_easy_afternm(file_buf, file_buf, bytes_read, nonce, result_k);
    if (rc != 0){
      IupMessagef("Error", "Failed to decrypt (or authenticate) data! rc=%d", rc);
      goto closeout;
    }
    // remove the size of the MAC
    size_t write_size = bytes_read - crypto_box_MACBYTES;

    size_t bytes_out = fwrite(file_buf, 1, write_size, fout);
    if (bytes_out != write_size){
      IupMessagef("Error", "Failed writing to %s! Expected %u bytes, got %u!\n", out_path, write_size, bytes_out);
      goto closeout;
    }
    out_size += bytes_out;
  }
  IupMessagef("Success!", "Wrote %u bytes to %s\n", out_size, out_path);

closeout:
  fclose(fout);

closein:
  fclose(self);
  return IUP_DEFAULT;
}

int main(int argc, char **argv){
  IupOpen(&argc, &argv);
  IupSetLanguage("ENGLISH");
  IupSetGlobal("UTF8MODE", "Yes");

  int rc = EXIT_FAILURE;

  // see if an archive is attached
  FILE *self = fopen(argv[0], "rb");
  if (self == NULL){
	IupMessagef("Error", "Failed to open %s", argv[0]);
	return EXIT_FAILURE;
  }

  // go to the end and see if a sentinel is present
  if (fseek(self, - SENTINEL_LEN, SEEK_END) != 0){
	IupMessagef("Error", "Failed to get to the end of %s", argv[0]);
	return EXIT_FAILURE;
  }

  char sentinel[SENTINEL_LEN + 1];
  memset(sentinel, 0, sizeof(sentinel));
  size_t b_read = fread(sentinel, 1, SENTINEL_LEN, self);
  if (b_read != SENTINEL_LEN){
	IupMessagef("Error", "Failed to read %s", argv[0]);
  }
  fclose(self);

  if (strncmp(sentinel, END_SENTINEL, SENTINEL_LEN) != 0){
	IupMessagef("Error", "File sentinel not found %s\nIs file attached?", argv[0]);
	return EXIT_FAILURE;
  }
  // close the file for now. We'll open it if we need to later.

  // copy the path of the exe into this buffer so we can use it later.
  strncpy(self_name, argv[0], sizeof(self_name) - 1);

  // set up the window to ask for a password
  pw_text = IupText(NULL);
  decrypt_button = IupButton("Decrypt File", "decrypt_button");
  IupSetCallback(decrypt_button, "ACTION", (Icallback)decrypt_cb);
  Ihandle *pw_label = IupLabel("Enter Password:");

  IupSetAttributes(pw_text, "EXPAND = HORIZONTAL, SIZE = 150x, PADDING = 10x10, PASSWORD = YES");

  Ihandle *dlg = IupDialog(
      IupHbox(
	pw_label,
	pw_text,
	decrypt_button,
	NULL)
      );
  IupShowXY(dlg, IUP_CENTER, IUP_CENTER);
  IupSetAttributes(dlg, "EXPAND = YES, TITLE = extract encrypted file");
  IupMainLoop();

  IupClose();

  return EXIT_SUCCESS;
}
