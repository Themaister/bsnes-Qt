void Utility::modifySystemState(system_state_t systemState) {
  fileBrowser->close();  //avoid edge case oddities (eg movie playback window still open from previous game)
  state.resetHistory();  //do not allow rewinding past a destructive system action
  movie.stop();  //movies cannot continue to record after destructive system actions

  video.clear();
  audio.clear();

  switch(systemState) {
    case LoadCartridge: {
      if(application.cartridgeLoaded == true) break;
      application.cartridgeLoaded = true;
      cartridge.loadCheats();

      application.power = true;
      application.pause = false;
      snes_power();

      showMessage(string()
      << "Loaded " << cartridge.name
      << (cartridge.patchApplied ? ", and applied UPS patch." : "."));
      mainWindow->setWindowTitle(string() << cartridge.name << " - " << ProgramName);
      #if defined(DEBUGGER)
      debugger->echo(string() << "Loaded " << cartridge.name << ".<br>");
      #endif
    } break;

    case UnloadCartridge: {
      if(application.cartridgeLoaded == false) break;  //no cart to unload?
      application.cartridgeLoaded = false;
      cartridge.saveCheats();

      cartridge.saveMemory();   //save memory to disk
      snes_unload_cartridge();  //deallocate memory

      application.power = false;
      application.pause = true;

      showMessage(string() << "Unloaded " << cartridge.name << ".");
      mainWindow->setWindowTitle(string() << ProgramName);
    } break;

    case PowerOn: {
      if(application.cartridgeLoaded == false || application.power == true) break;

      application.power = true;
      application.pause = false;
      snes_power();

      showMessage("Power on.");
    } break;

    case PowerOff: {
      if(application.cartridgeLoaded == false || application.power == false) break;

      application.power = false;
      application.pause = true;

      showMessage("Power off.");
    } break;

    case PowerCycle: {
      if(application.cartridgeLoaded == false) break;

      application.power = true;
      application.pause = false;
      snes_power();

      showMessage("System power was cycled.");
    } break;

    case Reset: {
      if(application.cartridgeLoaded == false || application.power == false) break;

      application.pause = false;
      snes_reset();

      showMessage("System was reset.");
    } break;
  }

  mainWindow->syncUi();
  #if defined(DEBUGGER)
  debugger->modifySystemState(systemState);
  debugger->synchronize();
  #endif
  cheatEditorWindow->synchronize();
  cheatFinderWindow->synchronize();
  stateManagerWindow->reload();
}
