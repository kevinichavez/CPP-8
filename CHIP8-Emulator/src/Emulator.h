#ifndef EMULATOR_H
#define EMULATOR_H

#include <string>
#include "Chip8.h"
#include <SDL.h>
#include "timer.h"

class Emulator {
public:
	Emulator();
	~Emulator();

	bool selectGame();
	int runGame();
	int runGame(std::string path);
	void reset();
private:
	Chip8 chip;
	std::string m_gamePath;
	SDL_Window* m_gameWindow;
	SDL_Renderer* m_renderer;
	SDL_Texture* m_texture;
	int m_width, m_height, m_scaleWidth, m_scaleHeight;
	Timer m_fpsTimer;
	unsigned long m_totalFrames;

	void drawScreen();
	void sendInput(const uint8_t* ks, bool keys[]);
	double getAvgFPS();
};

#endif