/*
 * RtpSender.cpp
 *
 *  Created on: 
 *      Author: 
 */

#include "RtpSender.h"


#define SEND_PACKETSIZE		36000

 
RtpSender::RtpSender()
		:mInited(false)
		,mbConnected(false)
{
	GLOGV("RtpSender::RtpSender construct.");
}

RtpSender::~RtpSender() 
{
	// TODO Auto-generated destructor stub
	mInited = false;
}

  
bool RtpSender::initSession(unsigned short localPort)
{   
	int iStatus=0;

	if(mInited)
		return false;

	mLocalPort = localPort;

	// Now, we'll create a RTP session, set the destination, send some
	// packets and poll for incoming data.

	RTPSessionParams sessparams;
	RTPUDPv4TransmissionParams transparams;

	// IMPORTANT: The local timestamp unit MUST be set, otherwise
	//            RTCP Sender Report info will be calculated wrong
	// In this case, we'll be sending 10 samples each second, so we'll
	// put the timestamp unit to (1.0/10.0)
	sessparams.SetOwnTimestampUnit(1.0/8000.0);//必须设置	
	//sessparams.SetUsePollThread(true);

	sessparams.SetAcceptOwnPackets(false); 
	transparams.SetPortbase(mLocalPort);

	//transparams.SetMaximumPacketSize(60000);
	iStatus = mSession.Create(sessparams, &transparams);

    string cname = sessparams.GetCNAME();
	if(iStatus>=0)
	{
		mSession.SetDefaultPayloadType(96);
		mSession.SetDefaultMark(true);
		mSession.SetMaximumPacketSize(MAX_PACKET_SIZE);
		mSession.SetDefaultTimestampIncrement(1);
	}
	else
	{
		GLOGE("function: %s, line: %d, error: %s portnum:%d", __FUNCTION__, __LINE__, RTPGetErrorString(iStatus).c_str(), mLocalPort);
		return false;
	}

	GLOGV("function %s,line:%d RtpSender::initSession() cname:%s",__FUNCTION__,__LINE__, cname.c_str());


	mInited = true;

	return true;
}

bool RtpSender::deinitSession()
{
	if(!mInited)
		return mInited;

	disConnect();

	mSession.BYEDestroy(RTPTime(1,0), 0, 0);

	mInited = false;

	return true;
}

bool RtpSender::connect(std::string sDestIp, unsigned short destPort)
{
	int iStatus=0;

	if(!mInited)
	{
		GLOGW("function %s,line:%d do not inited ",__FUNCTION__,__LINE__);
		return mInited;
	}

	if(mbConnected)
		return true;
	
	uint32_t uiDestip = inet_addr(sDestIp.c_str());
	if (uiDestip == INADDR_NONE)
	{
		GLOGE("function %s,line:%d Bad IP address specified ip:%s ",__FUNCTION__,__LINE__, sDestIp.c_str());
		return false;
	}

	uiDestip = ntohl(uiDestip);

	mDestAddr.SetIP(uiDestip);
	mDestAddr.SetPort(destPort);

	iStatus = mSession.AddDestination(mDestAddr);
	if(iStatus<0)
	{
		GLOGE("function: %s, line: %d, error: %s", __FUNCTION__, __LINE__, RTPGetErrorString(iStatus).c_str());
		if(ERR_RTP_HASHTABLE_ELEMENTALREADYEXISTS != iStatus)
			return false;
	}

	mbConnected = true;


	GLOGD("function %s,line:%d RtpSender::connect ",__FUNCTION__,__LINE__);
	return true;
}


bool RtpSender::disConnect()
{
	if(mbConnected)
	{
		mSession.DeleteDestination(mDestAddr);
		GLOGW("function %s,line:%d RtpSender::disConnect 1",__FUNCTION__,__LINE__);
		mSession.ClearDestinations();
		GLOGW("function %s,line:%d RtpSender::disConnect 2",__FUNCTION__,__LINE__);
		mbConnected = false;
	}
	return true;
}

bool RtpSender::sendBuffer(void*buff, int dataLen,  char*hdrextdata, int numhdrextwords, int64_t timeStamp)
{
	int iStatus=0;

	if(!mInited)
		return mInited;

	if(dataLen>=MAX_PACKET_SIZE)
	{
		GLOGE("function: %s, line: %d, data len big than max_packet_size.", __FUNCTION__, __LINE__);
		return false;
	}
	
	iStatus = mSession.SendPacketEx((void *)buff, dataLen, 0, hdrextdata, numhdrextwords);  

	if(iStatus<0)
	{
		GLOGE("function: %s, line: %d, error: %s", __FUNCTION__, __LINE__, RTPGetErrorString(iStatus).c_str());
		return false;
	}
	else
		GLOGV("function: %s, line: %d, sendBuffer len:%d", __FUNCTION__, __LINE__, dataLen);

#ifndef RTP_SUPPORT_THREAD
	
	iStatus = mSession.Poll();
	if(iStatus<0)
	{
		GLOGE("function: %s, line: %d, error: %s", __FUNCTION__, __LINE__, RTPGetErrorString(iStatus).c_str());
		return false;
	}
#endif // RTP_SUPPORT_THREAD

	return true;
}

bool RtpSender::sendBuffer(void*buff, int dataLen, int64_t timeStamp)
{
	int iStatus=0;

	if(!mInited)
		return mInited;

	iStatus = mSession.SendPacket((void *)buff, dataLen, 0, false, timeStamp);
	if(iStatus<0)
	{
		GLOGE("function: %s, line: %d, error: %s", __FUNCTION__, __LINE__, RTPGetErrorString(iStatus).c_str());
		return false;
	}

#ifndef RTP_SUPPORT_THREAD
	iStatus = mSession.Poll();
	if(iStatus<0)
	{
		GLOGE("function: %s, line: %d, error: %s", __FUNCTION__, __LINE__, RTPGetErrorString(iStatus).c_str());
		return false;
	}
#endif // RTP_SUPPORT_THREAD

	return true;
}



bool RtpSender::sendBufferEx1(unsigned char* sendBuf,int buflen)   
{   
	unsigned char *pSendTemp = sendBuf;  
	int packetSize=SEND_PACKETSIZE;
	int status = 0;  
	char sendbuf[SEND_PACKETSIZE];   //发送的数据缓冲  
	memset(sendbuf,0,SEND_PACKETSIZE);  

	if ( buflen <= packetSize )  
	{   
		memcpy(sendbuf,pSendTemp,buflen);
		status = mSession.SendPacket((void *)sendbuf,buflen + 1);  
		if(status<0)
			GLOGE("function: %s, line: %d, error: %s", __FUNCTION__, __LINE__, RTPGetErrorString(status).c_str());
	}    
	else if(buflen > packetSize )  
	{   
		//设置标志位Mark为0  
		mSession.SetDefaultMark(false);  
		//printf("buflen = %d\n",buflen);  
		//得到该需要用多少长度为MAX_RTP_PKT_LENGTH字节的RTP包来发送  
		int fullLen=0,tailLen=0;    
		fullLen = buflen / packetSize;  
		tailLen = buflen % packetSize;  
		int t=0;//用指示当前发送的是第几个分片RTP包  

		char nalHeader = pSendTemp[0]; // NALU
		if(0 == tailLen)
		{
			fullLen--;
		}

		for(t=0;t<fullLen;t++)
		{      
			memcpy(sendbuf, &pSendTemp[t*packetSize], packetSize);  
			status = mSession.SendPacket((void *)sendbuf,packetSize);   
			if(status<0)
				GLOGE("function: %s, line: %d, error: %s", __FUNCTION__, __LINE__, RTPGetErrorString(status).c_str());
		}   

		//设置标志位Mark为1  
		mSession.SetDefaultMark(true);
		int iSendLen;  
		iSendLen = buflen - t*packetSize;  
		memcpy(sendbuf,&pSendTemp[t*packetSize], iSendLen);  
		status = mSession.SendPacket((void *)sendbuf, iSendLen);  
		if(status<0)
			GLOGE("function: %s, line: %d, error: %s", __FUNCTION__, __LINE__, RTPGetErrorString(status).c_str());
	}  
	return true;
} 

 string RtpSender::intToString(int val)
{
	char temp[8];   
	sprintf(temp, "%d", val);  
	string s(temp);
	return s;
}

bool RtpSender::sendBufferEx(void*buff, int buflen, int64_t timeStamp)
{
	unsigned char *pSendTemp = (unsigned char *)buff;  
	int packetSize=SEND_PACKETSIZE;
	int status = 0;  
	char sendbuf[SEND_PACKETSIZE];   //发送的数据缓冲  
	memset(sendbuf,0,SEND_PACKETSIZE);  

	printf("send packet length %d \n",buflen);  

	if ( buflen <= packetSize )  
	{   
		memcpy(sendbuf,pSendTemp,buflen); 
		mSession.SendPacket((void *)sendbuf, buflen, 0, false, timeStamp);
		if(status<0)
			GLOGE("function: %s, line: %d, error: %s", __FUNCTION__, __LINE__, RTPGetErrorString(status).c_str());
	}    
	else if(buflen > packetSize )  
	{  
		//设置标志位Mark为0  
		mSession.SetDefaultMark(false);  
		//printf("buflen = %d\n",buflen);  
		//得到该需要用多少长度为MAX_RTP_PKT_LENGTH字节的RTP包来发送  
		int fullLen=0,tailLen=0;    
		fullLen = buflen / packetSize;  
		tailLen = buflen % packetSize;  
		int t=0;//用指示当前发送的是第几个分片RTP包  

		char nalHeader = pSendTemp[0]; // NALU
		string headerTem = "";
		string header="";
		if(0 == tailLen)
		{
			header=intToString(fullLen)+":";
			fullLen--;
		}
		else
		{
			header=intToString(fullLen+1)+":";
		}

		for(t=0;t<fullLen;t++)
		{   
			headerTem=header+intToString(t);
			memcpy(sendbuf,&pSendTemp[t*packetSize],packetSize);  
			status = mSession.SendPacketEx((void *)sendbuf,packetSize, t, headerTem.c_str(), headerTem.length());   
			if(status<0)
				GLOGE("function: %s, line: %d, error: %s", __FUNCTION__, __LINE__, RTPGetErrorString(status).c_str());
		}   

		//设置标志位Mark为1  
		mSession.SetDefaultMark(true);
		int iSendLen;  
		headerTem=header+intToString(t);
		iSendLen = buflen - t*packetSize;  
		memcpy(sendbuf,&pSendTemp[t*packetSize],iSendLen);  
		status = mSession.SendPacket((void *)sendbuf,iSendLen, t, headerTem.c_str(), headerTem.length());  
		if(status<0)
			GLOGE("function: %s, line: %d, error: %s", __FUNCTION__, __LINE__, RTPGetErrorString(status).c_str());
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////
