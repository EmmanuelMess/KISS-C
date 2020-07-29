
APPNAME=kissc
RAWDRAWANDROID=rawdrawandroid
CFLAGS:=-I. -ffunction-sections -Os -fvisibility=hidden -DRDALOGFNCB=example_log_function
LDFLAGS:=-s
PACKAGENAME?=com.emmanuelmess.$(APPNAME)
SRC:=test.c

ANDROIDVERSION=22
ANDROIDTARGET=28

MINIZ:=spng/miniz/miniz.c
SPNG:=spng/spng.c $(MINIZ)
SRC:=test.c $(SPNG)
TARGETS:=makecapk/lib/arm64-v8a/lib$(APPNAME).so makecapk/lib/armeabi-v7a/lib$(APPNAME).so # makecapk/lib/x86/lib$(APPNAME).so makecapk/lib/x86_64/lib$(APPNAME).so

include rawdrawandroid/Makefile


