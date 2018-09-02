/*
 * RtpSender.h
 *
 *  Created on: 
 *      Author: 
 */



#ifndef RtpSender_H_
#define RtpSender_H_

#include "rtpsession.h"
#include "rtpudpv4transmitter.h"
#include "rtpipv4address.h"
#include "rtpsessionparams.h"


#define TAG "RtpSender"
#include "common.h"

using namespace std;
using namespace jrtplib;


class RtpSender
{
	public:
		RtpSender();
		virtual ~RtpSender();
		bool initSession(unsigned short localPort);
		bool deinitSession();
		bool connect(std::string sDestIp, unsigned short destPort);
		bool disConnect();

		bool sendBuffer(void*buff, int dataLen, int64_t timeStamp);
		bool sendBuffer(void*buff, int dataLen, char*hdrextdata, int numhdrextwords, int64_t timeStamp);
		bool sendBufferEx(void*buff, int dataLen, int64_t timeStamp);
		bool sendBufferEx1(unsigned char* sendBuf,int buflen);
		
	protected:
		inline string intToString(int val);
		inline void CheckError(int rtperr);

	private:
		RTPSession		mSession;
		unsigned short			mLocalPort;
		bool			mInited;
		bool    		mbConnected;
		RTPIPv4Address	mDestAddr;
};

#endif /* RtpSender_H_ */


