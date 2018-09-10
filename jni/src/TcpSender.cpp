
#include "TcpSender.h"

#include "rtptimeutilities.h"

#include "spioutils.h"
#include <unistd.h>


using namespace std;
using namespace jrtplib;

TcpSender::TcpSender()
		:mInited(false)
		,mFile(NULL)
		,mRunning(false)
		,mSockId(-1)
{

	GLOGD("TcpSender::TcpSender construct.");
}

TcpSender::~TcpSender()
{
	// TODO Auto-generated destructor stub
	mInited = false;
}

bool TcpSender::connect(const char* destIp, unsigned short destPort){
	int ret = SP_IOUtils::tcpConnect(destIp, destPort, &mSockId, 0);
	GLOGD("connect ret:%d sockid:%d.",ret, mSockId);
	return true;
}

bool TcpSender::disConnect(){
	if(mSockId>0)
		close(mSockId);
	GLOGE("disConnect");
	return true;
}

bool TcpSender::startFileSend(char*filename){
	mFile = OpenBitstreamFile(filename);
	GLOGE("filename:%s", filename);
	if (GThread::IsRunning())
		return false;

	if (GThread::Start() < 0)
		return false;
	mRunning = true;
	return true;
}

bool TcpSender::stopFileSend(){
	bool done = false;
	RTPTime thetime = RTPTime::CurrentTime();
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

void *TcpSender::Thread(){
	GThread::ThreadStarted();

	int status = 0;
	NALU_HEADER		*nalu_hdr;
	FU_INDICATOR	*fu_ind;
	FU_HEADER		*fu_hdr;
	//char sendbuf[1500];
	NALU_t *n = AllocNALU(8000000);
	bool start=false;
	while (mRunning){
		if(feof(mFile)) {
			mRunning = false;
			break;
		}
		int size=GetAnnexbNALU(mFile, n);//每执行一次，文件的指针指向本次找到的NALU的末尾，下一个位置即为下个NALU的起始码0x000001
		GLOGE("GetAnnexbNALU size:%d", size);
		if(size<4)
		{
			printf("get nul error!\n");
			continue;
		}
		SP_IOUtils::tcpSendData(mSockId, (char*)n->buf, n->len);
		RTPTime delay(0.040);
		RTPTime::Wait(delay);
	}
	return 0;
}
