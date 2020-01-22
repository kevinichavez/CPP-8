#ifndef EMULATOR_H
#define EMULATOR_H

#include <string>
#include "Chip8.h"
#include <SDL.h>

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

	void drawScreen();
	void sendInput(const uint8_t* ks, bool keys[]);
};

#endif