#include "../ui-base.hpp"
Cartridge cartridge;

//================
//public functions
//================

bool Cartridge::information(const char *filename, Cartridge::Information &info) {
  if(extension(filename) != "sfc") return false;  //do not parse compressed images

  file fp;
  if(fp.open(filename, file::mode::read) == false) return false;

  unsigned offset = 0;
  if((fp.size() & 0x7fff) == 512) offset = 512;

  uint16_t complement, checksum;

  fp.seek(0x7fdc + offset);
  complement = fp.readl(2);
  checksum = fp.readl(2);

  unsigned header = offset + (complement + checksum == 65535 ? 0x7fb0 : 0xffb0);

  fp.seek(header + 0x10);
  char name[22];
  fp.read((uint8_t*)name, 21);
  name[21] = 0;
  info.name = decodeShiftJIS(name);

  fp.seek(header + 0x29);
  uint8_t region = fp.read();
  info.region = (region <= 1 || region >= 13) ? "NTSC" : "PAL";

  info.romSize = fp.size() & ~0x7fff;

  fp.seek(header + 0x28);
  info.ramSize = fp.readl(1);
  if(info.ramSize) info.ramSize = 1024 << (info.ramSize & 7);

  fp.close();
  return true;
}

bool Cartridge::saveStatesSupported() {
  if(application.cartridgeMode == SNES_CARTRIDGE_MODE_BSX) return false;

//if(SNES::cartridge.has_st0011()) return false;
//if(SNES::cartridge.has_st0018()) return false;
//if(SNES::cartridge.has_serial()) return false;

  return true;
}

bool Cartridge::loadNormal(const char *base) {
  unload();

  uint8_t *romData;
  unsigned romSize;

  if(loadCartridge(baseName = base, cartridge.baseXml, romData, romSize) == false) return false;
  snes_set_cartridge_basename(nall::basename(baseName));

  application.cartridgeMode = SNES_CARTRIDGE_MODE_NORMAL;
  snes_load_cartridge_normal(cartridge.baseXml, romData, romSize);
  if(romData) delete[] romData;

  loadMemory(baseName, ".srm", SNES_MEMORY_CARTRIDGE_RAM);
  loadMemory(baseName, ".rtc", SNES_MEMORY_CARTRIDGE_RTC);

  fileName = baseName;
  name = notdir(nall::basename(baseName));

  utility.modifySystemState(Utility::LoadCartridge);
  return true;
}

bool Cartridge::loadBsxSlotted(const char *base, const char *slot) {
  unload();

  uint8_t *romData, *bsxData;
  unsigned romSize,  bsxSize;

  if(loadCartridge(baseName = base, cartridge.baseXml, romData, romSize) == false) return false;
  loadCartridge(slotAName = slot, cartridge.slotAXml, bsxData, bsxSize);
  snes_set_cartridge_basename(nall::basename(baseName));

  application.cartridgeMode = SNES_CARTRIDGE_MODE_BSX_SLOTTED;
  snes_load_cartridge_bsx_slotted(
    cartridge.baseXml,  romData, romSize,
    cartridge.slotAXml, bsxData, bsxSize
  );
  if(romData) delete[] romData;
  if(bsxData) delete[] bsxData;

  loadMemory(baseName, ".srm", SNES_MEMORY_CARTRIDGE_RAM);
  loadMemory(baseName, ".rtc", SNES_MEMORY_CARTRIDGE_RTC);

  fileName = baseName;
  name = notdir(nall::basename(baseName));
  if(*slot) name << " + " << notdir(nall::basename(slotAName));

  utility.modifySystemState(Utility::LoadCartridge);
  return true;
}

bool Cartridge::loadBsx(const char *base, const char *slot) {
  unload();

  uint8_t *romData, *bsxData;
  unsigned romSize,  bsxSize;

  if(loadCartridge(baseName = base, cartridge.baseXml, romData, romSize) == false) return false;
  loadCartridge(slotAName = slot, cartridge.slotAXml, bsxData, bsxSize);
  snes_set_cartridge_basename(nall::basename(baseName));

  application.cartridgeMode = SNES_CARTRIDGE_MODE_BSX;
  snes_load_cartridge_bsx(
    cartridge.baseXml,  romData, romSize,
    cartridge.slotAXml, bsxData, bsxSize
  );
  if(romData) delete[] romData;
  if(bsxData) delete[] bsxData;

  loadMemory(baseName, ".srm", SNES_MEMORY_BSX_RAM );
  loadMemory(baseName, ".psr", SNES_MEMORY_BSX_PRAM);

  fileName = slotAName;
  name = *slot
  ? notdir(nall::basename(slotAName))
  : notdir(nall::basename(baseName));

  utility.modifySystemState(Utility::LoadCartridge);
  return true;
}

bool Cartridge::loadSufamiTurbo(const char *base, const char *slotA, const char *slotB) {
  unload();

  uint8_t *romData, *staData, *stbData;
  unsigned romSize,  staSize,  stbSize;

  if(loadCartridge(baseName = base, cartridge.baseXml, romData, romSize) == false) return false;
  loadCartridge(slotAName = slotA, cartridge.slotAXml, staData, staSize);
  loadCartridge(slotBName = slotB, cartridge.slotBXml, stbData, stbSize);
  snes_set_cartridge_basename(nall::basename(baseName));

  application.cartridgeMode = SNES_CARTRIDGE_MODE_SUFAMI_TURBO;
  snes_load_cartridge_sufami_turbo(
    cartridge.baseXml,  romData, romSize,
    cartridge.slotAXml, staData, staSize,
    cartridge.slotBXml, stbData, stbSize
  );
  if(romData) delete[] romData;
  if(staData) delete[] staData;
  if(stbData) delete[] stbData;

  loadMemory(slotAName, ".srm", SNES_MEMORY_SUFAMI_TURBO_A_RAM);
  loadMemory(slotBName, ".srm", SNES_MEMORY_SUFAMI_TURBO_B_RAM);

  fileName = slotAName;
  if(!*slotA && !*slotB) name = notdir(nall::basename(baseName));
  else if(!*slotB) name = notdir(nall::basename(slotAName));
  else if(!*slotA) name = notdir(nall::basename(slotBName));
  else name = notdir(nall::basename(slotAName)) << " + " << notdir(nall::basename(slotBName));

  utility.modifySystemState(Utility::LoadCartridge);
  return true;
}

bool Cartridge::loadSuperGameBoy(const char *base, const char *slot) {
  unload();

  uint8_t *romData, *dmgData;
  unsigned romSize,  dmgSize;

  if(loadCartridge(baseName = base, cartridge.baseXml, romData, romSize) == false) return false;
  loadCartridge(slotAName = slot, cartridge.slotAXml, dmgData, dmgSize);
  snes_set_cartridge_basename(nall::basename(baseName));

  application.cartridgeMode = SNES_CARTRIDGE_MODE_SUPER_GAME_BOY;
  snes_load_cartridge_super_game_boy(
    cartridge.baseXml,  romData, romSize,
    cartridge.slotAXml, dmgData, dmgSize
  );
  if(romData) delete[] romData;
  if(dmgData) delete[] dmgData;

  loadMemory(slotAName, ".sav", SNES_MEMORY_GAME_BOY_RAM);
  loadMemory(slotBName, ".rtc", SNES_MEMORY_GAME_BOY_RTC);

  fileName = slotAName;
  name = *slot
  ? notdir(nall::basename(slotAName))
  : notdir(nall::basename(baseName));

  utility.modifySystemState(Utility::LoadCartridge);
  return true;
}

void Cartridge::saveMemory() {
  if(application.cartridgeLoaded == false) return;

  switch(application.cartridgeMode) {
    case SNES_CARTRIDGE_MODE_NORMAL:
    case SNES_CARTRIDGE_MODE_BSX_SLOTTED: {
      saveMemory(baseName, ".srm", SNES_MEMORY_CARTRIDGE_RAM);
      saveMemory(baseName, ".rtc", SNES_MEMORY_CARTRIDGE_RTC);
    } break;

    case SNES_CARTRIDGE_MODE_BSX: {
      saveMemory(baseName, ".srm", SNES_MEMORY_BSX_RAM );
      saveMemory(baseName, ".psr", SNES_MEMORY_BSX_PRAM);
    } break;

    case SNES_CARTRIDGE_MODE_SUFAMI_TURBO: {
      saveMemory(slotAName, ".srm", SNES_MEMORY_SUFAMI_TURBO_A_RAM);
      saveMemory(slotBName, ".srm", SNES_MEMORY_SUFAMI_TURBO_B_RAM);
    } break;

    case SNES_CARTRIDGE_MODE_SUPER_GAME_BOY: {
      saveMemory(slotAName, ".sav", SNES_MEMORY_GAME_BOY_RAM);
      saveMemory(slotAName, ".rtc", SNES_MEMORY_GAME_BOY_RTC);
    } break;
  }
}

void Cartridge::unload() {
  if(application.cartridgeLoaded == false) return;
  utility.modifySystemState(Utility::UnloadCartridge);
}

void Cartridge::loadCheats() {
  string name;
  name << filepath(nall::basename(baseName), config().path.cheat);
  name << ".cht";
  cheatEditorWindow->load(name);
}

void Cartridge::saveCheats() {
  string name;
  name << filepath(nall::basename(baseName), config().path.cheat);
  name << ".cht";
  cheatEditorWindow->save(name);
}

//=================
//private functions
//=================

bool Cartridge::loadCartridge(string &filename, string &xml, uint8_t *&cartData, unsigned &cartSize) {
  if(file::exists(filename) == false) return false;

  uint8_t *data;
  unsigned size;
  audio.clear();
  if(reader.load(filename, data, size) == false) return false;

  patchApplied = false;
  string name(filepath(nall::basename(filename), config().path.patch), ".ups");

  file fp;
  if(config().file.applyPatches && fp.open(name, file::mode::read)) {
    unsigned patchsize = fp.size();
    uint8_t *patchdata = new uint8_t[patchsize];
    fp.read(patchdata, patchsize);
    fp.close();

    uint8_t *outdata = 0;
    unsigned outsize = 0;
    ups patcher;
    if(patcher.apply(patchdata, patchsize, data, size, 0, outsize) == ups::result::target_too_small) {
      outdata = new uint8_t[outsize];
      if(patcher.apply(patchdata, patchsize, data, size, outdata, outsize) == ups::result::success) {
        delete[] data;
        data = outdata;
        size = outsize;
        patchApplied = true;
      } else {
        delete[] outdata;
      }
    }
    delete[] patchdata;
  }

  name = string(nall::basename(filename), ".xml");
  if(patchApplied == false && file::exists(name)) {
    //prefer manually created XML cartridge mapping
    xml.readfile(name);
  } else {
    //generate XML mapping from data via heuristics
    xml = SNESCartridge(data, size).xmlMemoryMap;
  }

  cartData = data;
  cartSize = size;
  return true;
}

bool Cartridge::loadMemory(const char *filename, const char *extension, unsigned memory) {
  unsigned memory_size = snes_get_memory_size(memory);
  if(memory_size == 0 || memory_size == -1U) return false;

  string name;
  name << filepath(nall::basename(filename), config().path.save);
  name << extension;

  file fp;
  if(fp.open(name, file::mode::read) == false) return false;

  unsigned size = fp.size();
  uint8_t *data = new uint8_t[size];
  fp.read(data, size);
  fp.close();

  memcpy(snes_get_memory_data(memory), data, min(size, memory_size));
  delete[] data;
  return true;
}

bool Cartridge::saveMemory(const char *filename, const char *extension, unsigned memory) {
  unsigned memory_size = snes_get_memory_size(memory);
  if(memory_size == 0 || memory_size == -1U) return false;

  string name;
  name << filepath(nall::basename(filename), config().path.save);
  name << extension;

  file fp;
  if(fp.open(name, file::mode::write) == false) return false;

  fp.write(snes_get_memory_data(memory), memory_size);
  fp.close();
  return true;
}

string Cartridge::decodeShiftJIS(const char *text) {
  unsigned length = strlen(text), offset = 0;
  string output;

  for(unsigned i = 0; i < length;) {
    unsigned code = 0;
    uint8_t n = text[i++];

    if(n == 0x00) {
      //string terminator
      break;
    } else if(n >= 0x20 && n <= 0x7f) {
      //ASCII
      code = n;
    } else if(n >= 0xa0 && n <= 0xdf) {
      //ShiftJIS half-width katakana
      unsigned dakuten = 0, handakuten = 0;

      switch(n) {
        case 0xa1: code = 0xe38082; break;  //(period)
        case 0xa2: code = 0xe3808c; break;  //(open quote)
        case 0xa3: code = 0xe3808d; break;  //(close quote)
        case 0xa4: code = 0xe38081; break;  //(comma)
        case 0xa5: code = 0xe383bb; break;  //(separator)
        case 0xa6: code = 0xe383b2; break;  //wo
        case 0xa7: code = 0xe382a1; break;  //la
        case 0xa8: code = 0xe382a3; break;  //li
        case 0xa9: code = 0xe382a5; break;  //lu
        case 0xaa: code = 0xe382a7; break;  //le
        case 0xab: code = 0xe382a9; break;  //lo
        case 0xac: code = 0xe383a3; break;  //lya
        case 0xad: code = 0xe383a5; break;  //lyu
        case 0xae: code = 0xe383a7; break;  //lyo
        case 0xaf: code = 0xe38383; break;  //ltsu
        case 0xb0: code = 0xe383bc; break;  //-
        case 0xb1: code = 0xe382a2; break;  //a
        case 0xb2: code = 0xe382a4; break;  //i
        case 0xb3: code = 0xe382a6; break;  //u 
        case 0xb4: code = 0xe382a8; break;  //e
        case 0xb5: code = 0xe382aa; break;  //o
        case 0xb6: code = 0xe382ab; dakuten = 0xe382ac; break;  //ka,  ga
        case 0xb7: code = 0xe382ad; dakuten = 0xe382ae; break;  //ki,  gi
        case 0xb8: code = 0xe382af; dakuten = 0xe382b0; break;  //ku,  gu
        case 0xb9: code = 0xe382b1; dakuten = 0xe382b2; break;  //ke,  ge
        case 0xba: code = 0xe382b3; dakuten = 0xe382b4; break;  //ko,  go
        case 0xbb: code = 0xe382b5; dakuten = 0xe382b6; break;  //sa,  za
        case 0xbc: code = 0xe382b7; dakuten = 0xe382b8; break;  //shi, zi
        case 0xbd: code = 0xe382b9; dakuten = 0xe382ba; break;  //su,  zu
        case 0xbe: code = 0xe382bb; dakuten = 0xe382bc; break;  //se,  ze
        case 0xbf: code = 0xe382bd; dakuten = 0xe382be; break;  //so,  zo
        case 0xc0: code = 0xe382bf; dakuten = 0xe38380; break;  //ta,  da
        case 0xc1: code = 0xe38381; dakuten = 0xe38382; break;  //chi, di
        case 0xc2: code = 0xe38384; dakuten = 0xe38385; break;  //tsu, du
        case 0xc3: code = 0xe38386; dakuten = 0xe38387; break;  //te,  de
        case 0xc4: code = 0xe38388; dakuten = 0xe38389; break;  //to,  do
        case 0xc5: code = 0xe3838a; break;  //na
        case 0xc6: code = 0xe3838b; break;  //ni
        case 0xc7: code = 0xe3838c; break;  //nu
        case 0xc8: code = 0xe3838d; break;  //ne
        case 0xc9: code = 0xe3838e; break;  //no
        case 0xca: code = 0xe3838f; dakuten = 0xe38390; handakuten = 0xe38391; break;  //ha, ba, pa
        case 0xcb: code = 0xe38392; dakuten = 0xe38393; handakuten = 0xe38394; break;  //hi, bi, pi
        case 0xcc: code = 0xe38395; dakuten = 0xe38396; handakuten = 0xe38397; break;  //fu, bu, pu
        case 0xcd: code = 0xe38398; dakuten = 0xe38399; handakuten = 0xe3839a; break;  //he, be, pe
        case 0xce: code = 0xe3839b; dakuten = 0xe3839c; handakuten = 0xe3839d; break;  //ho, bo, po
        case 0xcf: code = 0xe3839e; break;  //ma
        case 0xd0: code = 0xe3839f; break;  //mi
        case 0xd1: code = 0xe383a0; break;  //mu
        case 0xd2: code = 0xe383a1; break;  //me
        case 0xd3: code = 0xe383a2; break;  //mo
        case 0xd4: code = 0xe383a4; break;  //ya
        case 0xd5: code = 0xe383a6; break;  //yu
        case 0xd6: code = 0xe383a8; break;  //yo
        case 0xd7: code = 0xe383a9; break;  //ra
        case 0xd8: code = 0xe383aa; break;  //ri
        case 0xd9: code = 0xe383ab; break;  //ru
        case 0xda: code = 0xe383ac; break;  //re
        case 0xdb: code = 0xe383ad; break;  //ro
        case 0xdc: code = 0xe383af; break;  //wa
        case 0xdd: code = 0xe383b3; break;  //n
      }

      if(dakuten && ((uint8_t)text[i] == 0xde)) {
        code = dakuten;
        i++;
      } else if(handakuten && ((uint8_t)text[i] == 0xdf)) {
        code = handakuten;
        i++;
      }
    }

    if(code) {
      if((uint8_t)(code >> 16)) output[offset++] = (char)(code >> 16);
      if((uint8_t)(code >>  8)) output[offset++] = (char)(code >>  8);
      if((uint8_t)(code >>  0)) output[offset++] = (char)(code >>  0);
    }
  }

  output[offset] = 0;
  return output;
}
