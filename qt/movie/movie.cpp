#include "../ui-base.hpp"

Movie movie;

void Movie::chooseFile() {
  fileBrowser->onChange.reset();
  fileBrowser->onActivate = { &Movie::play, this };
  fileBrowser->onAccept = { &Movie::play, this };
  fileBrowser->setWindowTitle("Select Movie");
  fileBrowser->setPath(config().path.current.movie);
  fileBrowser->setNameFilters("bsnes Movies (*.bsv)");
  fileBrowser->chooseFile();
}

void Movie::play(const string &filename) {
  if(QDir(filename).exists()) return;  //ignore folders
  config().path.current.movie = dir(filename);

  if(Movie::state != Inactive) stop();

  if(fp.open(filename, file::mode::read)) {
    if(fp.size() < 32) goto corrupt;

    unsigned signature = fp.readm(4);
    if(signature != 0x42535631) goto corrupt;

    unsigned version = fp.readl(4);
  //if(version != SNES::Info::SerializerVersion) goto corrupt;

    unsigned crc32 = fp.readl(4);
  //if(crc32 != SNES::cartridge.crc32()) goto corrupt;

    unsigned size = fp.readl(4);
    uint8_t *data = new uint8_t[size];
    fp.read(data, size);
    snes_unserialize(data, size);
    delete[] data;

    Movie::state = Playback;
    mainWindow->syncUi();
    utility.showMessage("Playback started.");

    return;
  }

corrupt:
  fp.close();
  utility.showMessage("Movie file is invalid, playback cancelled.");
}

void Movie::record() {
  if(Movie::state != Inactive) {
    utility.showMessage("Movie mode already active, recording cancelled.");
  } else {
    uint8_t *data;
    unsigned size;
    snes_serialize(data, size);

    utility.showMessage("Recording started.");

    Movie::state = Record;
    mainWindow->syncUi();
    fp.open(makeFilename(), file::mode::write);
    fp.writem(0x42535631, 4);
    fp.writel(15 /* SNES::Info::SerializerVersion */, 4);
    fp.writel(0 /* SNES::cartridge.crc32() */, 4);
    fp.writel(size, 4);
    fp.write(data, size);

    delete[] data;
  }
}

void Movie::stop() {
  if(Movie::state != Inactive) {
    Movie::state = Inactive;
    mainWindow->syncUi();
    fp.close();
    utility.showMessage("Recording / playback stopped.");
  }
}

string Movie::makeFilename() const {
  string filename = nall::basename(cartridge.fileName);

  time_t systemTime = time(0);
  tm *currentTime = localtime(&systemTime);
  char t[512];
  sprintf(t, "%.4u%.2u%.2u-%.2u%.2u%.2u",
    1900 + currentTime->tm_year, 1 + currentTime->tm_mon, currentTime->tm_mday,
    currentTime->tm_hour, currentTime->tm_min, currentTime->tm_sec
  );
  filename << "-" << t << ".bsv";

  return filepath(filename, config().path.data);
}

int16_t Movie::read() {
  int16_t result = fp.readl(2);

  if(fp.end()) {
    Movie::state = Inactive;
    mainWindow->syncUi();
    fp.close();
    utility.showMessage("Playback finished.");
  }

  return result;
}

void Movie::write(int16_t value) {
  fp.writel(value, 2);
}

Movie::Movie() {
  state = Inactive;
}
