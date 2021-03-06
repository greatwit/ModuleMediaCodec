package com.great.happyness.mediacodecmy;

import android.annotation.SuppressLint;
import android.media.MediaCodec;
import android.util.Log;
import android.view.Surface;

@SuppressLint("NewApi") 
public class NativeCodec 
{
	private static String TAG = "NativeCodec";
	
	private static MediaCodec mCodec = null;
	
	public NativeCodec()
	{
	}
	
	static
	{
		try
		{
			System.loadLibrary("mcndk");
			//System.loadLibrary("netcodec");  //
		}
		catch(Throwable e)
		{
			Log.e(TAG, "loadLibrary:"+e.toString());
		}
	}
	public native boolean StartExtratorPlayer(String filepath, Surface surface);
	public native boolean StopExtratorPlayer();
	
	public native boolean StartFileDecodec(Surface surface);
	public native boolean StopFileDecodec();
	
//	public native boolean CreateSender(String destip, int destport, int sendPort);
//	public native boolean ReleaseSender();
//	public native boolean StartFileSender(String filename);
//	public native boolean StopFileSender();
//	
//	public native boolean TcpConnect(String destip, int destport);
//	public native boolean TcpDisconnect(); 
//	public native boolean TcpStartFileSender(String filename);
//	public native boolean TcpStopFileSender();
}

