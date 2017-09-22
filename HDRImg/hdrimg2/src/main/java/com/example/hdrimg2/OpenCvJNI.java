package com.example.hdrimg2;

import android.graphics.Bitmap;


public class OpenCvJNI {
    static {
        System.loadLibrary("renderJni");
    }
    public native int[] process(int[] pixels, int w, int h);

    public Bitmap arrayToBitmap(int[] array, int width , int height){
        Bitmap resultImg = Bitmap.createBitmap(array, width, height, Bitmap.Config.ARGB_8888);
        return resultImg;
    }

    public Bitmap opencvTest(Bitmap srcBitmap)
    {
        int w = srcBitmap.getWidth();
        int h = srcBitmap.getHeight();
        int[] pixels = new int[w*h];
        srcBitmap.getPixels(pixels, 0, w, 0, 0, w, h);

        int[] resultInt = process(pixels, w, h);            //处理

        Bitmap resultImg = Bitmap.createBitmap(w, h, Bitmap.Config.ARGB_8888);
        resultImg.setPixels(resultInt, 0, w, 0, 0, w, h);
        return resultImg;
    }
}
