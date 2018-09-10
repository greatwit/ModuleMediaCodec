#ifndef TcpSender_H_
#define TcpSender_H_

#include "h264.h"
#include "gthread.h"

#include <iostream>

#define TAG "H264Sender"
#include "common.h"



class TcpSender : private GThread
{
	public:
	TcpSender();
		virtual ~TcpSender();

		bool connect(const char* destIp, unsigned short destPort);
		bool disConnect();

		bool startFileSend(char*filename);
		bool stopFileSend();

		void *Thread();

	private:
		int 			mSockId;
		FILE			*mFile;
		bool 			mRunning;
		bool			mInited;
};

#endif
