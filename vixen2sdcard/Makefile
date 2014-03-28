#OS = LINUX
#OS = MACOSX
OS = WINDOWS

PROG = vixen2sdcard

ifeq ($(OS), LINUX)
TARGET = $(PROG)
FINAL_TARGET = $(TARGET)
CXX = g++
STRIP = strip
WXCONFIG = ~/wxwidgets/2.8.12.gtk2.teensy/bin/wx-config
CPPFLAGS = -O2 -Wall -Wno-strict-aliasing `$(WXCONFIG) --cppflags` -D$(OS)
LIBS = `$(WXCONFIG) --libs`

else ifeq ($(OS), MACOSX)
TARGET = $(PROG)
FINAL_TARGET = $(PROG).dmg
SDK = /Developer/SDKs/MacOSX10.5.sdk
CXX = g++
STRIP = strip
WXCONFIG = ~/wxwidgets/2.8.10.mac.teensy/bin/wx-config
CPPFLAGS =  -O2 -Wall -Wno-strict-aliasing -isysroot $(SDK) `$(WXCONFIG) --cppflags` -D$(OS) -arch ppc -arch i386
LIBS = -Xlinker -syslibroot -Xlinker $(SDK) `$(WXCONFIG) --libs`

else ifeq ($(OS), WINDOWS)
TARGET = $(PROG).exe
FINAL_TARGET = $(TARGET)
CXX = i686-w64-mingw32-g++
STRIP = i686-w64-mingw32-strip
WINDRES = i686-w64-mingw32-windres
WXCONFIG = ~/wxwidgets/2.8.10.mingw.teensy/bin/wx-config
KEY_P12 = ~/bin/cert/mykey.p12
KEY_PASS = myname
KEY_URL = http://www.pjrc.com/teensy/
KEY_TS = http://timestamp.verisign.com/scripts/timstamp.dll
CPPFLAGS =  -O2 -Wall -Wno-strict-aliasing `$(WXCONFIG) --cppflags` -D$(OS)
LIBS = `$(WXCONFIG) --libs` -static-libgcc -static-libstdc++
endif

MAKEFLAGS = --jobs=4

OBJS = vixen2sdcard.o

all: $(FINAL_TARGET)

$(PROG): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LIBS)
	$(STRIP) $@

$(PROG).exe: $(PROG)
	cp $(PROG) $@ 
	-upx -q $@
	osslsigncode sign -pkcs12 $(KEY_P12) -pass $(KEY_PASS) -i $(KEY_URL) -n "vixen2sdcard" -t $(KEY_TS) -in $@ -out $@.signed
	mv $@.signed $@
	#-cp_win32.sh $@

$(PROG).app: $(PROG) Info.plist
	mkdir -p $(PROG).app/Contents/MacOS
	mkdir -p $(PROG).app/Contents/Resources/English.lproj
	cp Info.plist $(PROG).app/Contents/Info.plist
	/bin/echo -n 'APPL????' > $(PROG).app/Contents/PkgInfo
	cp $(PROG) $(PROG).app/Contents/MacOS/
	touch $(PROG).app

$(PROG).dmg: $(PROG).app
	mkdir dmg_tmpdir
	cp -r $(PROG).app dmg_tmpdir
	hdiutil create -ov -srcfolder dmg_tmpdir -format UDBZ -volname "vixen2sdcard" $(PROG).dmg

clean:
	rm -f $(PROG) $(PROG).exe $(PROG).exe.bak $(PROG).dmg *.o
	rm -rf $(PROG).app dmg_tmpdir

