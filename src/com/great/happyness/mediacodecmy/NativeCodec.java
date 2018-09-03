package com.great.happyness.mediacodecmy;

import android.annotation.SuppressLint;
import android.media.MediaCodec;
import android.util.Log;

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
			System.loadLibrary("netcodec");  //
		}
		catch(Throwable e)
		{
			Log.e(TAG, "loadLibrary:"+e.toString());
		}
	}
    
    
	public native boolean CreateSender(String destip, int destport, int sendPort);
	public native boolean ReleaseSender();
	public native boolean StartFileSender(String filename);
	public native boolean StopFileSender();
}

