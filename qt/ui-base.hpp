#define ProgramName "bsnes v073"
#define ProgramVersion "073"

#define UNICODE
#define QT_NO_DEBUG
#define QT_THREAD_SUPPORT

#include <QApplication>
#include <QtGui>
//Q_IMPORT_PLUGIN(QJpegPlugin)
//Q_IMPORT_PLUGIN(QMngPlugin)

#include <libsnes.hpp>

#define SNES_CARTRIDGE_MODE_NORMAL          0
#define SNES_CARTRIDGE_MODE_BSX_SLOTTED     1
#define SNES_CARTRIDGE_MODE_BSX             2
#define SNES_CARTRIDGE_MODE_SUFAMI_TURBO    3
#define SNES_CARTRIDGE_MODE_SUPER_GAME_BOY  4

#include <nall/algorithm.hpp>
#include <nall/any.hpp>
#include <nall/array.hpp>
#include <nall/detect.hpp>
#include <nall/dl.hpp>
#include <nall/endian.hpp>
#include <nall/file.hpp>
#include <nall/foreach.hpp>
#include <nall/function.hpp>
#include <nall/moduloarray.hpp>
#include <nall/platform.hpp>
#include <nall/priorityqueue.hpp>
#include <nall/property.hpp>
#include <nall/serializer.hpp>
#include <nall/stdint.hpp>
#include <nall/string.hpp>
#include <nall/utility.hpp>
#include <nall/varint.hpp>
#include <nall/vector.hpp>

#include <nall/base64.hpp>
#include <nall/config.hpp>
#include <nall/input.hpp>
#include <nall/ups.hpp>
#include <nall/snes/cartridge.hpp>
#include "template/concept.hpp"
#include "template/check-action.moc.hpp"
#include "template/file-dialog.moc.hpp"
#include "template/hex-editor.moc.hpp"
#include "template/radio-action.moc.hpp"
#include "template/window.moc.hpp"
using namespace nall;

#include <ruby/ruby.hpp>
using namespace ruby;

#include "config.hpp"
#include "interface.hpp"

#include "application/application.moc.hpp"

#include "base/about.moc.hpp"
#include "base/filebrowser.moc.hpp"
#include "base/htmlviewer.moc.hpp"
#include "base/loader.moc.hpp"
#include "base/main.moc.hpp"
#include "base/stateselect.moc.hpp"

#include "cartridge/cartridge.hpp"

#include "input/input.hpp"

#include "link/filter.hpp"
#include "link/reader.hpp"

#include "movie/movie.hpp"

#include "settings/settings.moc.hpp"
#include "settings/profile.moc.hpp"
#include "settings/video.moc.hpp"
#include "settings/audio.moc.hpp"
#include "settings/input.moc.hpp"
#include "settings/paths.moc.hpp"
#include "settings/advanced.moc.hpp"

#include "state/state.hpp"

#include "tools/tools.moc.hpp"
#include "tools/cheateditor.moc.hpp"
#include "tools/cheatfinder.moc.hpp"
#include "tools/statemanager.moc.hpp"
#include "tools/effecttoggle.moc.hpp"

#include "utility/utility.hpp"

struct Style {
  static const char Monospace[64];

  enum {
    WindowMargin     = 5,
    WidgetSpacing    = 5,
    SeparatorSpacing = 5,
  };
};

extern string filepath(const char *filename, const char *filepath);
