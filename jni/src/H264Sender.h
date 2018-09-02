/*
 * H264Sender.h
 *
 *  Created on: 
 *      Author: 
 */



#ifndef H264Sender_H_
#define H264Sender_H_

#include "rtpsession.h"
#include "rtpudpv4transmitter.h"
#include "rtpipv4address.h"
#include "rtpsessionparams.h"

#include "h264.h"
#include "gthread.h"

#include <iostream>

#define TAG "RtpSender"
#include "common.h"

using namespace std;
using namespace jrtplib;


class H264Sender : private GThread
{
	public:
		H264Sender();
		virtual ~H264Sender();
		bool initSession(unsigned short localPort);
		bool deinitSession();
		bool connect(std::string sDestIp, unsigned short destPort);
		bool disConnect();

		bool startFileSend(char*filename);
		bool stopFileSend();

		void *Thread();

		bool sendBuffer(void*buff, int dataLen, int64_t timeStamp);
		bool sendBuffer(void*buff, int dataLen, char*hdrextdata, int numhdrextwords, int64_t timeStamp);
		bool sendBufferEx(void*buff, int dataLen, int64_t timeStamp);
		bool sendBufferEx1(unsigned char* sendBuf,int buflen);
		
	protected:
		inline string intToString(int val);
		inline void CheckError(int rtperr);

	private:
		FILE			*mFile;
		bool 			mRunning;
		RTPSession		mSession;
		unsigned short			mLocalPort;
		bool			mInited;
		bool    		mbConnected;
		RTPIPv4Address	mDestAddr;
};

#endif /* RtpSender_H_ */


