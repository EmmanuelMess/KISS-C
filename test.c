//Copyright (c) 2011-2020 <>< Charles Lohr - Under the MIT/x11 or NewBSD License you choose.
// NO WARRANTY! NO GUARANTEE OF SUPPORT! USE AT YOUR OWN RISK
// Super basic test - see rawdrawandroid's thing for a more reasonable test.

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
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

static int keyboard_up;

void HandleKey( int keycode, int bDown ) {
	lastkey = keycode;
	lastkeydown = bDown;
	if (keycode == 4) {
		AndroidSendToBack(1);
	}
}

void HandleButton( int x, int y, int button, int bDown ) {
	lastbid = button;
	lastbuttonx = x;
	lastbuttony = y;
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

int main() {
	int i, x, y;
	double ThisTime;
	double LastFPSTime = OGGetAbsoluteTime();
	double LastFrameTime = OGGetAbsoluteTime();
	double SecToWait;
	int linesegs = 0;

	CNFGBGColor = 0x400000;
	CNFGDialogColor = 0x444444;
	CNFGSetup("Test Bench", 0, 0);

	AAsset *file = AAssetManager_open(gapp->activity->assetManager, "icon.explicit4ch8b", AASSET_MODE_BUFFER);

	size_t fileLength = AAsset_getLength(file);
	unsigned char * buffer = malloc(fileLength);
	memcpy(buffer, AAsset_getBuffer(file), fileLength);

	uint32_t imageWidth = *((uint32_t*) buffer);
	buffer += sizeof(u_int32_t);
	uint32_t imageHeight = *((uint32_t*) buffer);
	buffer += sizeof(u_int32_t);

	AAsset_close(file);


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
		CNFGGetDimensions(&screenx, &screeny);

		// Mesh in background
		CNFGColor(0xffffff);
		UpdateScreenWithBitmapOffsetX = 500;
		UpdateScreenWithBitmapOffsetY = 500;
		CNFGUpdateScreenWithBitmap(buffer, imageWidth, imageHeight);
		CNFGFlushRender();

		CNFGPenX = 0;
		CNFGPenY = 480;
		char st[50];
		sprintf(st, "%dx%d %d %d %d %d %d %d\n%d %d", screenx, screeny, lastbuttonx, lastbuttony, lastmotionx,
		        lastmotiony, lastkey, lastkeydown, lastbid, lastmask);
		CNFGDrawText(st, 10);
		glLineWidth(2.0);

		// Square behind text
		CNFGDrawBox(600, 0, 950, 350);

		CNFGPenX = 10;
		CNFGPenY = 10;

		pos = 0;
		CNFGColor(0xffffff);
		CNFGPenX = 5;
		CNFGPenY = 600;
		CNFGDrawText(genlog, 4);

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
