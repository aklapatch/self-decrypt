PROJNAME = iup
APPNAME = iupluascripter
APPTYPE = windows

STRIP = 
OPT = YES      
NO_SCRIPTS = Yes
# IM and IupPlot uses C++
LINKER = $(CPPC)
# To use a subfolder with the Lua version for binaries
LUAMOD_DIR = Yes

ifdef USE_LUA_VERSION
  USE_LUA51:=
  USE_LUA52:=
  USE_LUA53:=
  USE_LUA54:=
  ifeq ($(USE_LUA_VERSION), 54)
    USE_LUA54:=Yes
  endif
  ifeq ($(USE_LUA_VERSION), 53)
    USE_LUA53:=Yes
  endif
  ifeq ($(USE_LUA_VERSION), 52)
    USE_LUA52:=Yes
  endif
  ifeq ($(USE_LUA_VERSION), 51)
    USE_LUA51:=Yes
  endif
endif

ifdef USE_LUA54
  LUASFX = 54
  ifneq ($(findstring SunOS, $(TEC_UNAME)), )
    ifneq ($(findstring x86, $(TEC_UNAME)), )
      FLAGS = -std=gnu99
    endif
  endif
else
ifdef USE_LUA53
  LUASFX = 53
  ifneq ($(findstring SunOS, $(TEC_UNAME)), )
    ifneq ($(findstring x86, $(TEC_UNAME)), )
      FLAGS = -std=gnu99
    endif
  endif
else
ifdef USE_LUA52
  LUASFX = 52
else
  USE_LUA51 = Yes
  LUASFX = 51
endif
endif
endif

APPNAME := $(APPNAME)$(LUASFX)
SRC = iupluascripter.c

INCLUDES = ../include

IUP = ..
USE_IUP = Yes
USE_IUPLUA = Yes

ifdef DBG
  ALL_STATIC=Yes
endif

ifdef ALL_STATIC
  # Statically link everything only when debugging
  USE_STATIC = Yes
  
  ifdef DBG_DIR
    IUP_LIB = $(IUP)/lib/$(TEC_UNAME)d
    CD_LIB = $(CD)/lib/$(TEC_UNAME)d
    IM_LIB = $(IM)/lib/$(TEC_UNAME)d
#    LUA_LIB = $(LUA)/lib/$(TEC_UNAME)d
  else
    IUP_LIB = $(IUP)/lib/$(TEC_UNAME)
    CD_LIB = $(CD)/lib/$(TEC_UNAME)
    IM_LIB = $(IM)/lib/$(TEC_UNAME)
#    LUA_LIB = $(LUA)/lib/$(TEC_UNAME)
  endif  
  
  ifneq ($(findstring Win, $(TEC_SYSNAME)), )
    LIBS += iupluascripterdlg$(LUASFX) iupimglib iup_scintilla imm32
  else
    SLIB += $(IUP_LIB)/Lua$(LUASFX)/libiupluascripterdlg$(LUASFX).a \
            $(IUP_LIB)/libiup_scintilla.a \
            $(IUP_LIB)/libiupimglib.a
  endif
  
  DEFINES += USE_STATIC

  ifneq ($(findstring Win, $(TEC_SYSNAME)), )
    LIBS += iuplua_scintilla$(LUASFX)
  else
    SLIB += $(IUP_LIB)/Lua$(LUASFX)/libiuplua_scintilla$(LUASFX).a
  endif
  
  ifeq "$(TEC_UNAME)" "SunOS510x86"
    IUPLUA_NO_GL = Yes
  endif
    
  #IUPLUA_NO_GL = Yes
  ifndef IUPLUA_NO_GL 
    USE_OPENGL = Yes
    USE_IUPGLCONTROLS = Yes
  else
    DEFINES += IUPLUA_NO_GL
  endif

  #IUPLUA_NO_CD = Yes
  ifndef IUPLUA_NO_CD 
    USE_CDLUA = Yes
    USE_IUPCONTROLS = Yes
    ifneq ($(findstring Win, $(TEC_SYSNAME)), )
      LIBS += iuplua_plot$(LUASFX) iup_plot cdgl cdcontextplus
    else
      SLIB += $(IUP_LIB)/Lua$(LUASFX)/libiuplua_plot$(LUASFX).a $(IUP_LIB)/libiup_plot.a $(CD_LIB)/libcdgl.a $(CD_LIB)/libcdcontextplus.a
    endif
      
    ifndef IUPLUA_NO_IM
      ifneq ($(findstring Win, $(TEC_SYSNAME)), )
        LIBS += cdluaim$(LUASFX) cdim$(LUASFX)
      else
        SLIB += $(CD_LIB)/Lua$(LUASFX)/libcdluaim$(LUASFX).a $(CD_LIB)/libcdim.a
      endif
    endif
  else
    DEFINES += IUPLUA_NO_CD
  endif

  #IUPLUA_NO_IM = Yes
  ifndef IUPLUA_NO_IM
    USE_IMLUA = Yes
    ifneq ($(findstring Win, $(TEC_SYSNAME)), )
      LIBS += imlua_process$(LUASFX) iupluaim$(LUASFX) im_process iupim
    else
      SLIB += $(IM_LIB)/Lua$(LUASFX)/libimlua_process$(LUASFX).a $(IUP_LIB)/Lua$(LUASFX)/libiupluaim$(LUASFX).a $(IM_LIB)/libim_process.a $(IUP_LIB)/libiupim.a
    endif
  else
    DEFINES += IUPLUA_NO_IM
  endif

  IUPLUA_IMGLIB = Yes
  ifdef IUPLUA_IMGLIB
    DEFINES += IUPLUA_IMGLIB
    ifneq ($(findstring Win, $(TEC_SYSNAME)), )
      LIBS += iupluaimglib$(LUASFX)
    else
      SLIB += $(IUP_LIB)/Lua$(LUASFX)/libiupluaimglib$(LUASFX).a
    endif
  endif
  
  IUPLUA_WEB = Yes
  ifdef IUPLUA_WEB
    DEFINES += IUPLUA_WEB
    USE_IUPWEB = Yes
  endif
  
  #IUPLUA_TUIO = Yes
  ifdef IUPLUA_TUIO
    DEFINES += IUPLUA_TUIO
    ifneq ($(findstring Win, $(TEC_SYSNAME)), )
      LIBS += iupluatuio$(LUASFX) iuptuio
      LIBS += ws2_32 winmm
    else
      SLIB += $(IUP_LIB)/Lua$(LUASFX)/libiupluatuio$(LUASFX).a $(IUP_LIB)/libiuptuio.a
    endif
  endif
else
  ifneq ($(findstring Win, $(TEC_SYSNAME)), )
    # Dinamically link in Windows, when not debugging
    # Must call "tecmake dll10" so USE_* will use the correct TEC_UNAME
    USE_DLL = Yes
    GEN_MANIFEST = No
    LIBS += iupluascripterdlg$(LUASFX) iupimglib iup_scintilla imm32
  else
    LDIR += $(IUP_LIB)/Lua$(LUASFX)
    LIBS += iupluascripterdlg$(LUASFX) iupimglib iup_scintilla
    ifneq ($(findstring cygw, $(TEC_UNAME)), )
      # Except in Cygwin
    else
      # In UNIX Lua is always statically linked, late binding is used.
      NO_LUALINK = Yes
      SLIB += $(LUA_LIB)/liblua$(LUA_SFX).a
    endif
  endif
endif

ifneq ($(findstring Win, $(TEC_SYSNAME)), )
  #Comment the following line to build under MingW
  ifneq ($(findstring vc, $(TEC_UNAME)), )
    SLIB += setargv.obj
  endif
  SRC += iupluascripter.rc
  INCLUDES = ../etc
endif

ifneq ($(findstring cygw, $(TEC_UNAME)), )
endif

ifneq ($(findstring MacOS, $(TEC_UNAME)), )
endif

ifneq ($(findstring Linux, $(TEC_UNAME)), )
  LIBS += dl 
  #To allow late binding
  LFLAGS = -Wl,-E
  ifneq ($(findstring Linux26g4_64co, $(TEC_UNAME)), )
    LIBS += gthread-2.0
  endif
endif

ifneq ($(findstring BSD, $(TEC_UNAME)), )
  #To allow late binding
  LFLAGS = -Wl,-E
endif

ifneq ($(findstring SunOS, $(TEC_UNAME)), )
  LIBS += dl
endif
