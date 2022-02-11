#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <stddef.h>
#include <sodium.h>
#include "util.h"
#include <iup.h>

static char in_path[10240] = "", out_path[10240] = "";

static Ihandle *f_in_text = NULL, *f_out_text = NULL, *pw_text = NULL, *pw_text2 = NULL, *next_button = NULL; 

static bool has_in_path = false, has_out_path = false;

static clock_t in_pick_time = 0, out_pick_time = 0, generate_time = 0;

static char *file_base_name(char *str){
  char *end = str + strlen(str);

  // go backwards until you hit str or untile you hit '/' or '\\'
  for (char *cur = end; cur > str; --cur){
    if (*cur == '/' || *cur == '\\'){
      return cur + 1;
    }
  }
  return NULL;
}
void maybe_activate_next_button(){
  const char * attr = has_in_path && has_out_path ? "ACTIVE = YES" : "ACTIVE = NO";
  IupSetAttributes(next_button, attr);
}

int get_in_path(Ihandle *self){
  Ihandle *dlg = IupFileDlg();
  IupSetAttributes(dlg, "DIALOGTYPE = OPEN, TITLE = \"File Select\"");
  IupSetAttributes(dlg, "FILTER = \"*\", FILTERINFO = \"All Files\"");
  IupPopup(dlg, IUP_CENTER, IUP_CENTER);
  int ret = IupGetInt(dlg, "STATUS");
  switch (ret)
  {
    case 0 :
      memset(in_path, 0, sizeof(in_path));
      strncpy(in_path, IupGetAttribute(dlg, "VALUE"), sizeof(in_path));
      IupSetAttribute(f_in_text, "VALUE", in_path);
      has_in_path = true;
      break;
  }
  maybe_activate_next_button();
  in_pick_time = clock();
  return IUP_DEFAULT;
}

int get_out_path(Ihandle *self){
  Ihandle *dlg = IupFileDlg();
  IupSetAttributes(dlg, "DIALOGTYPE = SAVE, TITLE = \"File Select\"");
  IupSetAttributes(dlg, "FILTER = \"*.exe\", FILTERINFO = \".exe Files\"");
  IupPopup(dlg, IUP_CENTER, IUP_CENTER);
  switch(IupGetInt(dlg, "STATUS"))
  {
    case 1:
    case 0:
      memset(out_path, 0, sizeof(out_path));
      strncpy(out_path, IupGetAttribute(dlg, "VALUE"), sizeof(out_path));
      // append .exe if it's not there
      const char exe[] = ".exe";
      size_t pth_len = strlen(out_path), exe_len = strlen(exe);
      if (strncmp(out_path + pth_len - exe_len, exe, exe_len) != 0){
	strncat(out_path, exe, sizeof(out_path) - exe_len);
      }
      IupSetAttribute(f_out_text, "VALUE", out_path);
      has_out_path = true;
      break;
  }
  // make the Generate file button active if we have both paths and a password.
  maybe_activate_next_button();
  out_pick_time = clock();
  return IUP_DEFAULT;
}

bool to_hex(uint8_t *in, size_t in_len, char *out, size_t out_len){
  if (2*in_len + 1 > out_len){
    return false;
  }

  // populate the char map
  char char_map[16];
  for (uint8_t i = 0; i < sizeof(char_map)/sizeof(char_map[0]); ++i){
    if (i < 10){
      char_map[i] = '0' + i;
      continue;
    } 
    char_map[i] = 'a' + i - 10;
  }

  // last time I wrote code for this I messed it up, so this may be wrong
  size_t i = 0;
  for (; i < in_len; ++i){
    uint8_t upper_nibble = in[i] >> 4, lower_nibble = in[i] & 0xf;

    size_t out_i = 2*i;
    out[out_i] = char_map[upper_nibble];
    out[out_i + 1] = char_map[lower_nibble];
  }
  out[2*in_len] = '\0';
  return true;
}


int encrypt_archive_cb(Ihandle *self){
  generate_time = clock();

  // hash the password.
  char pwd[4096] = {0};
  strncpy(pwd, IupGetAttribute(pw_text, "VALUE"), sizeof(pwd));
  pwd[sizeof(pwd) - 1] = '\0';
  size_t pwd_len = strlen(pwd);
  if (pwd_len == 0){
    IupMessagef("Error", "Please enter a longer password.");
    return IUP_DEFAULT;
  }

  // error if the passwords don't match
  if (strncmp(pwd, IupGetAttribute(pw_text2, "VALUE"), strlen(pwd)) != 0){
    IupMessagef("Error", "Passwords do not match!");
    return IUP_DEFAULT;
  }
  
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
  if (rc != 0){
    IupMessagef("Error", "Failed to Generate key pair! rc=%d", rc);
    return IUP_DEFAULT;
  }

  // pre-generate key to encrypt the file.
  uint8_t result_k[crypto_box_BEFORENMBYTES] = {0};
  rc = crypto_box_beforenm(result_k, pk, sk);
  memset(sk, 0, sizeof(sk));
  memset(pk, 0, sizeof(pk));
  if (rc != 0){
    IupMessagef("Error", "Failed to pre-compute key! rc=%d", rc);
    return IUP_DEFAULT;
  }


  // grab the first file and write it to the output file first.
  FILE *fout = fopen(out_path, "wb");
  if (fout == NULL){
    IupMessagef("Error", "Opening %s failed!", out_path);
    return IUP_DEFAULT;
  }

  char extract_name[4096] = { "extract.exe"};
  FILE *extractor_f = fopen(extract_name, "rb");
  if (extractor_f == NULL){
    IupMessagef("Error", "Opening %s failed!", extract_name);
    return IUP_DEFAULT;
  }

  uint8_t file_buf[FREAD_BYTES] = {0};
  size_t rd_len = sizeof(file_buf);
  // write the extractor program to a file first.
  for (size_t bytes_read = rd_len; bytes_read == rd_len;){
    bytes_read = fread(file_buf, 1, rd_len, extractor_f);
    fwrite(file_buf, 1, bytes_read, fout);
  }
  fclose(extractor_f);

  FILE *fin1 = fopen(in_path, "rb");
  if (fin1 == NULL){
    IupMessagef("Error", "Opening %s failed!", in_path);
    return IUP_DEFAULT;
  }

  uint64_t in1_size = 0, out_size = 0;
  // read the file and encrypt it, dump it to the output file.
  // leave space for the maclen so we can encrypt in place
  size_t read_size = sizeof(file_buf) - crypto_box_MACBYTES;
  for (size_t bytes_read = read_size; bytes_read == read_size;){
    bytes_read = fread(file_buf, 1, read_size, fin1);
    in1_size += bytes_read;

    // encrypt the stuff
    rc = crypto_box_easy_afternm(file_buf, file_buf, bytes_read, nonce, result_k);
    if (rc != 0){
      IupMessagef("Error", "Failed to encrypt data! rc=%d", rc);
      goto cleanup;
    }
    size_t write_size = bytes_read + crypto_box_MACBYTES;

    size_t bytes_out = fwrite(file_buf, 1, write_size, fout);
    if (bytes_out != write_size){
      IupMessagef("Error", "Failed writing to %s! Expected %u bytes, got %u!\n", out_path, write_size, bytes_out);
      goto cleanup;
    }
    out_size += bytes_out;
  }

  // write sentinel and size
  out_size += fwrite(&out_size, 1, sizeof(out_size), fout);
  //
  // put down null terminator
  fputc('\0', fout);
  out_size += 1;

  // write a common file prefix
  const char prefix[] = "decrypted-";
  fputs(prefix, fout);
  out_size += strlen(prefix);

  // write the file name out
  char *base_name = file_base_name(in_path);
  size_t base_name_len = strlen(base_name);
  fputs(base_name, fout);
  out_size += base_name_len;

  out_size += fwrite(END_SENTINEL, 1, strlen(END_SENTINEL), fout);
  
  fclose(fin1); fclose(fout);
  fin1 = NULL, fout= NULL;

  IupMessagef("Success", "Success!\n\nRead %lu bytes from\n%s.\n\nWrote %lu bytes to\n%s",in1_size,  in_path, out_size, out_path);

cleanup:
  memset(result_k, 0, sizeof(result_k));
  if (fin1 != NULL){
    fclose(fin1);
  }
  if (fout != NULL){
    fclose(fout);
  }

  return IUP_DEFAULT; 
} 

int main(int argc, char **argv) {

  IupOpen(&argc, &argv);
  IupSetLanguage("ENGLISH");
  IupSetGlobal("UTF8MODE", "Yes");

  f_in_text = IupText(NULL);
  f_out_text = IupText(NULL);
  pw_text = IupText(NULL);
  pw_text2 = IupText(NULL);
  next_button = IupButton("Encrypt File", "next_button");
  IupSetAttributes(next_button, "ACTIVE = NO");

  Ihandle *in_button = IupButton("Browse", "btn_f_in"),
	  *out_button = IupButton("Browse", "btn_f_out"),
	  *in_label = IupLabel(" Input File: "),
	  *out_label = IupLabel(" Output File:"),
	  *pw_label = IupLabel(" Password:"),
	  *pw_label2 = IupLabel("Confirm Password:");

  IupSetAttributes(in_label, "PADDING = 5x5, SIZE = 60x8");
  IupSetAttributes(out_label, "PADDING = 5x5, SIZE = 60x8");
  IupSetAttributes(pw_label, "PADDING = 5x5, SIZE = 60x8");
  IupSetCallback( out_button, "ACTION", (Icallback)get_out_path);
  IupSetCallback( next_button, "ACTION", (Icallback)encrypt_archive_cb);

  IupSetCallback( in_button, "ACTION", (Icallback)get_in_path);
  
  IupSetAttributes(f_in_text, "READONLY = YES, EXPAND = HORIZONTAL, SIZE = 200x, PADDING = 10x10");
  IupSetAttributes(f_in_text, "VALUE = \"Use this Browse Button to select an input file. ->\"");

  IupSetAttributes(f_out_text, "READONLY = YES, EXPAND = HORIZONTAL, PADDING = 10x10");
  IupSetAttributes(f_out_text, "VALUE = \"Use this Browse Button to select an output file. ->\"");
  IupSetAttributes(pw_text, "PASSWORD = YES, EXPAND = HORIZONTAL, PADDING = 10x10");
  IupSetAttributes(pw_text2, "PASSWORD = YES, EXPAND = HORIZONTAL, PADDING = 10x10");

  Ihandle *dlg = IupDialog(
	      IupVbox(
		IupHbox(
		  in_label,
		  f_in_text,
		  in_button,
		  NULL),
		IupHbox(
		  out_label,
		  f_out_text,
		  out_button,
		  NULL),
		IupHbox(
		  pw_label,
		  pw_text,
		  NULL),
		IupHbox(
		  pw_label2,
		  pw_text2,
		  NULL),
		IupHbox(
		  next_button,
		  NULL),
		NULL
		)
	      );
  IupShowXY( dlg, IUP_CENTER, IUP_CENTER );
  IupSetAttributes(dlg, "EXPAND = YES, TITLE = self-decrypt");
  IupMainLoop();

  /* Finishes IUP */
  IupClose();

  /* Program finished successfully */
  return EXIT_SUCCESS;
}
