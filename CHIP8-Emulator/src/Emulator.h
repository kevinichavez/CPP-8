#ifndef EMULATOR_H
#define EMULATOR_H

#include <string>
#include "Chip8.h"
#include <SDL.h>

class Emulator {
public:
	Emulator();

	// Specify flags when constructing
	Emulator(uint16_t flags);

	bool selectGame();
	int runGame();
	int runGame(std::string path);
	void reset();
	void togglePause();
	bool isPaused() { return m_paused; }
	void toggleThrottle() { m_throttleSpeed = !m_throttleSpeed; }

private:
	Chip8 chip;
	std::string m_gamePath;
	SDL_Window* m_gameWindow;
	SDL_Renderer* m_renderer;
	SDL_Texture* m_texture;
	int m_width, m_height, m_scaleWidth, m_scaleHeight;
	unsigned long m_totalFrames;
	bool m_paused;
	bool m_throttleSpeed;
	bool m_useSDLdelay;
	double m_previousFPSarray[MAX_STORED_FPS_VALS];
	int m_numStoredFPS;

	void drawScreen();
	void sendInput(const uint8_t* ks, bool keys[]);
	double getFPS();
};

#endif