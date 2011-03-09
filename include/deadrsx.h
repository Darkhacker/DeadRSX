//    DeadRSX Hardware Renderd Graphics Library
//    Author DarkhackerPS3  
//   -Thanks to Matt_P from EFNET #psl1ght for explaining stuff to me :P

//    DeadRSX is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.

//    DeadRSX is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.

//    You should have received a copy of the GNU General Public License
//    along with DeadRSX.  If not, see <http://www.gnu.org/licenses/>.

#pragma once
#include <png.h>
#include <pngdec/loadpng.h>
#include <sysutil/video.h>
#include <rsx/gcm.h>

#define COLOR_NONE -1
#define COLOR_BLACK 0x00000000
#define COLOR_WHITE 0xffffffff
#define COLOR_RED 0x00ff0000
#define COLOR_GREEN 0x0000ff00
#define COLOR_BLUE 0x000000ff
#define Z_SCALE 1.0/65536.0

extern u32 *buffer[2];
extern gcmContextData *context; 
extern VideoResolution res;

void deadrsx_init(); // initilize the screen
void deadrsx_scale(); // scales the screen to 847x511 to support all tv screens
void waitFlip();
void flip(s32 buffer);
void setupRenderTarget(u32 currentBuffer);

void deadrsx_background(u32 trueColor);

void deadrsx_offset(u32 dooffset, int x, int y, int w, int h);
void deadrsx_scaleoffset(u32 dooffset, int x, int y, int w, int h, int ow, int oh);
void deadrsx_sprite(u32 dooffset, float x, float y, float w, float h, int ow, int oh, int tilex, int tiley, int ax, int ay);

void deadrsx_loadfile(char inputpath[], PngDatas inputImage, u32 *inputOffset);
