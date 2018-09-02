#include <jni.h>
#include "common.h"
#include "H264Sender.h"

#ifndef NELEM
#define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))
#endif

#define REG_PATH "com/great/happyness/mediacodecmy/NativeCodec"

JavaVM*		g_javaVM	= NULL;

H264Sender 	*mpH264Sender 	= NULL;

static jboolean playNativeAudio(JNIEnv *env, jobject, jstring strPath, jint cardid, jboolean loopPlay)
{
	jboolean bRet = JNI_FALSE;

	if(NULL == mpLocalPlayer)
	{
		jboolean isCopy = JNI_FALSE;
		const char *filePath = env->GetStringUTFChars(strPath, NULL);

		mpLocalPlayer = new LocalPlayer();
		bRet = mpLocalPlayer->Init(filePath, cardid, loopPlay);
		mpLocalPlayer->Start();

		env->ReleaseStringUTFChars(strPath, filePath);
	}

	return bRet;
}

static JNINativeMethod video_method_table[] = {
		//

};

int registerNativeMethods(JNIEnv* env, const char* className, JNINativeMethod* methods, int numMethods)
{
    jclass clazz;

    clazz = env->FindClass(className);
    if (clazz == NULL) {
        GLOGE("Native registration unable to find class '%s'", className);
        return JNI_FALSE;
    }
    if (env->RegisterNatives(clazz, methods, numMethods) < 0) {
        GLOGE("RegisterNatives failed for '%s'", className);
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

extern "C" jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
	JNIEnv* env = NULL;
	jint result = -1;
	if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
		GLOGE("GetEnv failed!");
		return result;
	}

	g_javaVM = vm;

	GLOGW("JNI_OnLoad......");
	registerNativeMethods(env,
			REG_PATH, video_method_table,
			NELEM(video_method_table));

	return JNI_VERSION_1_4;
}


