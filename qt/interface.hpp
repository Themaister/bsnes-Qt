void bsnes_video_refresh(const uint16_t *data, unsigned width, unsigned height);
void bsnes_audio_sample(uint16_t left, uint16_t right);
void bsnes_input_poll();
int16_t bsnes_input_state(bool port, unsigned device, unsigned index, unsigned id);

class Interface : public library {
public:
  void video_refresh(const uint16_t *data, unsigned width, unsigned height);
  void audio_sample(uint16_t left, uint16_t right);
  void input_poll();
  int16_t input_poll(bool port, unsigned device, unsigned index, unsigned id);

  Interface();
  void captureScreenshot(uint32_t*, unsigned, unsigned, unsigned);
  bool saveScreenshot;
  bool framesUpdated;
  unsigned framesExecuted;
};

extern Interface interface;
