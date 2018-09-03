#pragma once

#include <android/log.h>
#include <stdio.h>
#include <stdlib.h>


#ifndef TAG
#define TAG "NetCodec"
#endif

#define  LOGTAG true

#if LOGTAG
	#define GLOGV(...)  __android_log_print(ANDROID_LOG_VERBOSE,TAG,__VA_ARGS__)	//black
	#define GLOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,	TAG,  __VA_ARGS__)	//blue
	#define GLOGI(...)  __android_log_print(ANDROID_LOG_INFO,	TAG,  __VA_ARGS__)	//green
	#define GLOGW(...)  __android_log_print(ANDROID_LOG_WARN,	TAG,  __VA_ARGS__)  //yellow
	#define GLOGE(...)  __android_log_print(ANDROID_LOG_ERROR,	TAG,  __VA_ARGS__)  //red
#endif

#ifndef MAX_PACKET_SIZE
#define MAX_PACKET_SIZE	((60* 1024) - 1)
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(p) 	do{delete(p); p = NULL;}while(0)
#endif

#define SAFE_DELETE_ELEMENT(ptr) if(ptr != NULL){ delete ptr; ptr = NULL;}

#ifndef SAFE_FREE
#define SAFE_FREE(p) 	do{free(p); p = NULL;}while(0)
#endif


