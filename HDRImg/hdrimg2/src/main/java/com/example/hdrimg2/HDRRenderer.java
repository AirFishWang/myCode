package com.example.hdrimg2;

import android.opengl.GLSurfaceView;
import android.util.Log;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;


public class HDRRenderer implements GLSurfaceView.Renderer {

    public native void init(int imageWidth, int imageHeight, int gridWidth, int gridHeight, int gridDepth, String binFilePath, int modelType);

    //upload src image(BGRA)
    public native void upLoadImg(String imagePath, int type);

    //coeffsData: calculate by tensorflow
    //return int array, every element's foramt is BGRA
    public native int[] render(float[] coeffsData, int srcWidth, int srcHeight);

    //when calculate a image, you must release the state of the Render in order to deal another iamge
    public native void freeCurrentState();

    public native float[] getSmallImage(String imagePath, int type);

    public native void clearTime();

    public native void glesResize(int width, int height);

    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        Log.i("onSurfaceCreated", "onSurfaceCreated begin");
        Log.i("onSurfaceCreated", "onSurfaceCreated end");
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        Log.e("onSurfaceChanged", "width: "+ width + "height: " + height);
    }
    @Override
    public void onDrawFrame(GL10 gl) {

    }
}
