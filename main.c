#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iup.h>



int main(int argc, char **argv)
{
    Ihandle *filedlg;
  IupOpen(&argc, &argv);
  IupSetLanguage("ENGLISH");
  IupSetGlobal("UTF8MODE", "Yes");

  char in_path[10240] = "", out_path[10240] = "";

  Ihandle *f_in_text = IupText(NULL), 
	  *in_button = IupButton("Browse", "btn_f_in"),
	  *f_out_text = IupText(NULL),
	  *out_button = IupButton("Browse", "btn_f_out"),
	  *next_button = IupButton("Next", "btn_next");
  
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
  IupMainLoop();

  /* Finishes IUP */
  IupClose();

  /* Program finished successfully */
  return EXIT_SUCCESS;

  IupClose();
  return EXIT_SUCCESS;
}
