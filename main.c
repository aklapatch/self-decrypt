#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iup.h>


static char in_path[10240] = "", out_path[10240] = "";

int get_out_path(Ihandle *self){
  Ihandle *dlg = IupFileDlg();
  IupSetAttributes(dlg, "DIALOGTYPE = SAVE, TITLE = \"Archive Save\"");
  IupSetAttributes(dlg, "FILTER = \"*.exe\", FILTERINFO = \"Executable Files\"");
  IupPopup(dlg, IUP_CENTER, IUP_CENTER);
  switch(IupGetInt(dlg, "STATUS"))
  {
    case 1:
      memset(out_path, 0, sizeof(out_path));
      IupMessage("New file",IupGetAttribute(dlg, "VALUE"));
      strncpy(out_path, IupGetAttribute(dlg, "VALUE"), sizeof(out_path));
      printf("out_file = %s\n", out_path);
      break;

    case 0 :
      IupMessage("File already exists",IupGetAttribute(dlg, "VALUE"));
      break;

    case -1 :
      IupMessage("IupFileDlg","Operation Canceled");
      return 1;
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


  Ihandle *f_in_text = IupText(NULL), 
	  *in_button = IupButton("Browse", "btn_f_in"),
	  *f_out_text = IupText(NULL),
	  *out_button = IupButton("Browse", "btn_f_out"),
	  *next_button = IupButton("Next", "btn_next");

  IupSetCallback( out_button, "ACTION", (Icallback)get_out_path);
  
  IupSetAttribute(f_in_text, "READONLY", "YES");
  IupSetAttribute(f_out_text, "READONLY", "YES");

  Ihandle *dlg = IupDialog(
	      IupVbox(
		IupHbox(
		  f_in_text,
		  in_button,
		  NULL),
		IupHbox(
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

  IupClose();
  return EXIT_SUCCESS;
}
