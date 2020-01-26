#ifndef EMULATOR_H
#define EMULATOR_H

#include <string>
#include "Chip8.h"
#include <SDL.h>
#include "constants.h"
#include <vector>

const int DISABLE_WRAP = 0x01;
const int DISABLE_THROTTLE = 0x02;
const int DISABLE_SDL_DELAY = 0x04;
const int ENABLE_WRAP = 0x0;
const int ENABLE_THROTTLE = 0x0;
const int ENABLE_SDL_DELAY = 0x0;


class Emulator {
public:
	Emulator();

	// Specify flags when constructing
	// Use OR to combine flags
	// AVAILABLE FLAGS:
	// DISABLE_WRAP, DISABLE_THROTTLE, DISABLE_SDL_DELAY
	// ENABLE_WRAP, ENABLE_THROTTLE, ENABLE_SDL_DELAY
	Emulator(uint16_t flags);

	// Destructor
	~Emulator();

	// Have user select a file
	bool selectGame();

	// Run game at m_gamePath
	int runGame();

	// Specify game
	int runGame(std::string path);

	// Reset all values
	void reset();

	// Pause/unpause the game
	void togglePause();

	// Check if the emulator is paused
	bool isPaused() { return m_paused; }

	// Toggle gamespeed throttle
	void toggleThrottle() { m_throttleSpeed = !m_throttleSpeed; }

private:
	Chip8 chip;
	std::string m_gamePath;

	// SDL visual stuff
	SDL_Window* m_gameWindow;
	SDL_Renderer* m_renderer;
	SDL_Texture* m_texture;

	// Window properties
	int m_width, m_height, m_scaleWidth, m_scaleHeight;

	// How many frames have elapsed
	unsigned long m_totalFrames;
	bool m_paused;
	bool m_throttleSpeed;
	bool m_useSDLdelay;
	double m_previousFPS[MAX_STORED_FPS_VALS];
	int m_numStoredFPS;

	// Refresh the screen with what is currently in the Chip 8's gfx array
	void drawScreen();

	// Send keyboard input to Chip 8
	void sendInput(const uint8_t* ks, bool keys[]);

	// Get average FPS of last 10 frames
	double getFPS();

	// Initialize
	void init();

	// Audio related stuff
	SDL_AudioDeviceID m_audioDev;
	SDL_AudioSpec m_spec;

	// Modifies the sound volume
	int m_gain;

	// Store whether noise is being played or not
	bool m_isPlayingSound;

	// How we'll represent our sound wave
	struct Wave {
		// Determines the pitch of the sound
		int frequency;

		// We use this to loop seamlessly as we fill the buffer
		int position;

		// Where we store the points that make up the wave
		std::vector<int16_t> sampleVals;
	} m_square;

	// Buffer s seconds of sound
	void fillAudioQueue(int s);

	// Load a single cyle of a wave at a given frequency
	void setupWave();

	// Push a single sample to SDL
	void pushSample();
};

#endif