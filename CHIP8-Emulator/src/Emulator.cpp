#include "Emulator.h"
#include <nfd.h>
#include <SDL.h>
#include <iostream>
#include <cstdint>
#include "constants.h"

Emulator::Emulator() {
	reset();
}

Emulator::Emulator(uint16_t flags) {
	reset();
}

void Emulator::reset() {
	m_fpsTimer.stop();
	m_scaleWidth = DEFAULT_SCALE;
	m_scaleHeight = DEFAULT_SCALE;
	m_width = CH8_WIDTH * m_scaleWidth;
	m_height = CH8_HEIGHT * m_scaleHeight;
	m_gamePath = "";
	m_totalFrames = 0;
	m_paused = false;
	m_throttleSpeed = true;
	m_useSDLdelay = true;
}

void Emulator::togglePause() {
	if (m_paused)
		m_fpsTimer.unpause();
	else m_fpsTimer.pause();

	m_paused = !m_paused;
}

bool Emulator::selectGame() {
	// Select ROM file from open file dialogue
	nfdchar_t* outPath = nullptr;
	nfdresult_t result = NFD_OpenDialog("ch8", NULL, &outPath);

	switch (result) {
	case NFD_OKAY:
		std::cout << "ROM path found successfully!" << std::endl;
		std::cout << outPath << std::endl;
		m_gamePath = outPath;
		delete outPath;
		return true;
		break;

	case NFD_CANCEL:
		std::cout << "User pressed cancel." << std::endl << "Exiting program..." << std::endl;
		return false;
		break;

	default:
		std::cerr << "Error: " << NFD_GetError() << std::endl;
		return false;
	}
}

void Emulator::drawScreen() {
	// Set render fill color to black
	SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);

	// Clear all current pixels by making the screen black
	SDL_RenderClear(m_renderer);

	// Draw pixels in white
	SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);

	// Make every non-zero value in the graphics array a white rectangle
	for (int i = 0; i < CH8_HEIGHT; i++) {
		for (int j = 0; j < CH8_WIDTH; j++) {
			if (chip.gfx[j][i]) {
				SDL_Rect pixel;
				pixel.x = j * m_scaleWidth;
				pixel.y = i * m_scaleHeight;
				pixel.w = m_scaleWidth;
				pixel.h = m_scaleHeight;

				SDL_RenderFillRect(m_renderer, &pixel);
			}
		}
	}

	// Present the pixel positions to the user
	SDL_RenderPresent(m_renderer);

	// Add one frame to the total
	++m_totalFrames;
}

void Emulator::sendInput(const uint8_t* ks, bool keys[]) {
	keys[0x1] = ks[SDL_SCANCODE_1]; keys[0x2] = ks[SDL_SCANCODE_2]; keys[0x3] = ks[SDL_SCANCODE_3]; keys[0xC] = ks[SDL_SCANCODE_4];
	keys[0x4] = ks[SDL_SCANCODE_Q]; keys[0x5] = ks[SDL_SCANCODE_W]; keys[0x6] = ks[SDL_SCANCODE_E]; keys[0xD] = ks[SDL_SCANCODE_R];
	keys[0x7] = ks[SDL_SCANCODE_A]; keys[0x8] = ks[SDL_SCANCODE_S]; keys[0x9] = ks[SDL_SCANCODE_D]; keys[0xE] = ks[SDL_SCANCODE_F];
	keys[0xA] = ks[SDL_SCANCODE_Z]; keys[0x0] = ks[SDL_SCANCODE_X]; keys[0xB] = ks[SDL_SCANCODE_C]; keys[0xF] = ks[SDL_SCANCODE_V];

	chip.setKeys(keys);
}

int Emulator::runGame() {
	// Load ROM into memory
	int result = chip.loadRom(m_gamePath);
	if (result != SUCCESS) {
		return result;
	}

	m_gameWindow = SDL_CreateWindow(
		"CHIP-8",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		m_width,
		m_height,
		SDL_WINDOW_OPENGL |
		SDL_WINDOW_SHOWN
	);

	SDL_GL_SetSwapInterval(0);

	if (m_gameWindow == NULL) {
		std::cerr << "Could not create window. SDL Error: " << SDL_GetError() << std::endl;
		return ERR_INIT_SDL;
	}

	// Create Renderer
	m_renderer = SDL_CreateRenderer(m_gameWindow, -1, SDL_RENDERER_ACCELERATED);

	// Create Texture
	m_texture = SDL_CreateTexture(m_renderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		m_width,
		m_height);

	// Snapshot of keyboard's current state
	const uint8_t* keystate = SDL_GetKeyboardState(NULL);

	// Store values of keyboard keys mapped to CHIP-8
	bool keys[16];

	SDL_Event e;

	bool quit = false;
	double speed = 1.0;
	uint64_t prevFrame = SDL_GetPerformanceCounter();
	uint64_t currentFrame;
	double secondsBetweenFrames;
	std::string title;
	m_paused = false;
	m_fpsTimer.start();
	if (avgSleep.valid())
		m_useSDLdelay = (avgSleep.get() <= TARGET_FRAMETIME_SECONDS);
	else {
		quit = true;
		std::cerr << "There was an error in the async call to getAvgSleep";
	}

	// main loop
	while (!quit) {

		// Mostly debug; will remove all except SDL_QUIT in future version
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT)
				quit = true;
			if (e.type == SDL_KEYDOWN) {
				if (keystate[SDL_SCANCODE_COMMA]) {
					if (speed > 0)
						speed -= 0.1;
					std::cout << "Emulation speed slowed to " << (int)(speed * 100 + 0.5) << "%\n";
				}
				else if (keystate[SDL_SCANCODE_PERIOD]) {
					if (speed < 500)
						speed += 0.1;
					std::cout << "Emulation speed sped up to " << (int)(speed * 100 + 0.5) << "%\n";
				}
				else if (keystate[SDL_SCANCODE_SLASH]) {
					std::cout << "Current FPS: " << (int)(getAvgFPS() + 0.5) << '\n';
				}
				else if (keystate[SDL_SCANCODE_ESCAPE]) {
					if (m_paused)
						std::cout << "Unpaused\n";
					else std::cout << "Paused\n";
					togglePause();
				}
				else if (keystate[SDL_SCANCODE_M]) {
					std::cout << "Throttle ";
					if (m_throttleSpeed)
						std::cout << "disabled\n";
					else std::cout << "enabled\n";
					toggleThrottle();
				}

			}

		}

		if (!m_paused) {

			// Pass currently pressed keys to CHIP-8
			sendInput(keystate, keys);

			chip.emulateCycle();

			// Redraw the screen if CHIP-8 drawflag was set
			if (chip.shouldDraw()) {

				drawScreen();

				if (m_throttleSpeed) {
					if(m_useSDLdelay)
						SDL_Delay(SDL_DELAY_VALUE);

					do {
						currentFrame = SDL_GetPerformanceCounter();
						secondsBetweenFrames = (double) (currentFrame - prevFrame) / (double) SDL_GetPerformanceFrequency();
					} while (secondsBetweenFrames < TARGET_FRAMETIME_SECONDS * (1.0 / speed));

					prevFrame = currentFrame;
				}

			}
		}
		else SDL_Delay(10);
		
	}

	return SUCCESS;
}

int Emulator::runGame(std::string path) {
	m_gamePath = path;
	return runGame();
}

double Emulator::getAvgFPS() {
	// we divide by 1000 to get time passed in seconds
	double avg = m_totalFrames / (m_fpsTimer.getTicks() / 1000.0);

	// When total number of ticks is low, average will be too high due to the math, so we correct it here
	if (avg > 2000000)
		return 0;

	return avg;
}