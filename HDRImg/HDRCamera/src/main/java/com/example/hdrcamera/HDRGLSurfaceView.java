package com.example.hdrcamera;

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.ImageFormat;
import android.graphics.SurfaceTexture;
import android.graphics.drawable.GradientDrawable;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CameraMetadata;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.media.Image;
import android.media.ImageReader;
import android.opengl.GLSurfaceView;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v7.widget.LinearLayoutCompat;
import android.util.Log;
import android.util.Size;
import android.util.SparseIntArray;
import android.widget.Toast;
import android.view.Surface;


import org.tensorflow.contrib.android.TensorFlowInferenceInterface;

import java.nio.ByteBuffer;
import java.util.Arrays;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class HDRGLSurfaceView extends GLSurfaceView implements GLSurfaceView.Renderer {
    private static final int OPENCAMERA = 1;
    private static final int UPDATEVIEW = 2;

    static {
        System.loadLibrary("renderJni");
        System.loadLibrary("tensorflow_inference");
    }

    private boolean processing = false;
    private String cameraLock = new String("cameraLock");
    private String renderLock = new String("renderLock");
    private TensorFlowInferenceInterface tfi;
    private static String  inputName = "lowres_input";
    private static String  outputName = "output_coefficients";
    private static int netInputSize = 256;      //lowres_input size
    private String binFilePath;

    long startTime;
    long endTime;
    String timeInformation = "";

    private Handler hander;
    private AssetManager mgr;
    private Context context;

    private String cameraId;
    private int cameraIndex = 0;    //0: 后置   1: 前置
    protected CameraDevice cameraDevice;
    protected CameraCaptureSession cameraCaptureSessions;
    protected CaptureRequest.Builder captureRequestBuilder;
    private Size imageDimension;
    private Handler mBackgroundHandler;
    private HandlerThread mBackgroundThread;
    private Image image = null;
    private byte[][] yuvBytes;
    private int[] rgbBytes = null;

    private int gridDepth;
    private int gridHeight;
    private int gridWidth;

    private int previewWidth;
    private int previewHeight;
    private int modelType = 2;

    public native void init(int imageWidth, int imageHeight, int gridWidth, int gridHeight, int gridDepth, String binFilePath, int cameraIndex, int modelType);
    //upload src image(BGRA)
    public native void upLoadImg(int[] rgbBytes, int width, int height, int type);
    //coeffsData: calculate by tensorflow
    //return int array, every element's foramt is BGRA
    public native void render(float[] coeffsData, int srcWidth, int srcHeight);
    //when calculate a image, you must release the state of the Render in order to deal another iamge
    public native void freeCurrentState();
    public native float[] getSmallImage(int[] rgbBytes, int width, int height, int cameraIndex, int type);
    public native void glesResize(int width, int height);

    public HDRGLSurfaceView(Context context, AssetManager assetManager, String binFilePath, Handler handerUI){
        super(context);
        this.context = context;
        this.binFilePath = binFilePath;
        hander = handerUI;

        //total coefficients is 16 * 16 * 8 * 4 * 3
        gridDepth = 8;
        gridHeight = 16;
        gridWidth = 16;
        mgr = assetManager;

        yuvBytes = new byte[3][];
        initTensorflow();
    }

    public void initTensorflow(){
        String assestPath = "file:///android_asset/";
        String pdName;
        switch (modelType) {
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
        tfi = new TensorFlowInferenceInterface(mgr, assestPath + pdName);
    }

    public void releaseTensorflow(){
        tfi.close();
    }

    public void initOpenGL(){
        this.queueEvent(new Runnable() {
            @Override
            public void run() {
                init(previewWidth, previewHeight, gridWidth, gridHeight, gridDepth, binFilePath, cameraIndex, modelType);
                glesResize(previewHeight, previewWidth);  //宽高与图片尺寸相反
                //glesResize(previewWidth/2, previewHeight/2);
                //glesResize(1080, 1920);
            }
        });
    }

    public void releaseOpenGL(){
        this.queueEvent(new Runnable() {
            @Override
            public void run() {
                freeCurrentState();
            }
        });
    }

    public void changeCamera()
    {
        closeCamera();
        stopBackgroundThread();
        synchronized (cameraLock) {
            synchronized (renderLock){
                releaseOpenGL();
                if(cameraIndex == 0)
                    cameraIndex = 1;
                else
                    cameraIndex = 0;
                startBackgroundThread();
                openCamera();
            }
        }
        return;
    }

    public void changeModel(int type){
        synchronized (cameraLock) {
            synchronized (renderLock){
                modelType = type;
                releaseTensorflow();
                initTensorflow();
                releaseOpenGL();
                initOpenGL();
            }
        }
    }

    public void hdrProcess(final int[] rgbBytes){
        //put the image to the Graph and get the output as the input of opengl engine, then calculate the render
        //scale to netInputSize * netInputSize
        startTime = System.currentTimeMillis();
        float[] smallArray = getSmallImage(rgbBytes, previewWidth, previewHeight, cameraIndex, modelType);   //resize to 256*256 BGR(or YUV) image
        endTime = System.currentTimeMillis();
        timeInformation = timeInformation + "resize image: " + (endTime - startTime) + " ms\n";

        startTime = System.currentTimeMillis();
        tfi.feed(inputName, smallArray, 1, netInputSize, netInputSize, 3);
        tfi.run(new String[]{outputName});
        final float[] outBuffer = new float[gridDepth * gridHeight * gridWidth * 4 * 3];
        tfi.fetch(outputName, outBuffer);           //coeffs_data
        endTime = System.currentTimeMillis();
        timeInformation = timeInformation + "tensorflow forward: " + (endTime - startTime) + " ms\n";

        //openGL
        this.queueEvent(new Runnable() {
            @Override
            public void run() {
                synchronized (renderLock){
                    startTime = System.currentTimeMillis();
                    upLoadImg(rgbBytes, previewWidth, previewHeight, modelType);
                    render(outBuffer, previewWidth, previewHeight);
                    endTime = System.currentTimeMillis();
                    timeInformation = timeInformation + "openGL uploadImg and render: " + (endTime - startTime) + " ms";

                    //Log.e("timeInformation: ", timeInformation);
                    Message message = new Message();
                    message.what = UPDATEVIEW;
                    message.obj = timeInformation;
                    hander.sendMessage(message);
                    processing = false;
                }
            }
        });
    }

    private Size selectBestResolution(StreamConfigurationMap map)
    {
        int length = map.getOutputSizes(SurfaceTexture.class).length;
        for(int i = 0; i < length; i++)
        {
            if(map.getOutputSizes(SurfaceTexture.class)[i].getWidth() > 1920)
                continue;
            else
                return map.getOutputSizes(SurfaceTexture.class)[i];
        }
        return map.getOutputSizes(SurfaceTexture.class)[0];
    }

    public void openCamera() {
        CameraManager manager = (CameraManager)(context.getSystemService(Context.CAMERA_SERVICE));
        try {
            cameraId = manager.getCameraIdList()[cameraIndex];
            CameraCharacteristics characteristics = manager.getCameraCharacteristics(cameraId);
            StreamConfigurationMap map = characteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
            assert map != null;
            //imageDimension = map.getOutputSizes(SurfaceTexture.class)[2];
            imageDimension = selectBestResolution(map);
            previewWidth = imageDimension.getWidth();
            previewHeight = imageDimension.getHeight();
            /*
            huawei nove2  后置摄像头
            0: width: 3968, height; 2976
            1: width: 2240, height; 1680
            2: width: 1920, height; 1080
            3: width: 1440, height; 1080
            4: width: 1280, height; 960
            5: width: 1280, height; 720
            6: width: 960, height; 720
            7: width: 960, height; 544
            8: width: 720, height; 720
            9: width: 640, height; 480
            10: width: 352, height; 288
            11: width: 320, height; 240
            12: width: 208, height; 144
            13: width: 176, height; 144
            */
            //for(int i = 0; i < map.getOutputSizes(SurfaceTexture.class).length; i++)
            //{
                //输出所有支持的预览尺寸
            //    Log.e("resolution", i + ": " + "width: " + map.getOutputSizes(SurfaceTexture.class)[i].getWidth() + ", height; " + map.getOutputSizes(SurfaceTexture.class)[i].getHeight());
            //}
            Log.i("openCamera", "the best resolution: width: " + previewWidth + " height: " + previewHeight);
            initOpenGL();
            manager.openCamera(cameraId, stateCallback, null);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    private final CameraDevice.StateCallback stateCallback = new CameraDevice.StateCallback() {
        @Override
        public void onOpened(CameraDevice camera) {
            cameraDevice = camera;
            createCameraPreview();
        }
        @Override
        public void onDisconnected(CameraDevice camera) {
            cameraDevice.close();
        }
        @Override
        public void onError(CameraDevice camera, int error) {
            cameraDevice.close();
            cameraDevice = null;
        }
    };

    protected void createCameraPreview() {
        try {
            ImageReader reader = ImageReader.newInstance(previewWidth, previewHeight, ImageFormat.YUV_420_888, 4);
            //ImageReader reader = ImageReader.newInstance(previewWidth, previewHeight, ImageFormat.FLEX_RGB_888, 4);
            ImageReader.OnImageAvailableListener readerListener = new ImageReader.OnImageAvailableListener() {
                @Override
                public void onImageAvailable(ImageReader reader) {
                    try {
                        image = reader.acquireNextImage();
                        if (processing) {
                            image.close();         //如果正在处理，丢弃该帧
                            return;
                        }
                        synchronized (cameraLock) {
                            processing = true;
                            timeInformation = "";
                            startTime = System.currentTimeMillis();
                            final Image.Plane[] planes = image.getPlanes();
                            fillBytes(planes, yuvBytes);

                            final int yRowStride = planes[0].getRowStride();
                            final int uvRowStride = planes[1].getRowStride();
                            final int uvPixelStride = planes[1].getPixelStride();
                            rgbBytes = new int[previewWidth * previewHeight];
                            ImageUtils.convertYUV420ToARGB8888(
                                    yuvBytes[0],
                                    yuvBytes[1],
                                    yuvBytes[2],
                                    rgbBytes,
                                    previewWidth,
                                    previewHeight,
                                    yRowStride,
                                    uvRowStride,
                                    uvPixelStride,
                                    false);

                            endTime = System.currentTimeMillis();
                            timeInformation = timeInformation + "YUV2RGB: " + (endTime - startTime) + " ms\n";
                            hdrProcess(rgbBytes);
                        }

                    } finally {
                        if (image != null) {
                            image.close();
                        }
                    }
                }
            };
            reader.setOnImageAvailableListener(readerListener, mBackgroundHandler);
            captureRequestBuilder = cameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
            captureRequestBuilder.addTarget(reader.getSurface());

            cameraDevice.createCaptureSession(Arrays.asList(reader.getSurface()), new CameraCaptureSession.StateCallback(){
                @Override
                public void onConfigured(@NonNull CameraCaptureSession cameraCaptureSession) {
                    if (null == cameraDevice) {
                        return;
                    }
                    cameraCaptureSessions = cameraCaptureSession;
                    updatePreview();
                }
                @Override
                public void onConfigureFailed(@NonNull CameraCaptureSession cameraCaptureSession) {
                    Toast.makeText(context, "Configuration change", Toast.LENGTH_SHORT).show();
                }
            }, null);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    protected void updatePreview() {
        if(null == cameraDevice) {
            return;
        }
        captureRequestBuilder.set(CaptureRequest.CONTROL_MODE, CameraMetadata.CONTROL_MODE_AUTO);
        try {
            cameraCaptureSessions.setRepeatingRequest(captureRequestBuilder.build(), null, mBackgroundHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    public void closeCamera() {
        if (null != cameraDevice) {
            cameraDevice.close();
            cameraDevice = null;
        }
    }

    protected void startBackgroundThread() {
        mBackgroundThread = new HandlerThread("Camera Background");
        mBackgroundThread.start();
        mBackgroundHandler = new Handler(mBackgroundThread.getLooper());
    }
    protected void stopBackgroundThread() {

        mBackgroundThread.quitSafely();

        try {
            Log.e("stopBackgroundThread","mBackgroundThread.join()  begin");
            mBackgroundThread.join();
            Log.e("stopBackgroundThread","mBackgroundThread.join()  end");
            mBackgroundThread = null;
            mBackgroundHandler = null;
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        Message message = new Message();
        message.what = OPENCAMERA;
        hander.sendMessage(message);
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
    }
    @Override
    public void onDrawFrame(GL10 gl) {
    }

    @Override
    public void onResume() {
        super.onResume();
        startBackgroundThread();
    }

    @Override
    public void onPause() {
        super.onPause();
        closeCamera();
        stopBackgroundThread();
        releaseOpenGL();
    }

    protected void fillBytes(final Image.Plane[] planes, final byte[][] yuvBytes) {
        // Because of the variable row stride it's not possible to know in
        // advance the actual necessary dimensions of the yuv planes.
        for (int i = 0; i < planes.length; ++i) {
            final ByteBuffer buffer = planes[i].getBuffer();
            if (yuvBytes[i] == null) {
                yuvBytes[i] = new byte[buffer.capacity()];
            }
            buffer.get(yuvBytes[i]);
        }
    }

    public void setModelType(int type){
        modelType = type;
    }

    public int getModelType(){
        return modelType;
    }

    public String getTimeInformation(){ return timeInformation; }
}
