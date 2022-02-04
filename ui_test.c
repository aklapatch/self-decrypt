#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iup.h>

int main(int argc, char **argv)
{
	  Ihandle *filedlg;
  IupOpen(&argc, &argv);
  IupSetLanguage("ENGLISH");

  filedlg = IupFileDlg();

  IupSetAttributes(filedlg, "DIALOGTYPE = SAVE, TITLE = \"File Save\"");
  IupSetAttributes(filedlg, "FILTER = \"*\", FILTERINFO = \"All Files\"");

  IupPopup(filedlg, IUP_CENTER, IUP_CENTER);

  switch(IupGetInt(filedlg, "STATUS"))
  {
    case 1:
      IupMessage("New file",IupGetAttribute(filedlg, "VALUE"));
      break;

    case 0 :
      IupMessage("File already exists",IupGetAttribute(filedlg, "VALUE"));
      break;

    case -1 :
      IupMessage("IupFileDlg","Operation Canceled");
      return 1;
      break;
  }

  IupDestroy(filedlg);
  IupClose();
  return EXIT_SUCCESS;
}
