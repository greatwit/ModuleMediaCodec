package com.great.happyness.mediacodecmy;

import android.app.Activity;
import android.os.Bundle;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class McNdkActivity extends Activity implements SurfaceHolder.Callback{
	private SurfaceHolder holder = null;
	private NativeCodec mCodec = new NativeCodec();
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_encode);
		SurfaceView sfv_video = (SurfaceView) findViewById(R.id.surfaceViewPlay);
		holder = sfv_video.getHolder();
		holder.addCallback(this);
	}
	
	protected void onDestroy() {
		super.onDestroy();
	}
	
	@Override
	public void surfaceCreated(SurfaceHolder holder) {
		//mCodec.StartFileDecodec(holder.getSurface());
		mCodec.StartExtratorPlayer("/sdcard/tmp.mp4", holder.getSurface());
	}

	@Override
	public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

	}

	@Override
	public void surfaceDestroyed(SurfaceHolder holder) {
		//mCodec.StopFileDecodec();
		mCodec.StopExtratorPlayer();
	}
	
}
