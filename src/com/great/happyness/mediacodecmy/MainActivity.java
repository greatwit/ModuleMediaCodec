package com.great.happyness.mediacodecmy;


import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

public class MainActivity extends Activity implements View.OnClickListener{
	
	private Button btnEncode, btnDecode, btnStartSend, btnStopSend;
	private NativeCodec mCodec = new NativeCodec();

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		initWidgets();
	}
	
	private void initWidgets(){
		findViewById(R.id.btnEncode).setOnClickListener(this);
		findViewById(R.id.btnDecode).setOnClickListener(this);
		findViewById(R.id.btnStartSend).setOnClickListener(this);
		findViewById(R.id.btnStopSend).setOnClickListener(this);
	}

	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
        switch (v.getId()) {
        case R.id.btnEncode:
			startActivity(new Intent(MainActivity.this, EncodeActivity.class));
            break;
        case R.id.btnDecode:
        	startActivity(new Intent(MainActivity.this, DecodeActivity.class));
        	break;
        case R.id.btnStartSend:
        	//mCodec.CreateSender("", 9000, 8000);
        	//mCodec.StartFileSender("");
        	mCodec.TcpConnect("192.168.1.106", 8080);
        	mCodec.TcpStartFileSender("/storage/emulated/0/camera.h264");
        	break;
        case R.id.btnStopSend:
        	//mCodec.StopFileSender();
        	//mCodec.ReleaseSender();
        	mCodec.TcpStopFileSender();
        	mCodec.TcpDisconnect();
        	break;
        }
	}
}
