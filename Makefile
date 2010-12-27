include nall/Makefile
include config.mk

ui := qt

# compiler
CC      += -std=gnu99
CXX     += -std=gnu++0x
flags   := -g -I.
link    := -lsnes
objects :=

# platform
ifeq ($(platform),x)
  link += -ldl -lX11 -lXext
else ifeq ($(platform),osx)
else ifeq ($(platform),win)
  link += -mwindows
# link += -mconsole
  link += -mthreads -s -luuid -lkernel32 -luser32 -lgdi32 -lcomctl32 -lcomdlg32 -lshell32 -lole32
  link += -enable-stdcall-fixup -Wl,-enable-auto-import -Wl,-enable-runtime-pseudo-reloc
else
  unknown_platform: help;
endif

# implicit rules
compile = \
  $(strip \
    $(if $(filter %.c,$<), \
      $(CC) $(flags) $1 -c $< -o $@, \
      $(if $(filter %.cpp,$<), \
        $(CXX) $(flags) $1 -c $< -o $@ \
      ) \
    ) \
  )

%.o: $<; $(call compile)

all: build;


qtlibs := $(strip QtCore QtGui $(if $(findstring osx,$(platform)),QtOpenGL))
include $(ui)/template/Makefile

ui_objects := ui-main ui-base ui-cartridge ui-input ui-movie ui-settings ui-state ui-tools
ui_objects += ruby
ui_objects += $(if $(call streq,$(platform),win),resource)
link += $(qtlib)



headers := $(call rwildcard,$(ui)/,%.hpp)
moc_headers := $(call rwildcard,nall/qt/,%.moc.hpp) $(call rwildcard,$(ui)/,%.moc.hpp)
moc_objects := $(foreach f,$(moc_headers),obj/$(notdir $(patsubst %.moc.hpp,%.moc,$f)))
qt_compile = $(call compile,-Iobj $(qtinc))

# platform
ifeq ($(platform),x)

  ruby := qtraster input.x

ifeq ($(HAVE_GLX),1)
  ruby += video.glx
endif
ifeq ($(HAVE_XV),1)
  ruby += video.xv
endif
ifeq ($(HAVE_SDL),1)
  ruby += video.sdl
endif
ifeq ($(HAVE_ALSA),1)
  ruby += audio.alsa
endif
ifeq ($(HAVE_AL),1)
  ruby += audio.openal
endif
ifeq ($(HAVE_OSS),1)
  ruby += audio.oss
endif
ifeq ($(HAVE_PULSE),1)
  ruby += audio.pulseaudio
endif
ifeq ($(HAVE_PULSE_SIMPLE),1)
  ruby += audio.pulseaudiosimple
endif
ifeq ($(HAVE_AO),1)
  ruby += audio.ao
endif
ifeq ($(HAVE_SDL),1)
  ruby += input.sdl
endif

  link += $(if $(findstring audio.openal,$(ruby)),-lopenal)
else ifeq ($(platform),osx)
  ruby := video.qtopengl video.qtraster
  ruby += audio.openal
  ruby += input.carbon

  link += $(if $(findstring audio.openal,$(ruby)),-framework OpenAL)
else ifeq ($(platform),win)
  ruby := video.direct3d video.wgl video.directdraw video.gdi video.qtraster
  ruby += audio.directsound audio.xaudio2
  ruby += input.rawinput input.directinput

  link += $(if $(findstring audio.openal,$(ruby)),-lopenal32)
else
  unknown_platform: help;
endif

# ruby
rubyflags := $(if $(finstring .sdl,$(ruby)),`sdl-config --cflags`)
rubyflags += $(if $(findstring .qt,$(ruby)),$(qtinc))

link += $(if $(findstring .sdl,$(ruby)),`sdl-config --libs`)
link += $(if $(findstring video.direct3d,$(ruby)),-ld3d9)
link += $(if $(findstring video.directdraw,$(ruby)),-lddraw)
link += $(if $(findstring video.glx,$(ruby)),-lGL)
link += $(if $(findstring video.wgl,$(ruby)),-lopengl32)
link += $(if $(findstring video.xv,$(ruby)),-lXv)
link += $(if $(findstring audio.alsa,$(ruby)),-lasound)
link += $(if $(findstring audio.ao,$(ruby)),-lao)
link += $(if $(findstring audio.directsound,$(ruby)),-ldsound)
link += $(if $(findstring audio.pulseaudio,$(ruby)),-lpulse)
link += $(if $(findstring audio.pulseaudiosimple,$(ruby)),-lpulse-simple)
link += $(if $(findstring input.directinput,$(ruby)),-ldinput8 -ldxguid)
link += $(if $(findstring input.rawinput,$(ruby)),-ldinput8 -ldxguid)

rubydef := $(foreach c,$(subst .,_,$(call strupper,$(ruby))),-D$c)

# rules
objects := $(ui_objects) $(objects)

# automatically run moc on all .moc.hpp (MOC header) files
%.moc: $<; $(moc) -i $< -o $@

# automatically generate %.moc build rules
__list = $(moc_headers)
$(foreach f,$(moc_objects), \
  $(eval __file = $(word 1,$(__list))) \
  $(eval __list = $(wordlist 2,$(words $(__list)),$(__list))) \
  $(eval $f: $(__file)) \
)

obj/ui-main.o: $(ui)/main.cpp $(headers) $(wildcard $(ui)/*.cpp) $(wildcard $(ui)/link/*.cpp) $(wildcard $(ui)/platform/*.cpp) $(wildcard $(ui)/utility/*.cpp); $(qt_compile)
obj/ui-base.o: $(ui)/base/base.cpp $(headers) $(wildcard $(ui)/base/*.cpp); $(qt_compile)
obj/ui-cartridge.o: $(ui)/cartridge/cartridge.cpp $(headers) $(wildcard $(ui)/cartridge/*.cpp); $(qt_compile)
obj/ui-input.o: $(ui)/input/input.cpp $(headers) $(wildcard $(ui)/input/*.cpp); $(qt_compile)
obj/ui-movie.o: $(ui)/movie/movie.cpp $(headers) $(wildcard $(ui)/movie/*.cpp); $(qt_compile)
obj/ui-settings.o: $(ui)/settings/settings.cpp $(headers) $(wildcard $(ui)/settings/*.cpp); $(qt_compile)
obj/ui-state.o: $(ui)/state/state.cpp $(headers) $(wildcard $(ui)/state/*.cpp); $(qt_compile)
obj/ui-tools.o: $(ui)/tools/tools.cpp $(headers) $(wildcard $(ui)/tools/*.cpp); $(qt_compile)

obj/ruby.o: ruby/ruby.cpp $(call rwildcard,ruby/*) config.mk
	$(call compile,$(rubydef) $(rubyflags))

obj/resource.rcc: $(ui)/resource/resource.qrc $(ui)/data/*
	$(rcc) $(ui)/resource/resource.qrc -o obj/resource.rcc

obj/resource.o: $(ui)/resource/resource.rc
	windres $(ui)/resource/resource.rc obj/resource.o

# targets
ui_build: obj/resource.rcc $(moc_objects);
ui_clean:
	-@$(call delete,obj/*.rcc)
	-@$(call delete,obj/*.moc)

#####

objects := $(patsubst %,obj/%.o,$(objects))

# targets
build: ui_build $(objects) config.mk
ifeq ($(platform),osx)
	test -d ../bsnes.app || mkdir -p ../bsnes.app/Contents/MacOS
	$(strip $(CXX) -o ../bsnes.app/Contents/MacOS/bsnes $(objects) $(link) $(LDFLAGS))
else
	$(strip $(CXX) -o out/bsnes $(objects) $(link) $(LDFLAGS))
endif

config.mk: configure qb/*
	@echo Rebuild config.mk with ./configure! && /bin/false

install:
ifeq ($(platform),x)
	install -D -m 755 out/bsnes $(DESTDIR)$(PREFIX)/bin/bsnes
	install -D -m 644 data/bsnes.png $(DESTDIR)$(PREFIX)/share/pixmaps/bsnes.png
	install -D -m 644 data/bsnes.desktop $(DESTDIR)$(PREFIX)/share/applications/bsnes.desktop
	test -d ~/.bsnes || mkdir ~/.bsnes
	cp data/cheats.xml ~/.bsnes/cheats.xml
	chmod 777 ~/.bsnes ~/.bsnes/cheats.xml
endif

uninstall:
ifeq ($(platform),x)
	rm $(DESTDIR)$(PREFIX)/bin/bsnes
	rm $(DESTDIR)$(PREFIX)/share/pixmaps/bsnes.png
	rm $(DESTDIR)$(PREFIX)/share/applications/bsnes.desktop
endif

clean: ui_clean
	-@$(call delete,obj/*.o)
	-@$(call delete,obj/*.a)
	-@$(call delete,obj/*.so)
	-@$(call delete,obj/*.dylib)
	-@$(call delete,obj/*.dll)
	-@$(call delete,*.res)
	-@$(call delete,*.pgd)
	-@$(call delete,*.pgc)
	-@$(call delete,*.ilk)
	-@$(call delete,*.pdb)
	-@$(call delete,*.manifest)

help:;
