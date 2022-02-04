#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iup.h>


static char in_path[10240] = "", out_path[10240] = "";

static Ihandle *f_in_text = NULL, *f_out_text = NULL; 

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
      break;
  }
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
      strncpy(out_path, IupGetAttribute(dlg, "VALUE"), sizeof(out_path));
      IupSetAttribute(f_out_text, "VALUE", out_path);
      IupSetAttributes(f_out_text, "READONLY = YES, EXPAND = YES, SIZE = 4x8");
      IupRefresh(f_out_text);
      break;
  }
  return IUP_DEFAULT;
}

int main(int argc, char **argv)
{
    Ihandle *filedlg;
  IupOpen(&argc, &argv);
  IupSetLanguage("ENGLISH");
  IupSetGlobal("UTF8MODE", "Yes");

  f_in_text = IupText(NULL);
  f_out_text = IupText(NULL);
  Ihandle *in_button = IupButton("Browse", "btn_f_in"),
	  *out_button = IupButton("Browse", "btn_f_out"),
	  *next_button = IupButton("Next", "btn_next"),
	  *in_label = IupLabel("Input File"),
	  *out_label = IupLabel("Output File");

  IupSetAttributes(in_label, "PADDING = 3x3");
  IupSetAttributes(out_label, "PADDING = 3x3");
  IupSetCallback( out_button, "ACTION", (Icallback)get_out_path);

  IupSetCallback( in_button, "ACTION", (Icallback)get_in_path);
  
  IupSetAttributes(f_in_text, "READONLY = YES, EXPAND = YES, SIZE = 4x8");
  IupSetAttribute(f_out_text, "READONLY", "YES");
  IupSetAttribute(f_out_text, "EXPAND", "YES");

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
		next_button,
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
