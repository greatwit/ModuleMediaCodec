/*
 * RtpSender.cpp
 *
 *  Created on: 
 *      Author: 
 */

#include "H264Sender.h"


#define SEND_PACKETSIZE		36000

 
H264Sender::H264Sender()
		:mInited(false)
		,mbConnected(false)
		,mFile(NULL)
		,mRunning(false)
{
	GLOGV("RtpSender::RtpSender construct.");
}

H264Sender::~H264Sender()
{
	// TODO Auto-generated destructor stub
	mInited = false;
}

  
bool H264Sender::initSession(unsigned short localPort)
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

bool H264Sender::deinitSession()
{
	if(!mInited)
		return mInited;

	disConnect();

	mSession.BYEDestroy(RTPTime(1,0), 0, 0);

	mInited = false;

	return true;
}

bool H264Sender::connect(std::string sDestIp, unsigned short destPort)
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


bool H264Sender::disConnect()
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

bool H264Sender::startFileSend(char*filename){
	mFile = OpenBitstreamFile(filename);
	if (GThread::IsRunning())
		return ERR_RTP_POLLTHREAD_ALREADYRUNNING;
	mRunning = true;
	if (GThread::Start() < 0)
		return ERR_RTP_POLLTHREAD_CANTSTARTTHREAD;
	return true;
}

bool H264Sender::stopFileSend(){

	RTPTime thetime = RTPTime::CurrentTime();
	bool done = false;

	while (GThread::IsRunning() && !done)
	{
		// wait max 5 sec
		RTPTime curtime = RTPTime::CurrentTime();
		if ((curtime.GetDouble() - thetime.GetDouble()) > 5.0)
			done = true;
		RTPTime::Wait(RTPTime(0,10000));
	}

	if (GThread::IsRunning()) {
		GThread::Kill();
	}
	mRunning = false;
	CloseBitstreamFile(mFile);

	return true;
}

void *H264Sender::Thread(){
	GThread::ThreadStarted();

	int status = 0;
	NALU_HEADER		*nalu_hdr;
	FU_INDICATOR	*fu_ind;
	FU_HEADER		*fu_hdr;
	char sendbuf[1500];
	char* nalu_payload;
	unsigned int timestamp_increse=0,ts_current=0;

	NALU_t *n = AllocNALU(8000000);
	bool start=false;
	while (mRunning){
		if(feof(mFile)) {
			mRunning = false;
			break;
		}
		int size=GetAnnexbNALU(mFile, n);//每执行一次，文件的指针指向本次找到的NALU的末尾，下一个位置即为下个NALU的起始码0x000001
		if(size<4)
		{
			printf("get nul error!\n");
			continue;
		}
		dump(n);//输出NALU长度和TYPE
		if(!start)
		{
			if(n->nal_unit_type==5||n->nal_unit_type==6||
					n->nal_unit_type==7||n->nal_unit_type==7)
			{
				printf("begin\n");
				start=true;
			}
		}
		//	当一个NALU小于MAX_RTP_PKT_LENGTH字节的时候，采用一个单RTP包发送
			if(n->len<=MAX_RTP_PKT_LENGTH)
			{
				//printf("ddd0\n");
				//session.SetDefaultMark(false);
				//设置NALU HEADER,并将这个HEADER填入sendbuf[12]
				nalu_hdr =(NALU_HEADER*)&sendbuf[0]; //将sendbuf[12]的地址赋给nalu_hdr，之后对nalu_hdr的写入就将写入sendbuf中；
				nalu_hdr->F=n->forbidden_bit;
				nalu_hdr->NRI=n->nal_reference_idc>>5;//有效数据在n->nal_reference_idc的第6，7位，需要右移5位才能将其值赋给nalu_hdr->NRI。
				nalu_hdr->TYPE=n->nal_unit_type;

				nalu_payload=&sendbuf[1];//同理将sendbuf[13]赋给nalu_payload
				memcpy(nalu_payload,n->buf+1,n->len-1);//去掉nalu头的nalu剩余内容写入sendbuf[13]开始的字符串。
				ts_current=ts_current+timestamp_increse;

				//status = session.SendPacket((void *)sendbuf,n->len);
				if(n->nal_unit_type==1 || n->nal_unit_type==5)
				{
					status = mSession.SendPacket((void *)sendbuf,n->len,96,true,3600);
				}
				else
				{
						status = mSession.SendPacket((void *)sendbuf,n->len,96,true,0);\
						//如果是6,7类型的包，不应该延时；之前有停顿，原因这在这
						continue;
				}
				//发送RTP格式数据包并指定负载类型为96
				if (status < 0)
				{
					std::cerr << RTPGetErrorString(status) << std::endl;
					exit(-1);
				}

			}
			else if(n->len>MAX_RTP_PKT_LENGTH)
			{
				//得到该nalu需要用多少长度为MAX_RTP_PKT_LENGTH字节的RTP包来发送
				int k=0,l=0;
				k=n->len/MAX_RTP_PKT_LENGTH;//需要k个MAX_RTP_PKT_LENGTH字节的RTP包
				l=n->len%MAX_RTP_PKT_LENGTH;//最后一个RTP包的需要装载的字节数
				int t=0;//用于指示当前发送的是第几个分片RTP包
				ts_current=ts_current+timestamp_increse;
				while(t<=k)
				{

					if(!t)//发送一个需要分片的NALU的第一个分片，置FU HEADER的S位
					{
						//printf("dddd1");
						memset(sendbuf,0,1500);
						//session.SetDefaultMark(false);
						//设置FU INDICATOR,并将这个HEADER填入sendbuf[12]
						fu_ind =(FU_INDICATOR*)&sendbuf[0]; //将sendbuf[12]的地址赋给fu_ind，之后对fu_ind的写入就将写入sendbuf中；
						fu_ind->F=n->forbidden_bit;
						fu_ind->NRI=n->nal_reference_idc>>5;
						fu_ind->TYPE=28;

						//设置FU HEADER,并将这个HEADER填入sendbuf[13]
						fu_hdr =(FU_HEADER*)&sendbuf[1];
						fu_hdr->E=0;
						fu_hdr->R=0;
						fu_hdr->S=1;
						fu_hdr->TYPE=n->nal_unit_type;


						nalu_payload=&sendbuf[2];//同理将sendbuf[14]赋给nalu_payload
						memcpy(nalu_payload,n->buf+1,MAX_RTP_PKT_LENGTH);//去掉NALU头

						//status = session.SendPacket((void *)sendbuf,MAX_RTP_PKT_LENGTH+2);
						status = mSession.SendPacket((void *)sendbuf,MAX_RTP_PKT_LENGTH+2,96,false,0);
						if (status < 0)
						{
							std::cerr << RTPGetErrorString(status) << std::endl;
							exit(-1);
						}
						t++;
					}
					//发送一个需要分片的NALU的非第一个分片，清零FU HEADER的S位，如果该分片是该NALU的最后一个分片，置FU HEADER的E位
					else if(k==t)//发送的是最后一个分片，注意最后一个分片的长度可能超过MAX_RTP_PKT_LENGTH字节（当l>1386时）。
					{
						//printf("dddd3\n");
						memset(sendbuf,0,1500);
						//session.SetDefaultMark(true);
						//设置FU INDICATOR,并将这个HEADER填入sendbuf[12]
						fu_ind =(FU_INDICATOR*)&sendbuf[0]; //将sendbuf[12]的地址赋给fu_ind，之后对fu_ind的写入就将写入sendbuf中；
						fu_ind->F=n->forbidden_bit;
						fu_ind->NRI=n->nal_reference_idc>>5;
						fu_ind->TYPE=28;

						//设置FU HEADER,并将这个HEADER填入sendbuf[13]
						fu_hdr =(FU_HEADER*)&sendbuf[1];
						fu_hdr->R=0;
						fu_hdr->S=0;
						fu_hdr->TYPE=n->nal_unit_type;
						fu_hdr->E=1;
						nalu_payload=&sendbuf[2];//同理将sendbuf[14]赋给nalu_payload
						memcpy(nalu_payload,n->buf+t*MAX_RTP_PKT_LENGTH+1,l-1);//将nalu最后剩余的l-1(去掉了一个字节的NALU头)字节内容写入sendbuf[14]开始的字符串。

						//status = session.SendPacket((void *)sendbuf,l+1);
						status = mSession.SendPacket((void *)sendbuf,l+1,96,true,3600);
						if (status < 0)
						{
							std::cerr << RTPGetErrorString(status) << std::endl;
							exit(-1);
						}
						t++;
					//	Sleep(100);
					}
					else if(t<k&&0!=t)
					{
						//printf("dddd2");
						memset(sendbuf,0,1500);
						//session.SetDefaultMark(false);
						//设置FU INDICATOR,并将这个HEADER填入sendbuf[12]
						fu_ind =(FU_INDICATOR*)&sendbuf[0]; //将sendbuf[12]的地址赋给fu_ind，之后对fu_ind的写入就将写入sendbuf中；
						fu_ind->F=n->forbidden_bit;
						fu_ind->NRI=n->nal_reference_idc>>5;
						fu_ind->TYPE=28;

						//设置FU HEADER,并将这个HEADER填入sendbuf[13]
						fu_hdr =(FU_HEADER*)&sendbuf[1];
						//fu_hdr->E=0;
						fu_hdr->R=0;
						fu_hdr->S=0;
						fu_hdr->E=0;
						fu_hdr->TYPE=n->nal_unit_type;

						nalu_payload=&sendbuf[2];//同理将sendbuf[14]的地址赋给nalu_payload
						memcpy(nalu_payload,n->buf+t*MAX_RTP_PKT_LENGTH+1,MAX_RTP_PKT_LENGTH);//去掉起始前缀的nalu剩余内容写入sendbuf[14]开始的字符串。

						//status = session.SendPacket((void *)sendbuf,MAX_RTP_PKT_LENGTH+2);
						status = mSession.SendPacket((void *)sendbuf,MAX_RTP_PKT_LENGTH+2,96,false,0);
						if (status < 0)
						{
							std::cerr << RTPGetErrorString(status) << std::endl;
							exit(-1);
						}
						t++;
					}
				}
			}

		RTPTime delay(0.040);
		RTPTime::Wait(delay);
	}

	return 0;
}

bool H264Sender::sendBuffer(void*buff, int dataLen,  char*hdrextdata, int numhdrextwords, int64_t timeStamp)
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

bool H264Sender::sendBuffer(void*buff, int dataLen, int64_t timeStamp)
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



bool H264Sender::sendBufferEx1(unsigned char* sendBuf,int buflen)
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

 string H264Sender::intToString(int val)
{
	char temp[8];   
	sprintf(temp, "%d", val);  
	string s(temp);
	return s;
}

bool H264Sender::sendBufferEx(void*buff, int buflen, int64_t timeStamp)
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
