#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <stddef.h>
#include <sodium.h>
#include <iup.h>


static char in_path[10240] = "", out_path[10240] = "";

static Ihandle *f_in_text = NULL, *f_out_text = NULL, *pw_text = NULL, *next_button = NULL; 

static bool has_in_path = false, has_out_path = false;

static clock_t in_pick_time = 0, out_pick_time = 0, generate_time = 0;

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
  IupSetAttributes(dlg, "DIALOGTYPE = SAVE, TITLE = \"Archive Save\"");
  IupSetAttributes(dlg, "FILTER = \"*.exe\", FILTERINFO = \"Executable Files\"");
  IupPopup(dlg, IUP_CENTER, IUP_CENTER);
  switch(IupGetInt(dlg, "STATUS"))
  {
    case 1:
    case 0:
      memset(in_path, 0, sizeof(in_path));
      strncpy(out_path, IupGetAttribute(dlg, "VALUE"), sizeof(out_path));
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
  pwd[sizeof(pwd)  -1] = '\0';
  size_t pwd_len = strlen(pwd);
  
  uint8_t hash[48] = {0};
  // use a bad salt (It needs to be predictable so it can be verified).
  char salty[42] = {0};
  memset(salty, '3', sizeof(salty) - 1);
  size_t hash_len = sizeof(hash);
  // hash with no salt.
  int hash_rc = crypto_pwhash(hash, sizeof(hash), pwd, pwd_len, salty, crypto_pwhash_OPSLIMIT_INTERACTIVE, crypto_pwhash_MEMLIMIT_INTERACTIVE, crypto_pwhash_ALG_DEFAULT);
  if (hash_rc != 0){
    IupMessagef("Error", "Failed to hash password rc=%d", hash_rc);
    return IUP_DEFAULT;
  }

  // hash password for storage too
  char hash_pwd[crypto_pwhash_STRBYTES] = {0};
  hash_rc = crypto_pwhash_str(hash_pwd, pwd, pwd_len, crypto_pwhash_OPSLIMIT_INTERACTIVE, crypto_pwhash_MEMLIMIT_INTERACTIVE);
  if (hash_rc != 0){
    IupMessagef("Error", "Failed to hash password for storage rc=%d", hash_rc);
    return IUP_DEFAULT;
  }
  char hex_hash[sizeof(hash)*2 + 1], storage_hash[crypto_pwhash_STRBYTES*2 + 1];
  to_hex(hash, sizeof(hash), hex_hash, sizeof(hex_hash));
  to_hex(hash_pwd, sizeof(hash_pwd), storage_hash, sizeof(storage_hash));
  IupMessagef("Password hash", "hash=%s\nstorage hash= %s", hex_hash, storage_hash);

  // gerneate public and secret keys from the password hash
  //int rc = crypto_box_seed_keypair(
  return IUP_DEFAULT; 
} 

int main(int argc, char **argv) {
  IupOpen(&argc, &argv);
  IupSetLanguage("ENGLISH");
  IupSetGlobal("UTF8MODE", "Yes");

  f_in_text = IupText(NULL);
  f_out_text = IupText(NULL);
  pw_text = IupText(NULL);
  next_button = IupButton("Generate File", "next_button");
  IupSetAttributes(next_button, "ACTIVE = NO");

  Ihandle *in_button = IupButton("Browse", "btn_f_in"),
	  *out_button = IupButton("Browse", "btn_f_out"),
	  *in_label = IupLabel(" Input File: "),
	  *out_label = IupLabel(" Output File:"),
	  *pw_label = IupLabel(" Password:");

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
