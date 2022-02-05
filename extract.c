#include<stdio.h>
#include<string.h>
#include<stddef.h>
#include<sodium.h>
#include <iup.h>
#include "util.h"

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

  // read the length of the archive

  uint64_t file_size = 0;
  if (fseek(self, - (SENTINEL_LEN + sizeof(file_size)), SEEK_END) != 0){
	IupMessagef("Error", "Failed to seek in %s", argv[0]);
	goto cleanup;
  }

  if (fread(&file_size, 1, sizeof(file_size), self) != sizeof(file_size)){

	IupMessagef("Error", "Failed to read %s", argv[0]);
	goto cleanup;
  }

  //seek to the beginning of the stored file (not the .exe file)
  if (fseek(self, - (SENTINEL_LEN + sizeof(file_size) + file_size), SEEK_END) != 0){
	IupMessagef("Error", "Failed to seek in %s", argv[0]);
	goto cleanup;
  }
  
  // decrypt the achive

cleanup:
  fclose(self);
  return rc;
}
