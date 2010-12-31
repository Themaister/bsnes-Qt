. qb/qb.libs.sh

check_switch_cxx GNU0X -std=gnu++0x
check_critical GNU0X "Cannot find C++0x compatible compiler."

check_lib_cxx LIBSNES $LIBSNES snes_init -ldl
check_critical LIBSNES "Cannot find libsnes."

add_define_make libsnes $LIBSNES

check_pkgconf QTCORE QtCore 
check_pkgconf QTGUI QtGui
check_critical QTCORE "Cannot find QtCore."
check_critical QTGUI "Cannot find QtGui."


check_lib ALSA -lasound snd_pcm_open
check_header OSS sys/soundcard.h
check_lib AL -lopenal alcOpenDevice
check_lib PULSE -lpulse pa_context_new
check_lib PULSE_SIMPLE -lpulse-simple pa_simple_new
check_lib AO -lao ao_open_live
check_lib XV -lXv XvShmCreateImage
check_lib GLX -lGL glXQueryVersion
check_lib SDL -lSDL SDL_Init

# Creates config.mk.
VARS="ALSA OSS AL PULSE PULSE_SIMPLE AO XV GLX SDL QTCORE QTGUI"
create_config_make config.mk $VARS

