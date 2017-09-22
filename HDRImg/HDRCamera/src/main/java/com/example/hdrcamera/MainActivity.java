package com.example.hdrcamera;

import android.Manifest;
import android.app.ActivityManager;
import android.content.Context;
import android.content.pm.ConfigurationInfo;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.graphics.Color;
import android.opengl.GLSurfaceView;
import android.os.Handler;
import android.os.Message;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {

    private static final int OPENCAMERA = 1;
    private static final int UPDATEVIEW = 2;
    private final int CONTEXT_CLIENT_VERSION = 3;
    private Button hdrButton;
    private Button laplacianButton;
    private Button facesButton;
    private Button earlyBirdButton;
    private Button changeCameraButton;
    private TextView tv;
    private AssetManager mgr;
    private HDRGLSurfaceView hdrglSurfaceView;

    private Handler handerUI = new Handler(){
        @Override
        public void handleMessage(Message msg) {
            switch(msg.what){
                case OPENCAMERA:
                    hdrglSurfaceView.openCamera();
                    break;
                case UPDATEVIEW:
                    hdrglSurfaceView.requestRender();
                    tv.setText((String)msg.obj);
                    break;
                default:
                    break;
            }
        }
    };

    private String copyBinFiles()
    {
        String appFilePath = getExternalFilesDir(null).getAbsolutePath();      // /storage/emulated/0/Android/data/com.example.hdrcamera/files
        String binFilePath = "hdrmodel";
        String root = appFilePath + "/" + binFilePath;
        if(!AssetCopyer.isExits(root))
        {
            AssetCopyer.copyAllAssets(this, binFilePath, root);
        }
        else
            Log.i("copyBinFiles", "the bin files have been there");
        return root;
    }


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);

        String binFilePath = copyBinFiles();

        mgr = getResources().getAssets();
        hdrglSurfaceView = new HDRGLSurfaceView(this, mgr, binFilePath, handerUI);

        if (detectOpenGLES30()) {
            hdrglSurfaceView.setEGLContextClientVersion(CONTEXT_CLIENT_VERSION);
            hdrglSurfaceView.setRenderer(hdrglSurfaceView);
        } else {
            Log.e("opengles30", "OpenGL ES 3.0 not supported on device.  Exiting...");
            finish();
        }
        //只有在数据改变时才进行Render，这样来提高性能
        hdrglSurfaceView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
        setContentView(hdrglSurfaceView);

        if(ActivityCompat.checkSelfPermission(this, Manifest.permission.MOUNT_UNMOUNT_FILESYSTEMS) != PackageManager.PERMISSION_GRANTED &&
                ActivityCompat.checkSelfPermission(this, Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED &&
                ActivityCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED &&
                ActivityCompat.checkSelfPermission(this, Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED){
            ActivityCompat.requestPermissions(MainActivity.this, new String[]{Manifest.permission.MOUNT_UNMOUNT_FILESYSTEMS,
                    Manifest.permission.READ_EXTERNAL_STORAGE,
                    Manifest.permission.WRITE_EXTERNAL_STORAGE,
                    Manifest.permission.CAMERA}, 1);
        }
        createLayout();
        setListener();

    }

    private void setListener(){
        hdrButton.setOnClickListener(new Button.OnClickListener(){
            public void onClick(View v){
                hdrglSurfaceView.changeModel(1);
            }
        });
        laplacianButton.setOnClickListener(new Button.OnClickListener(){
            public void onClick(View v){hdrglSurfaceView.changeModel(2);
            }
        });
        facesButton.setOnClickListener(new Button.OnClickListener(){
            public void onClick(View v){
                hdrglSurfaceView.changeModel(3);
            }
        });
        earlyBirdButton.setOnClickListener(new Button.OnClickListener(){
            public void onClick(View v){hdrglSurfaceView.changeModel(4);
            }
        });

        changeCameraButton.setOnClickListener(new Button.OnClickListener(){
            public void onClick(View v){
                hdrglSurfaceView.changeCamera();
            }
        });

    }

    private void createLayout(){
        //垂直
        LinearLayout verLayout = new LinearLayout(this);
        verLayout.setOrientation(LinearLayout.VERTICAL);
        //verLayout.setLayoutParams(new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT));

        //水平
        LinearLayout horLayout = new LinearLayout(this);
        horLayout.setOrientation(LinearLayout.HORIZONTAL);
        horLayout.setLayoutParams(new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT));

        hdrButton = new Button(this);
        hdrButton.setLayoutParams(new LinearLayout.LayoutParams(LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT));
        hdrButton.setText("hdrp");
        hdrButton.setWidth(10);

        horLayout.addView(hdrButton);

        laplacianButton = new Button(this);
        laplacianButton.setLayoutParams(new LinearLayout.LayoutParams(LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT));
        laplacianButton.setText("Laplacian");
        horLayout.addView(laplacianButton);

        facesButton = new Button(this);
        facesButton.setLayoutParams(new LinearLayout.LayoutParams(LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT));
        facesButton.setText("faces");
        horLayout.addView(facesButton);

        earlyBirdButton = new Button(this);
        earlyBirdButton.setLayoutParams(new LinearLayout.LayoutParams(LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT));
        earlyBirdButton.setText("earlyBird");
        horLayout.addView(earlyBirdButton);
        verLayout.addView(horLayout);

        changeCameraButton = new Button(this);
        changeCameraButton.setLayoutParams(new LinearLayout.LayoutParams(LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT));
        changeCameraButton.setText("切换");
        verLayout.addView(changeCameraButton);

        LinearLayout horLayout2 = new LinearLayout(this);
        horLayout2.setOrientation(LinearLayout.HORIZONTAL);
        horLayout2.setLayoutParams(new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT));

        tv = new TextView(this);
        tv.setLayoutParams(new LinearLayout.LayoutParams(LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT));
        tv.setTextColor(Color.rgb(255,0,0));
        tv.setTextSize(20.0f);
        horLayout2.addView(tv);

        verLayout.addView(horLayout2);
        addContentView(verLayout, new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.MATCH_PARENT));
    }

    private boolean detectOpenGLES30() {
        ActivityManager am = (ActivityManager)getSystemService(Context.ACTIVITY_SERVICE);
        ConfigurationInfo info = am.getDeviceConfigurationInfo();

        return (info.reqGlEsVersion >= 0x30000);
    }

    protected void onPause(){
        super.onPause();
        hdrglSurfaceView.onPause();
    }

    protected void onResume(){
        super.onResume();
        hdrglSurfaceView.onResume();
    }
}
