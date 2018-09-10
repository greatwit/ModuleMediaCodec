#include <jni.h>
#include "common.h"
#include "H264Sender.h"
#include "TcpSender.h"

#ifndef NELEM
#define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))
#endif

#define REG_PATH "com/great/happyness/mediacodecmy/NativeCodec"

JavaVM*		g_javaVM		= NULL;
H264Sender 	*mpH264Sender 	= NULL;
TcpSender	*mpTcpSender	= NULL;

static jboolean CreateSender(JNIEnv *env, jobject, jstring destip, jint destport, jint sendPort)
{
	jboolean bRet = JNI_FALSE;

	if(NULL == mpH264Sender) {
		mpH264Sender = new H264Sender();
	}

	const char *ip = env->GetStringUTFChars(destip, NULL);

	if(!mpH264Sender->initSession(sendPort))
	{
		GLOGE("TAG 2,function %s,line:%d mpSender->initSession() failed.", __FUNCTION__, __LINE__);
		SAFE_DELETE(mpH264Sender);
		return false;
	}
	else
		bRet = mpH264Sender->connect(std::string(ip), destport);

	env->ReleaseStringUTFChars(destip, ip);

	return bRet;
}

static jboolean ReleaseSender(JNIEnv *env, jobject){
	bool bRes = false;
	if(mpH264Sender) {
		bRes = mpH264Sender->deinitSession();
		SAFE_DELETE(mpH264Sender);
	}
	return bRes;
}

static jboolean StartFileSender(JNIEnv *env, jobject, jstring sfilename)
{
	 bool bRes = false;

		 jboolean isCopy = JNI_FALSE;
		 const char *fn = env->GetStringUTFChars(sfilename, NULL);
		 GLOGV("Enter:AudioConnectDest->strIP:%s,isCopy:%c",fn, isCopy);

		 mpH264Sender->startFileSend((char*)fn);

		 env->ReleaseStringUTFChars(sfilename, fn);


	 return bRes;
}

static jboolean StopFileSender(JNIEnv *env, jobject){
	return mpH264Sender->stopFileSend();
}


/////////////////////////////////////////////////////TcpSender////////////////////////////////////////////////////////

static jboolean TcpConnect(JNIEnv *env, jobject, jstring destip, jint destport)
{
	jboolean bRet = JNI_FALSE;

	if(NULL == mpTcpSender) {
		mpTcpSender = new TcpSender();
	}

	const char *ip = env->GetStringUTFChars(destip, NULL);

	bRet = mpTcpSender->connect(ip, destport);

	env->ReleaseStringUTFChars(destip, ip);

	return bRet;
}

static jboolean TcpDisconnect(JNIEnv *env, jobject){
	bool bRes = false;
	if(mpTcpSender) {
		bRes = mpTcpSender->disConnect();
		SAFE_DELETE(mpTcpSender);
	}
	return bRes;
}

static jboolean TcpStartFileSender(JNIEnv *env, jobject, jstring sfilename)
{
	 bool bRes = false;

	 jboolean isCopy = JNI_FALSE;
	 const char *fn = env->GetStringUTFChars(sfilename, NULL);
	 GLOGV("Enter:AudioConnectDest->strIP:%s,isCopy:%c",fn, isCopy);

	 mpTcpSender->startFileSend((char*)fn);

	 env->ReleaseStringUTFChars(sfilename, fn);

	 return bRes;
}

static jboolean TcpStopFileSender(JNIEnv *env, jobject){
	return mpTcpSender->stopFileSend();
}

static JNINativeMethod video_method_table[] = {
		//
		{"CreateSender", "(Ljava/lang/String;II)Z", (void*)CreateSender },
		{"ReleaseSender", "()Z", (void*)ReleaseSender },
		{"StartFileSender", "(Ljava/lang/String;)Z", (void*)StartFileSender },
		{"StopFileSender", "()Z", (void*)StopFileSender },

		{"TcpConnect", "(Ljava/lang/String;I)Z", (void*)TcpConnect },
		{"TcpDisconnect", "()Z", (void*)TcpDisconnect },
		{"TcpStartFileSender", "(Ljava/lang/String;)Z", (void*)TcpStartFileSender },
		{"TcpStopFileSender", "()Z", (void*)TcpStopFileSender },
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


