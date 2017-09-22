package com.example.hdrimg2;

import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.opengl.GLSurfaceView;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

import org.tensorflow.contrib.android.TensorFlowInferenceInterface;

public class HDRGLSurfaceView extends GLSurfaceView {
    static {
        System.loadLibrary("renderJni");
        System.loadLibrary("tensorflow_inference");
    }

    public HDRRenderer mRenderer;
    private TensorFlowInferenceInterface tfi;
    private static String  inputName = "lowres_input";
    private static String  outputName = "output_coefficients";
    private static int netInputSize = 256;      //lowres_input size
    private String binFilePath;

    private int gridDepth;
    private int gridHeight;
    private int gridWidth;
    private int[] resultArray;
    private Handler hander;
    private AssetManager mgr;

    private double tensorTotalTime = 0d;

    public HDRGLSurfaceView(Context context, AssetManager assetManager, String binFilePath, Handler handerUI){
        super(context);
        hander = handerUI;
        this.binFilePath = binFilePath;
        mRenderer = new HDRRenderer();

        //total coefficients is 16 * 16 * 8 * 4 * 3
        gridDepth = 8;
        gridHeight = 16;
        gridWidth = 16;
        mgr = assetManager;
    }

    public void hdrProcess(final String srcImagePath,final int srcWidth,final int srcHeight, final int type){
        //put the image to the Graph and get the output as the input of opengl engine, then calculate the render
        //scale to netInputSize * netInputSize

        String assestPath = "file:///android_asset/";
        String pdName;
        switch (type) {
            case 1:
                pdName = "hdrp_optimized_graph.pb";
                break;
            case 2:
                pdName = "normal_optimized_graph.pb";
                break;
            case 3:
                pdName = "faces_optimized_graph.pb";
                break;
            case 4:
                pdName = "earlyBird_optimized_graph.pb";
                break;
            default:
                Log.e("hdrProcess","Please assign the model type");
                pdName = "";
        }
        long loadStartTime = System.currentTimeMillis();
        tfi = new TensorFlowInferenceInterface(mgr, assestPath + pdName);
        long loadEndTime = System.currentTimeMillis();
        Log.e("Tensorflow ", "*************************************************the tensorflow load cast "+ (loadEndTime - loadStartTime) + " ms");

        float[] smallArray = mRenderer.getSmallImage(srcImagePath,type);   //resize to 256*256 BGRA(or YUVA) image

        Log.i("hdrProcess", "the srcWidth of srcBitmap is "+ srcWidth);
        Log.i("hdrProcess", "the srcHeight of srcBitmap is "+ srcHeight);

        //循环迭代的时候 outBuffer会出现线程同步的问题，如果渲染线程足够快，则可以保证每次渲染可以取到对应的outBUffer，但是渲染线程如果慢了(例如增加了消息
        //传递来通知UI线程更新界面）则可能取不到正确的outBuffer
        long t1, t2;
        //long tensorStartTime = System.currentTimeMillis();
        t1 = System.currentTimeMillis();
        tfi.feed(inputName, smallArray, 1, netInputSize, netInputSize, 3);
        tfi.run(new String[]{outputName});
        final float[] outBuffer = new float[gridDepth * gridHeight * gridWidth * 4 * 3];
        tfi.fetch(outputName, outBuffer);           //coeffs_data
        t2 = System.currentTimeMillis();
        //long tensorEndTime = System.currentTimeMillis();
        //tensorTotalTime += tensorEndTime - tensorStartTime;
        tensorTotalTime += t2 - t1;
        //Log.e("Tensorflow forward", "*************************************************the tensorflow process cast "+ (tensorEndTime - tensorStartTime) + " ms");
        Log.e("Tensorflow forward", "*************************************************the tensorflow process cast "+ (t2 - t1) + " ms");
        Log.e("Tensorflow forward", "*************************************************the tensorflow tensorTotalTime is "+ tensorTotalTime + " ms");
        tfi.close();

            //openGL
        this.queueEvent(new Runnable() {
            @Override
            public void run() {
                long initStartTime = System.currentTimeMillis();
                mRenderer.init(srcWidth, srcHeight, gridWidth, gridHeight, gridDepth, binFilePath, type);
                long initEndTime = System.currentTimeMillis();
                Log.e("OpenGL Thread android", "*************************************************the OpenGL init cast "+ (initEndTime - initStartTime) + " ms");

                long upLoadImgStartTime = System.currentTimeMillis();
                mRenderer.upLoadImg(srcImagePath, type);
                long upLoadImgEndTime = System.currentTimeMillis();
                Log.e("OpenGL Thread android", "*************************************************the OpenGL upLoadImg cast "+ (upLoadImgEndTime - upLoadImgStartTime) + " ms");

                long renderStartTime = System.currentTimeMillis();
                resultArray = mRenderer.render(outBuffer, srcWidth, srcHeight);
                long renderEndTime = System.currentTimeMillis();
                Log.e("OpenGL Thread android", "*************************************************the render cast "+ (renderEndTime - renderStartTime) + " ms");

                mRenderer.freeCurrentState();

                //多次迭代测试时，注释下段代码(即不发送消息更新UI，避免线程同步异常)

                //update the imageView
                Bitmap resultImg = Bitmap.createBitmap(srcWidth, srcHeight, Bitmap.Config.ARGB_8888);
                resultImg.setPixels(resultArray, 0, srcWidth, 0, 0, srcWidth, srcHeight);
                Message message = new Message();
                message.what = 1;   //UPDATE_IMAGE
                message.obj = resultImg;
                hander.sendMessage(message);
            }
        });


    }

    public void clearTime(){

        tensorTotalTime  = 0d;
        this.queueEvent(new Runnable() {
            @Override
            public void run() {
                mRenderer.clearTime();
            }
        });
    }
}
