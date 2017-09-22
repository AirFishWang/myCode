package com.example.astest;

import android.app.Activity;
import android.widget.TextView;

public class TestA {
    public Activity activity;
    public TestA(Activity activity){
        this.activity = activity;
    }

    public void showTestA(){
        ((TextView) activity.findViewById(R.id.text)).setText("this is TestA");
    }
}
