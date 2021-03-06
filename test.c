//Copyright (c) 2011-2020 <>< Charles Lohr - Under the MIT/x11 or NewBSD License you choose.
// NO WARRANTY! NO GUARANTEE OF SUPPORT! USE AT YOUR OWN RISK
// Super basic test - see rawdrawandroid's thing for a more reasonable test.

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include "os_generic.h"
#include <GLES3/gl3.h>
#include <asset_manager.h>
#include <asset_manager_jni.h>
#include <android_native_app_glue.h>
#include <android/log.h>
#include <android/sensor.h>
#include "CNFGAndroid.h"

#define CNFG3D
#define CNFG_IMPLEMENTATION
#include "CNFG.h"

#include "spng/spng.h"

unsigned frames = 0;
unsigned long iframeno = 0;


#define GENLINEWIDTH 89
#define GENLINES 67

int genlinelen = 0;
char genlog[(GENLINEWIDTH+1)*(GENLINES+1)+2] = "log";
int genloglen;
int genloglines;
int firstnewline = -1;

void example_log_function( int readSize, char * buf ) {
	static og_mutex_t *mt;
	if (!mt) mt = OGCreateMutex();
	OGLockMutex(mt);
	int i;
	for (i = 0; (readSize >= 0) ? (i <= readSize) : buf[i]; i++) {
		char c = buf[i];
		if (c == '\0') c = '\n';
		if ((c != '\n' && genlinelen >= GENLINEWIDTH) || c == '\n') {
			int k;
			genloglines++;
			if (genloglines >= GENLINES) {
				genloglen -= firstnewline + 1;
				int offset = firstnewline;
				firstnewline = -1;

				for (k = 0; k < genloglen; k++) {
					if ((genlog[k] = genlog[k + offset + 1]) == '\n' && firstnewline < 0) {
						firstnewline = k;
					}
				}
				genlog[k] = 0;
				genloglines--;
			}
			genlinelen = 0;
			if (c != '\n') {
				genlog[genloglen + 1] = 0;
				genlog[genloglen++] = '\n';
			}
			if (firstnewline < 0) firstnewline = genloglen;
		}
		genlog[genloglen + 1] = 0;
		genlog[genloglen++] = c;
		if (c != '\n') genlinelen++;
	}

	OGUnlockMutex(mt);
}

volatile int suspended;

short screenx, screeny;
int lastbuttonx = 0;
int lastbuttony = 0;
int lastmotionx = 0;
int lastmotiony = 0;
int lastbid = 0;
int lastmask = 0;
int lastkey, lastkeydown;

static bool keyboard_up;

int textBoxStartX = -1;
int textBoxEndX = -1;
int textBoxStartY = -1;
int textBoxEndY = -1;
char text[15] = "text\0";

void HandleKey( int keycode, int bDown ) {
	lastkey = keycode;
	lastkeydown = bDown;

	if(bDown) {
		int len = strlen(text);
		if(keycode == 67) {
			text[len-1] = '\0';
		} else if(('0' <= keycode && keycode <= '9')
					|| ('a' <= keycode && keycode <= 'z')
					|| ('A' <= keycode && keycode <= 'Z')) {
			if(len < 15) {
				text[len] = keycode;
				text[len+1] = '\0';
			}
		}
	}

	if(keycode == 10 && !bDown) {
		keyboard_up = false;
		AndroidDisplayKeyboard(keyboard_up);
	} else if (keycode == 4) {
		AndroidSendToBack(1); //Handle Physical Back Button.
	}
}

void HandleButton(int x, int y, int button, int bDown) {
	lastbid = button;
	lastbuttonx = x;
	lastbuttony = y;

	if(bDown && (textBoxStartX <= x && x <= textBoxEndX) && (textBoxStartY <= y && y <= textBoxEndY)) {
		keyboard_up = true;
		AndroidDisplayKeyboard(true);
	}
}

void HandleMotion( int x, int y, int mask ) {
	lastmask = mask;
	lastmotionx = x;
	lastmotiony = y;
}

extern struct android_app * gapp;


void HandleDestroy() {
}

void HandleSuspend() {
	suspended = 1;
}

void HandleResume() {
	suspended = 0;
}

struct Image {
	uint32_t * pixels;//RGBA 8bits
	uint32_t width, height;
};

struct Image load(const char * name) {
	struct Image image;

	AAsset *file = AAssetManager_open(gapp->activity->assetManager, name, AASSET_MODE_BUFFER);

	size_t fileLength = AAsset_getLength(file);
	unsigned char * buffer = malloc(fileLength);
	memcpy(buffer, AAsset_getBuffer(file), fileLength);

    spng_ctx * ctx = spng_ctx_new(0);
    spng_set_png_buffer(ctx, buffer, fileLength);

    size_t outSize;
    spng_decoded_image_size(ctx, SPNG_FMT_RGBA8, &outSize);

    struct spng_ihdr ihdr;
    spng_get_ihdr(ctx, &ihdr);
    image.width = ihdr.width;
    image.height = ihdr.height;

    unsigned char * pixels = malloc(outSize);
    spng_decode_image(ctx, pixels, outSize, SPNG_FMT_RGBA8, 0);

    image.pixels = pixels;

    spng_ctx_free(ctx);
	AAsset_close(file);

	return image;
}

int main() {
	int i, x, y;
	double ThisTime;
	double LastFPSTime = OGGetAbsoluteTime();
	double LastFrameTime = OGGetAbsoluteTime();
	double SecToWait;
	int linesegs = 0;

	CNFGBGColor = 0x400000;
	CNFGSetup("Test Bench", 0, 0);

	struct Image imageIcon = load("icon.png");
	uint64_t size = imageIcon.width*imageIcon.height;
	for(int pos = 0; pos < size; pos++) {
		if((imageIcon.pixels[pos] & 0x000000FF) == 0) {
			imageIcon.pixels[pos] = 0xFFFFFFFF;
		}
	}

	struct Image imageBackground = load("background.png");

	while (1) {
		int i, pos;
		float f;
		iframeno++;
		RDPoint pto[3];

		CNFGHandleInput();

		if (suspended) {
			usleep(50000);
			continue;
		}

		CNFGClearFrame();
		CNFGColor(0xFFFFFF);
		glLineWidth(1.0);
		CNFGGetDimensions(&screenx, &screeny);

		UpdateScreenWithBitmapOffsetX = 0;
		UpdateScreenWithBitmapOffsetY = 0;
		CNFGUpdateScreenWithBitmap(imageBackground.pixels, imageBackground.width, imageBackground.height);

		int keyboardHeight = keyboard_up? 500 : 0;

		textBoxStartX = 20;
		textBoxEndX = screenx-20;
		textBoxStartY = screeny-120-100 - keyboardHeight;
		textBoxEndY = screeny-20-100 - keyboardHeight;

		// Square behind text
		CNFGDialogColor = 0xFFFFFF;
		CNFGDrawBox(textBoxStartX, textBoxStartY, textBoxEndX, textBoxEndY);

		//Green circle
		UpdateScreenWithBitmapOffsetX = textBoxStartX + 15;
		UpdateScreenWithBitmapOffsetY = textBoxStartY + 10;
		CNFGUpdateScreenWithBitmap(imageIcon.pixels, imageIcon.width, imageIcon.height);
		CNFGFlushRender();

		const int TEXT_SCALE = 10;
		const int TEXT_WIDTH = 3 * TEXT_SCALE;

		if(((int)OGGetAbsoluteTime()) % 2 == 0) {
			// Text input line
			CNFGDialogColor = 0x000000;
			int len = strlen(text);
			CNFGDrawBox(textBoxStartX + 10 + 100 + 10 + len * TEXT_WIDTH, textBoxStartY + 20,
				textBoxStartX + 10 + 100 + 10 + 2 + len * TEXT_WIDTH, textBoxEndY - 20);
			CNFGFlushRender();
		}

		//Inputted text
		glLineWidth(5.0);
		CNFGPenX = textBoxStartX  + 10 + 100 + 10;
		CNFGPenY = textBoxStartY + 20;
		CNFGColor(0x000000);
		CNFGDrawText(text, TEXT_SCALE);
		CNFGFlushRender();

		frames++;
		CNFGSwapBuffers();

		ThisTime = OGGetAbsoluteTime();
		if (ThisTime > LastFPSTime + 1) {
			printf("FPS: %d\n", frames);
			frames = 0;
			linesegs = 0;
			LastFPSTime += 1;
		}
	}
}
