package com.example.hdrimg2;

import android.Manifest;
import android.app.ActivityManager;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ConfigurationInfo;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.opengl.GLSurfaceView;
import android.os.Message;
import android.provider.MediaStore;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.CursorLoader;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.LinearLayout.LayoutParams;
import android.os.Handler;

import java.io.FileNotFoundException;
import java.util.logging.LogRecord;


public class MainActivity extends AppCompatActivity {
    private final int CONTEXT_CLIENT_VERSION = 3;
    private static final int SELECT_IMAGE = 1;
    private static final int UPDATE_IMAGE = 1;
    private ImageView imageView;
    private Button selectButton;
    private Button hdrButton;
    private Button laplacianButton;
    private Button facesButton;
    private Button earlyBirdButton;
    private Bitmap yourSelectedImage = null;
    private String imagePath = "";
    private AssetManager mgr;
    private int iter = 1;

    private HDRGLSurfaceView hdrglSurfaceView;
    private Handler handerUI = new Handler(){
        @Override
        public void handleMessage(Message msg) {
            switch(msg.what){
                case UPDATE_IMAGE:
                    imageView.setImageBitmap((Bitmap) msg.obj);
                    //imageView.setImageBitmap(null);
                    //hdrglSurfaceView.requestRender();
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
        mgr = getResources().getAssets();
        this.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
        String binFilePath = copyBinFiles();

        hdrglSurfaceView = new HDRGLSurfaceView(this, mgr, binFilePath, handerUI);

        if (detectOpenGLES30()) {
            hdrglSurfaceView.setEGLContextClientVersion(CONTEXT_CLIENT_VERSION);
            hdrglSurfaceView.setRenderer(hdrglSurfaceView.mRenderer);
        } else {
            Log.e("opengles30", "OpenGL ES 3.0 not supported on device.  Exiting...");
            finish();
        }
        //只有在数据改变时才进行Render，这样来提高性能
        hdrglSurfaceView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
        setContentView(hdrglSurfaceView);

        if(ActivityCompat.checkSelfPermission(this, Manifest.permission.MOUNT_UNMOUNT_FILESYSTEMS) != PackageManager.PERMISSION_GRANTED &&
                ActivityCompat.checkSelfPermission(this, Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED &&
                ActivityCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED ){
            ActivityCompat.requestPermissions(MainActivity.this, new String[]{Manifest.permission.MOUNT_UNMOUNT_FILESYSTEMS, Manifest.permission.READ_EXTERNAL_STORAGE, Manifest.permission.WRITE_EXTERNAL_STORAGE}, 1);
        }

        createLayout();

        selectButton.setOnClickListener(new Button.OnClickListener() {
            @Override
            //select image
            public void onClick(View v) {
                Intent i = new Intent(Intent.ACTION_PICK);
                i.setType("image/*");
                startActivityForResult(i, SELECT_IMAGE);
            }
        });

        hdrButton.setOnClickListener(new Button.OnClickListener(){

            public void onClick(View v){
                //do something
                if (yourSelectedImage == null || imagePath == "")
                    return;
                hdrglSurfaceView.clearTime();
                for(int i = 0; i < iter; i++)
                {
                    hdrglSurfaceView.hdrProcess(imagePath, yourSelectedImage.getWidth(), yourSelectedImage.getHeight(), 1);
                }

            }
        });

        laplacianButton.setOnClickListener(new Button.OnClickListener(){
            public void onClick(View v){
                //do something
                if (yourSelectedImage == null || imagePath == "")
                    return;
                hdrglSurfaceView.clearTime();
                for(int i = 0; i < iter; i++) {
                    hdrglSurfaceView.hdrProcess(imagePath, yourSelectedImage.getWidth(), yourSelectedImage.getHeight(), 2);
                }
            }
        });

        facesButton.setOnClickListener(new Button.OnClickListener(){
            public void onClick(View v){
                //do something
                if (yourSelectedImage == null || imagePath == "")
                    return;
                hdrglSurfaceView.clearTime();
                for(int i = 0; i < iter; i++) {
                    hdrglSurfaceView.hdrProcess(imagePath, yourSelectedImage.getWidth(), yourSelectedImage.getHeight(), 3);
                }
            }
        });

        earlyBirdButton.setOnClickListener(new Button.OnClickListener(){
            public void onClick(View v){
                //do something
                if (yourSelectedImage == null || imagePath == "")
                    return;
                hdrglSurfaceView.clearTime();
                for(int i = 0; i < iter; i++) {
                    hdrglSurfaceView.hdrProcess(imagePath, yourSelectedImage.getWidth(), yourSelectedImage.getHeight(), 4);
                }
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
        horLayout.setLayoutParams(new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT));

        selectButton = new Button(this);
        selectButton.setLayoutParams(new LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT));
        selectButton.setText("选图");
        horLayout.addView(selectButton);

        hdrButton = new Button(this);
        hdrButton.setLayoutParams(new LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT));
        hdrButton.setText("hdrp");
        horLayout.addView(hdrButton);

        laplacianButton = new Button(this);
        laplacianButton.setLayoutParams(new LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT));
        laplacianButton.setText("Laplacian");
        horLayout.addView(laplacianButton);

        facesButton = new Button(this);
        facesButton.setLayoutParams(new LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT));
        facesButton.setText("faces");
        horLayout.addView(facesButton);
        verLayout.addView(horLayout);

        LinearLayout horLayout2 = new LinearLayout(this);
        horLayout2.setOrientation(LinearLayout.HORIZONTAL);
        horLayout2.setLayoutParams(new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT));

        earlyBirdButton = new Button(this);
        earlyBirdButton.setLayoutParams(new LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT));
        earlyBirdButton.setText("earlyBird");
        horLayout2.addView(earlyBirdButton);
        verLayout.addView(horLayout2);


        imageView = new ImageView(this);
        imageView.setLayoutParams(new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT));
        verLayout.addView(imageView);

        addContentView(verLayout, new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT));
    }

    protected void onActivityResult(int requestCode, int resultCode, Intent data)
    {
        super.onActivityResult(requestCode, resultCode, data);

        if (resultCode == RESULT_OK && null != data) {
            Uri selectedImage = data.getData();
            if (requestCode == SELECT_IMAGE) {
                imagePath = getRealPathFromURI(selectedImage);
                yourSelectedImage = BitmapFactory.decodeFile(imagePath);
                imageView.setImageBitmap(yourSelectedImage);  //显示部分
            }
        }
    }

    private Bitmap decodeUri(Uri selectedImage) throws FileNotFoundException
    {
        // Decode image size
        BitmapFactory.Options o = new BitmapFactory.Options();
        o.inJustDecodeBounds = true;
        BitmapFactory.decodeStream(getContentResolver().openInputStream(selectedImage), null, o);

        // The new size we want to scale to
        final int REQUIRED_SIZE = 400;

        // Find the correct scale value. It should be the power of 2.
        int width_tmp = o.outWidth, height_tmp = o.outHeight;
        int scale = 1;
        while (true) {
            if (width_tmp / 2 < REQUIRED_SIZE
                    || height_tmp / 2 < REQUIRED_SIZE) {
                break;
            }
            width_tmp /= 2;
            height_tmp /= 2;
            scale *= 2;
        }

        // Decode with inSampleSize
        BitmapFactory.Options o2 = new BitmapFactory.Options();
        o2.inSampleSize = scale;
        return BitmapFactory.decodeStream(getContentResolver().openInputStream(selectedImage), null, o2);
    }

    private String getRealPathFromURI(Uri contentUri) { //传入图片uri地址
        String[] proj = { MediaStore.Images.Media.DATA };
        CursorLoader loader = new CursorLoader(this, contentUri, proj, null, null, null);
        Cursor cursor = loader.loadInBackground();
        int column_index = cursor.getColumnIndexOrThrow(MediaStore.Images.Media.DATA);
        cursor.moveToFirst();
        return cursor.getString(column_index);
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
