. qb/qb.params.sh

PACKAGE_NAME=bsnes
PACKAGE_VERSION=v073

# Adds a command line opt to ./configure --help
# $1: Variable (HAVE_ALSA, HAVE_OSS, etc)   
# $2: Comment                 
# $3: Default arg. auto implies that HAVE_ALSA will be set according to library checks later on.
add_command_line_string LIBSNES "Path to libsnes library." "-lsnes"
add_command_line_enable XV "Enable XVideo support" auto
add_command_line_enable GLX "Enable OpenGL video support" auto
add_command_line_enable ALSA "Enable ALSA support" auto
add_command_line_enable OSS "Enable OSS support" auto
add_command_line_enable PULSE "Enable PulseAudio support" auto
add_command_line_enable PULSE_SIMPLE "Enable PulseAudio simple support" auto
add_command_line_enable AO "Enable libao support" auto
add_command_line_enable AL "Enable OpenAL support" auto
add_command_line_enable SDL "Enable SDL support" auto
