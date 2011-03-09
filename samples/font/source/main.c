/*

   DeadRSX SAMPLE || Font
   Author DarkhackerPS3
*/

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <math.h>
#include <rsx/commands.h>
#include <rsx/nv40.h>
#include <rsx/reality.h>
#include <io/pad.h>
#include <sysmodule/sysmodule.h>
#include <sysutil/events.h>

#include <deadrsx/deadrsx.h>
#include <deadrsx/texture.h>
#include <deadrsx/nv_shaders.h>

#include "font.h"
#include "sprite.bin.h"

int i,sprite_x=0,sprite_y=0;
int currentBuffer = 0;

int app_state = 1; // used to switch screens
int background_color = COLOR_WHITE;

PadInfo padinfo;
PadData paddata;

u32 *tx_mem,sprite_offset;
Image sprite_image;

void drawFrame(int buffer, long frame) {

	realityViewportTranslate(context, 0.0, 0.0, 0.0, 0.0);
	realityViewportScale(context, 1.0, 1.0, 1.0, 0.0); 

        deadrsx_scale(); // scales screen to 847x511 - deadrsx library

	realityZControl(context, 0, 1, 1); // disable viewport culling
	realityBlendFunc(context,
		NV30_3D_BLEND_FUNC_SRC_RGB_SRC_ALPHA |
		NV30_3D_BLEND_FUNC_SRC_ALPHA_SRC_ALPHA,
		NV30_3D_BLEND_FUNC_DST_RGB_ONE_MINUS_SRC_ALPHA |
		NV30_3D_BLEND_FUNC_DST_ALPHA_ZERO);
	realityBlendEquation(context, NV40_3D_BLEND_EQUATION_RGB_FUNC_ADD |
		NV40_3D_BLEND_EQUATION_ALPHA_FUNC_ADD);
	realityBlendEnable(context, 1);
	realityViewport(context, res.width, res.height);
	setupRenderTarget(buffer);

	deadrsx_background(background_color);

	realitySetClearDepthValue(context, 0xffff);
	realityClearBuffers(context, REALITY_CLEAR_BUFFERS_COLOR_R |
				     REALITY_CLEAR_BUFFERS_COLOR_G |
				     REALITY_CLEAR_BUFFERS_COLOR_B |
				     NV30_3D_CLEAR_BUFFERS_COLOR_A |
				     NV30_3D_CLEAR_BUFFERS_STENCIL |
				     REALITY_CLEAR_BUFFERS_DEPTH);
	realityLoadVertexProgram_old(context, &nv40_vp);
	realityLoadFragmentProgram_old(context, &nv30_fp);

        switch(app_state) {

        case 1:
        background_color = COLOR_WHITE;
        drawText(sprite_offset, 32, "DeadRSX 0.1 Font Sample", 0, 0);
        drawText(sprite_offset, 20, "Check www.ps3sdk.com for updates", 5,35);
        break;

        case 2: // screen while in-game xmb open
        background_color = COLOR_BLACK;

        break;

        }
}

static void eventHandle(u64 status, u64 param, void * userdata) {
    (void)param;
    (void)userdata;
	if(status == EVENT_REQUEST_EXITAPP){
	exit(0);
	}else if(status == EVENT_MENU_OPEN){
	app_state = 2;
	}else if(status == EVENT_MENU_CLOSE) {
        app_state = 1;
	}else{
	printf("Unhandled event: %08llX\n", (unsigned long long int)status);
	}
}
void appCleanup(){
	sysUnregisterCallback(EVENT_SLOT0);
}
void loading() {
// where all are loading going be done :D
	sprite_image = loadPng(sprite_bin);
	assert(realityAddressToOffset(sprite_image.data, &sprite_offset) == 0); 
}
void ps3_pad() {

  ioPadGetInfo(&padinfo);

  switch(app_state) {
  case 1: // are only screen open at the moment
	for(i=0; i<MAX_PADS; i++){
	  if(padinfo.status[i]){
	  ioPadGetData(i, &paddata);
	    if(paddata.BTN_START){
	    exit(0);
	    }else if(paddata.BTN_UP) {

            sprite_y -= 5;

            }else if(paddata.BTN_DOWN) {

            sprite_y += 5;

            }else if(paddata.BTN_LEFT) {

            sprite_x -= 5;

            }else if(paddata.BTN_RIGHT) {

            sprite_x += 5;

            }else{

            }

	  }		
	}
  break;
  }

}
s32 main(s32 argc, const char* argv[])
{

	atexit(appCleanup);
	deadrsx_init();
	ioPadInit(7);
	sysRegisterCallback(EVENT_SLOT0, eventHandle, NULL);

	u32 *frag_mem = rsxMemAlign(256, 256);
	printf("frag_mem = 0x%08lx\n", (u64) frag_mem);
	realityInstallFragmentProgram_old(context, &nv30_fp, frag_mem);

        loading(); // where all the loading done xD

	long frame = 0; 

	while(1){
                ps3_pad(); // where all are controls are
		waitFlip(); // Wait for the last flip to finish, so we can draw to the old buffer
		drawFrame(currentBuffer, frame++); // Draw into the unused buffer
		flip(currentBuffer); // Flip buffer onto screen
		currentBuffer = !currentBuffer;
		sysCheckCallback();

	}
	
	return 0;
}

