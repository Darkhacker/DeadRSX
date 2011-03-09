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

#include <assert.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>

#include <sysutil/video.h>
#include <rsx/gcm.h>
#include <rsx/reality.h>
#include <rsx/commands.h>
#include <rsx/nv40.h>

#include <sysmodule/sysmodule.h>
#include <png.h>
#include <pngdec/loadpng.h>

#include "deadrsx.h"
#include "texture.h"
// #include "nv_shaders.h" does not need be included in deadrsx.c

u32 *buffer[2];
u32 offset[2];
u32 *depth_buffer;
u32 depth_offset;
int pitch;
int depth_pitch;

gcmContextData *context; 
VideoResolution res;

void deadrsx_init() {
	void *host_addr = memalign(1024*1024, 1024*1024);
	assert(host_addr != NULL);
	context = realityInit(0x10000, 1024*1024, host_addr); 
	assert(context != NULL);
	VideoState state;
	assert(videoGetState(0, 0, &state) == 0);
	assert(state.state == 0);
	assert(videoGetResolution(state.displayMode.resolution, &res) == 0);
	pitch = 4 * res.width;
	depth_pitch = 2 * res.width;
	VideoConfiguration vconfig;
	memset(&vconfig, 0, sizeof(VideoConfiguration));
	vconfig.resolution = state.displayMode.resolution;
	vconfig.format = VIDEO_BUFFER_FORMAT_XRGB;
	vconfig.pitch = pitch;
	assert(videoConfigure(0, &vconfig, NULL, 0) == 0);
	assert(videoGetState(0, 0, &state) == 0); 
	s32 buffer_size = pitch * res.height; 
	s32 depth_buffer_size = depth_pitch * res.height;
	printf("buffers will be 0x%x bytes\n", buffer_size);
	gcmSetFlipMode(GCM_FLIP_VSYNC); 
	buffer[0] = rsxMemAlign(16, buffer_size);
	buffer[1] = rsxMemAlign(16, buffer_size);
	assert(buffer[0] != NULL && buffer[1] != NULL);
	depth_buffer = rsxMemAlign(16, depth_buffer_size * 2);
	assert(realityAddressToOffset(buffer[0], &offset[0]) == 0);
	assert(realityAddressToOffset(buffer[1], &offset[1]) == 0);
	assert(gcmSetDisplayBuffer(0, offset[0], pitch, res.width, res.height) == 0);
	assert(gcmSetDisplayBuffer(1, offset[1], pitch, res.width, res.height) == 0);
	assert(realityAddressToOffset(depth_buffer, &depth_offset) == 0);
	gcmResetFlipStatus();
	flip(1);
        SysLoadModule(SYSMODULE_PNGDEC);
}

void deadrsx_scale() { // code used from hermes Tiny3d to rescale screen
        if(res.width < 1280) {
        realityViewportTranslate(context, 38.0 , 16.0, 0.0, 0.0);
        realityViewportScale(context, (float) (res.width - 72) / 848.0,(res.height == 480) ? (512.0) / 576.0 : 548.0 / 512.0, Z_SCALE, 1.0);
        } else if(res.width == 1280) {
        realityViewportTranslate(context, 54.0, 24.0, 0.0, 0.0);
        realityViewportScale(context, 848.0 / 611.0 , 674.0 / 512.0, Z_SCALE, 1.0);
        }else{
        realityViewportTranslate(context, 63.0, 40.0, 0.0, 0.0);
        realityViewportScale(context, 848.0 / 400.0 , 952.0 / 512.0, Z_SCALE, 1.0);
        }
}

void waitFlip() {
	while(gcmGetFlipStatus() != 0) 
		usleep(200);
	gcmResetFlipStatus();
}

void flip(s32 buffer) {
    assert(gcmSetFlip(context, buffer) == 0);
    realityFlushBuffer(context);
    gcmSetWaitFlip(context);
}

void setupRenderTarget(u32 currentBuffer) {
	realitySetRenderSurface(context, REALITY_SURFACE_COLOR0, REALITY_RSX_MEMORY, 
					offset[currentBuffer], pitch);
	realitySetRenderSurface(context, REALITY_SURFACE_ZETA, REALITY_RSX_MEMORY, 
					depth_offset, depth_pitch);
	realitySelectRenderTarget(context, REALITY_TARGET_0, 
		REALITY_TARGET_FORMAT_COLOR_X8R8G8B8 | 
		REALITY_TARGET_FORMAT_ZETA_Z16 | 
		REALITY_TARGET_FORMAT_TYPE_LINEAR,
		res.width, res.height, 0, 0);
}

void deadrsx_background(u32 trueColor) {
	realitySetClearColor(context, trueColor);
}

void deadrsx_offset(u32 dooffset, int x, int y, int w, int h) {
	load_tex(0, dooffset, w, h, w*4,  NV40_3D_TEX_FORMAT_FORMAT_A8R8G8B8, 1);
	realityVertexBegin(context, REALITY_QUADS);
	{
		realityTexCoord2f(context, 0.0, 0.0);
		realityVertex4f(context, x, y, 0.0, 1.0);
		realityTexCoord2f(context, 1.0, 0.0);
		realityVertex4f(context, x + w, y, 0.0, 1.0);
		realityTexCoord2f(context, 1.0, 1.0);
		realityVertex4f(context, x + w, y + h, 0.0, 1.0); 
		realityTexCoord2f(context, 0.0, 1.0);
		realityVertex4f(context, x, y + h, 0.0, 1.0); 
	}
	realityVertexEnd(context);
}

void deadrsx_scaleoffset(u32 dooffset, int x, int y, int w, int h, int ow, int oh) {
	load_tex(0, dooffset, ow, oh, ow*4,  NV40_3D_TEX_FORMAT_FORMAT_A8R8G8B8, 1);
	realityVertexBegin(context, REALITY_QUADS);
	{
		realityTexCoord2f(context, 0.0, 0.0);
		realityVertex4f(context, x, y, 0.0, 1.0);
		realityTexCoord2f(context, 1.0, 0.0);
		realityVertex4f(context, x + w, y, 0.0, 1.0); 
		realityTexCoord2f(context, 1.0, 1.0);
		realityVertex4f(context, x + w, y + h, 0.0, 1.0); 
		realityTexCoord2f(context, 0.0, 1.0);
		realityVertex4f(context, x, y + h, 0.0, 1.0); 
	}
	realityVertexEnd(context);
}

void deadrsx_sprite(u32 dooffset, float x, float y, float w, float h, int ow, int oh, int tilex, int tiley, int ax, int ay) {

        float wx = 1.0/ax;
        float wy = 1.0/ay;
	load_tex(0, dooffset, ow, oh, ow*4,  NV40_3D_TEX_FORMAT_FORMAT_A8R8G8B8, 1);
	realityVertexBegin(context, REALITY_QUADS);
	{
		realityTexCoord2f(context, tilex * wx, tiley * wy);
		realityVertex4f(context, x, y, 0.0, 1.0);
		realityTexCoord2f(context, (tilex + 1) * wx, tiley * wy);
		realityVertex4f(context, x + w, y, 1.0, 1.0);
		realityTexCoord2f(context, (tilex + 1) * wx, (tiley + 1) * wy);
		realityVertex4f(context, x + w, y + h, 1.0, 1.0); 
		realityTexCoord2f(context, tilex * wx, (tiley + 1) * wy);
		realityVertex4f(context, x, y + h, 0.0, 1.0);
	}
	realityVertexEnd(context);

}

void deadrsx_loadfile(char inputpath[], PngDatas inputImage, u32 *inputOffset) {
LoadPNG(&inputImage, inputpath);
  if(inputImage.bmp_out) {
  uint32_t * text= rsxMemAlign(16, 4*inputImage.width*inputImage.height);
  if(text) memcpy(text, inputImage.bmp_out,inputImage.wpitch*inputImage.height);
  free(inputImage.bmp_out);
  inputImage.bmp_out = text;
  }
assert(realityAddressToOffset(inputImage.bmp_out, inputOffset) == 0);
}
