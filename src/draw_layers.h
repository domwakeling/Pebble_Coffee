#pragma once
#include <pebble.h>
	
int next_up(int current);
int next_down(int current);

char* header_text(int i);
char* detail_text(int i);

void draw_graphics_image(int recordNum, GContext *ctx);