package frankandrobot.glwallpapervideodemo.com;

import net.rbgrn.android.glwallpaperservice.*;
import android.util.Log;
import android.widget.Toast;
import java.lang.reflect.Field;
import java.io.IOException;
import java.util.Arrays;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.SystemClock;
import android.service.wallpaper.WallpaperService;
import android.util.Log;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.app.WallpaperManager;
import android.util.TypedValue;
import android.content.res.Resources;
import android.os.SystemClock;
/* for loading file */
import android.content.res.AssetManager;
import java.io.BufferedInputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.FileOutputStream;
import android.graphics.RectF;
/* for threads */
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantReadWriteLock;
import java.lang.Math; 
import android.view.WindowManager;
import android.view.Display;
/* communicating with renderer */
import android.os.Handler;
import android.os.Message;
/* math */
import java.lang.Math;
/* opengl */
import android.opengl.GLSurfaceView;

public class GLWallpaperVideoDemo extends GLWallpaperService {
    public static final String folder = "video";
    public static final String TAG = "GLWVD";
    public static String videoName="VIDEOWALL102344df@#%";
    //video variables
    public int videoWidth,videoHeight;
    public boolean videoWideScreen=false;
    
    public GLWallpaperVideoDemo() { 
	super(); 
	Log.d(TAG,"constructor()");	
    }

    @Override
    public void onCreate() {
	Log.d(TAG,"onCreate()");
        super.onCreate();
	//transfer video to sdcard
	Log.d(TAG,"transferring video asset to sdcard");
	copyVideoToCard();
	Log.d(TAG,"transferred");
	//if videoName == blankVideo, then don't load anything
	//TODO
	NativeCalls.initVideo();
	Log.d(TAG,"Opening video");
	NativeCalls.loadVideo("file:/"+"sdcard/"
			      +GLWallpaperVideoDemo.videoName);
	//set video dimensions (now that we opened the video)
	videoWidth = NativeCalls.getVideoWidth();
	videoHeight = NativeCalls.getVideoHeight();
	videoWideScreen = ( videoWidth > videoHeight ) ? true : false;
    }

    private void copyVideoToCard() {
	//open file in assets
	AssetManager assetManager = getAssets();
	String[] videoAsset = null;
	try {
	    videoAsset = assetManager.list(folder);
	} catch (IOException e) {
	    Log.e(TAG, e.getMessage());
	}
	InputStream in = null;
	OutputStream out = null;
	try {
	    in = assetManager.open(folder+"/"+videoAsset[0]);
	    out = new FileOutputStream("/sdcard/" + videoName);
	    //actually copy over file
	    byte[] buffer = new byte[1024];
	    int read;
	    while((read = in.read(buffer)) != -1){
		out.write(buffer, 0, read);
	    }
	    //close up everything
	    in.close();
	    in = null;
	    out.flush();
	    out.close();
	    out = null;
	    //assetManager.close();
	} catch(Exception e) {
	    Log.e(TAG, e.getMessage());
	}
    }

    @Override
    public void onDestroy() {
	Log.d(TAG,"onDestroy()");
        super.onDestroy();
	NativeCalls.closeVideo();
    }

    private VideoEngine mEngine=null;

    @Override
    public Engine onCreateEngine() {
	Log.d(TAG,"onCreateEngine()");
        mEngine = new VideoEngine();
	return mEngine;
    }

    class VideoEngine extends GLEngine {

	VideoRenderer renderer = null;

	VideoEngine() { 
	    super();
	    Log.d(TAG,"VideoEngine VideoEngine()");
	    renderer = new VideoRenderer(GLWallpaperVideoDemo.this, 
					 this);
	    setRenderer(renderer);
	    //setRenderMode(RENDERMODE_WHEN_DIRTY);
	    setRenderMode(RENDERMODE_CONTINUOUSLY);
	}

	VideoRenderer getRenderer() { return renderer; }

        @Override
        public void onCreate(SurfaceHolder surfaceHolder) {
            Log.d(TAG,"VideoEngine onCreate()");
	    super.onCreate(surfaceHolder);
        }

        @Override
        public void onDestroy() {
            Log.d(TAG,"VideoEngine onDestroy()");
	    super.onDestroy();
	    if (renderer != null) {
		renderer.release();
	    }
	    renderer = null;
        }

        @Override
        public void onSurfaceChanged(SurfaceHolder holder, int format, 
				     int w, int h) {
	    Log.d(TAG, "VideoEngine onSurfaceChanged()");
            super.onSurfaceChanged(holder, format, w, h);
	}

        @Override
        public void onSurfaceCreated(SurfaceHolder holder) {
	    Log.d(TAG,"VideoEngine onSurfaceCreated()");
            super.onSurfaceCreated(holder);
        }

        @Override
        public void onSurfaceDestroyed(SurfaceHolder holder) {
	    Log.d(TAG,"VideoEngine onSurfaceDestroyed");
            super.onSurfaceDestroyed(holder);
	}

	@Override
	public void onVisibilityChanged(boolean visible) {
	    Log.d(TAG,"VideoEngine onVisibilityChanged()");
	    super.onVisibilityChanged(visible);
	}
    }
} 