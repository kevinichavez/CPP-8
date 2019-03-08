#pragma once

// Constants relating to the CHIP-8 machine itself
const int CH8_WIDTH = 64;
const int CH8_HEIGHT = 32;
const int CH8_STACK_SIZE = 24;
const int CH8_MESIZE = 0x1000;

// Constants relating to clock rate and frames per second
const float TIMER_RATE = 1000.0 / 60.0;