#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <iup.h>


static char in_path[10240] = "", out_path[10240] = "";

static Ihandle *f_in_text = NULL, *f_out_text = NULL, *pw_text = NULL, *next_button = NULL; 

static bool has_in_path = false, has_out_path = false;

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
  return IUP_DEFAULT;
}


int encrypt_archive_cb(Ihandle *self){

  return IUP_DEFAULT;
}

int main(int argc, char **argv)
{
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
