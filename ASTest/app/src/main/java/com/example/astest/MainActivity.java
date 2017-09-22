package com.example.astest;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {
    public static final String TAG = "MainActicity";
    private Button mButuon;
    private Button AButton;
    private TextView mTextView;
    private int mTest = 0;
    private TestA testA = new TestA(this);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.e(TAG, "onCreate");
        setContentView(R.layout.activity_main);

        mTextView = (TextView) findViewById(R.id.text);
        mTextView.setText(""+ mTest);
        mButuon = (Button) findViewById(R.id.button);
        mButuon.setOnClickListener(new View.OnClickListener(){
            public void onClick(View v){
                mTest = 2;
                Log.e("MainActivity", "mTest = " + mTest);
                mTextView.setText("" + mTest);
            }
        });

        AButton.setOnClickListener(new View.OnClickListener(){
            public void onClick(View v){
                testA.showTestA();
            }
        });
    }

    @Override
    protected void onStart() {
        super.onStart();
        Log.e(TAG, "onStart");
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.e("MainActivity", "onResume: mTest = " + mTest);
        Log.e(TAG, "onResume");
    }

    @Override
    protected void onPause() {
        super.onPause();
        Log.e("MainActivity", "onPause: mTest = " + mTest);
        Log.e(TAG, "onPause");
    }

    @Override
    protected void onStop() {
        super.onStop();
        Log.e(TAG, "onStop");
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Log.e(TAG, "onDestroy");
    }

    @Override
    protected void onRestart() {
        super.onRestart();
        Log.e(TAG, "onRestart");
    }
}
