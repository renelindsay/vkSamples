#ifndef NATIVE_H
#define NATIVE_H

#include <jni.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <android/log.h>
#include <android_native_app_glue.h>
#include "android_fopen.h"                       // redirect fopen, to read files from asset folder
#include <string>

extern android_app* Android_App;                 // Native Activity state info

int  printf(const char* format, ...);            // printf for Android (allows multiple printf's per line)
void ShowKeyboard(bool visible, int flags = 0);  // Show/hide Android keyboard
int  GetUnicodeChar(int eventType, int keyCode, int metaState);
std::string UnicodeToUTF8(int unicode);

#endif