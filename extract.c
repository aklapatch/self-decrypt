#include<stdio.h>
#include<string.h>
#include<stddef.h>
#include<sodium.h>
#include <iup.h>
#include "util.h"

static Ihandle *pw_text= NULL, *decrypt_button = NULL;

static char self_name[4096] = {0};

int decrypt_cb(Ihandle *self){
  // read the length of the archive

  FILE *self = fopen(self_name, "rb");
  if (!self){
    IupMessagef("Error", "Failed Opening %s for extraction!", self_name);
    return IUP_DEFAULT;
  }

  uint64_t file_size = 0;
  if (fseek(self, - (SENTINEL_LEN + sizeof(file_size)), SEEK_END) != 0){
	IupMessagef("Error", "Failed to seek in %s", argv[0]);
	goto closein;
  }

  if (fread(&file_size, 1, sizeof(file_size), self) != sizeof(file_size)){
	IupMessagef("Error", "Failed to read %s", argv[0]);
	goto closein;
  }

  //seek to the beginning of the stored file (not the .exe file)
  if (fseek(self, - (SENTINEL_LEN + sizeof(file_size) + file_size), SEEK_END) != 0){
	IupMessagef("Error", "Failed to seek in %s", argv[0]);
	goto closein;
  }
  
  FILE *fout = fopen("decrypted.txt", "wb");
  if (fout == NULL){
	IupMessagef("Error", "Failed to open output file!");
	goto closein;
  }

  // decrypt the achive and write it out
  uint8_t file_buf[FREAD_BYTES] = {0};

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
	goto cleanup;
  }

  char sentinel[SENTINEL_LEN + 1];
  memset(sentinel, 0, sizeof(sentinel));
  size_t b_read = fread(sentinel, 1, SENTINEL_LEN, self);
  if (b_read != SENTINEL_LEN){
	IupMessagef("Error", "Failed to read %s", argv[0]);
	goto cleanup;
  }

  if (strncmp(sentinel, END_SENTINEL, SENTINEL_LEN) != 0){
	IupMessagef("Error", "File sentinel not found %s\nIs file attached?", argv[0]);
	goto cleanup;
  }
  // close the file for now. We'll open it if we need to later.
  fclose(self);

  // copy the path of the exe into this buffer so we can use it later.
  strncpy(self_name, argv[0], sizeof(self_name) - 1);

  // set up the window to ask for a password
  pw_text = IupText(NULL);
  decrypt_button = IupButton("Decrypt File", "decrypt_button");
  IupSetCallback(decrypt_button, (Icallback)decrypt_cb);
  Ihandle *pw_label = IupLabel("Enter Password:");

  IupSetAttributes(pw_text, "EXPAND = HORIZONTAL, SIZE = 150x, PADDING = 10x10, PASSWORD = YES");

  Ihandle *dlg = IupDialog(
      IupHbox(
	pw_label,
	pw_text,
	decrypt_button,
	NULL),
      NULL);
  IupShowXY(dlg, IUP_CENTER, IUP_CENTER);
  IupSetAttributes(dlg, "EXPAND = YES, TITLE = extract encrypted file");
  IupMainLoop();

  IupClose();

  return EXIT_SUCCESS;
}
