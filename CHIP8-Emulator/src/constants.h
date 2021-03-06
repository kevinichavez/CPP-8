#ifndef CONSTANTS_H
#define CONSTANTS_H

// Constants relating to the CHIP-8 machine itself
const int CH8_WIDTH = 64;
const int CH8_HEIGHT = 32;
const int CH8_STACK_SIZE = 24;
const int CH8_MEM_SIZE = 0x1000;
const int DEFAULT_SCALE = 10;
const int CH8_MAX_SPRITE_WIDTH = 8;
const int CH8_FONT_WIDTH = 5;
const uint8_t CH8_FONTSET[80] = {
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

// Constants relating to clock rate and frames per second
const double TARGET_FRAMERATE = 60.0;
const double TARGET_FRAMETIME_MILLISECONDS = 1000.0 / TARGET_FRAMERATE;
const double TARGET_FRAMETIME_SECONDS = 1.0 / TARGET_FRAMERATE;
const int SDL_DELAY_VALUE = 10;
const int MAX_STORED_FPS_VALS = 10;

// Error code constants
const int SUCCESS = 0;
const int ERR_INIT_SDL = 1;
const int ERR_ROM_READ = -1;
const int ERR_ROM_TOO_BIG = -2;

// Sound
const int MEGABYTE = 1048576;
const int SOUND_BUFFER_SIZE = MEGABYTE * 16;
const int SOUND_FREQUENCY = 44100;
const int SOUND_NUM_CHANNELS = 1;
const int SOUND_NUM_SAMPLES = 1024;
const int SOUND_DEFAULT_GAIN = 4000;
const int SOUND_SAMPLE_SIZE = 2;      // sizeof(int16_t)
const int SOUND_INITIAL_BUFFER_TIME = 10;
const int SOUND_DEFAULT_PLAY_FREQUENCY = 400;

#endif