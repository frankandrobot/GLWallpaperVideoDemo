package frankandrobot.glwallpapervideodemo.com;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.view.SurfaceHolder;
import java.io.File;
import java.io.FileInputStream;
import android.graphics.Canvas;
import android.graphics.drawable.Drawable;
import net.rbgrn.android.glwallpaperservice.*;
import android.opengl.GLU;
import java.nio.FloatBuffer;
import java.nio.Buffer;
import java.io.BufferedInputStream;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.opengl.GLUtils;
import java.io.IOException;
import java.nio.ShortBuffer;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import android.os.SystemClock;
import android.util.Log;
/* threads */
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import java.lang.Thread;
import java.lang.InterruptedException;
/* screen dimensions */
import android.view.Display;
import android.content.Context;
import android.view.WindowManager;

// Original code provided by Robert Green
// http://www.rbgrn.net/content/354-glsurfaceview-adapted-3d-live-wallpapers
public class VideoRenderer implements GLWallpaperService.Renderer {
    static private String TAG="Renderer>>>>>>>>>>>>";
    static boolean runOnce = false;
    //screen variables
    int screenWidth=50,screenHeight=50;
    int drawWidth, drawHeight; //dimensions of fit-to-screen video
    int paddingX, paddingY; //padding for fit-to-screen-video
    //texture variables
    int powWidth,powHeight;
    //pointers
    GLWallpaperVideoDemo mParent;
    GLWallpaperVideoDemo.VideoEngine mParentEngine;
    //lock
    static public Object lock = new Object();

    public VideoRenderer() { 
	super();
	Log.d(TAG,"Constructor()");
    }

    public VideoRenderer(GLWallpaperVideoDemo p, 
			   GLWallpaperVideoDemo.VideoEngine e) {
	super();
	mParent = p;
	mParentEngine = e;
	Log.d(TAG,"constructor()");
    }

    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
	Log.d(TAG, "onSurfaceCreated()");
    }

    void process(int width, int height) {
	setScreenDimensions( width, height );
	Log.d(TAG,"Killing texture");
	NativeCalls.closeOpenGL();
	setTextureDimensions();
	setFitToScreenDimensions( videoWidth, videoHeight );
	if ( !runOnce ) {
	    Log.d(TAG,"Preparing frame");
	    NativeCalls.prepareStorageFrame();
	}
	NativeCalls.initOpenGL();
	runOnce = true;
    }

    //This gets called whenever you preview the wallpaper or set the
    //wallpaper
    public void onSurfaceChanged(GL10 gl, int width, int height) {
	Log.d(TAG,"onSurfaceChanged()");
	 synchronized(lock) {
	     process(width, height);
	 }
    }
    
    public void onDrawFrame(GL10 gl) {
	synchronized(lock) {
	    //Log.d(TAG,"Drawing ....");
	    NativeCalls.getFrame(); // from video
	    NativeCalls.drawFrame(); // using openGL
	    if (MyDebug.showFPS) {
		final float fpsRate;
		    fpsRate = 1000f/((float) (SystemClock.uptimeMillis() 
					      - fpsTime) );
		    fpsTime = SystemClock.uptimeMillis();
		    Log.d(TAG, 
			  TAG+"drawFrame(): fps: "
			  +String.valueOf(fpsRate)
			  );
	    }
	}
    }

    /**
     * Called when the engine is destroyed. Do any necessary clean up because
     * at this point your renderer instance is now done for.
     */
    public void release() {
	Log.d(TAG, 
	      "Released"
	      );
    }

    public void setScreenDimensions(int w, int h) {
	screenWidth = w;
	screenHeight = h;
	NativeCalls.setScreenDimensions( screenWidth,
					 screenHeight );
	Log.d(TAG,"New screen dimensions:"+screenWidth+"x"+screenHeight);
    }

    //set texture dimensions 
    //set nearest power of 2 dimensions for
    //texture based on either screen dimensions OR video
    public void setTextureDimensions(boolean span) {
	int s = Math.max( screenWidth, screenHeight );
	powWidth = getNextHighestPO2( s ) / 2;
	powHeight = getNextHighestPO2( s ) / 2;
	NativeCalls.setTextureDimensions( powWidth,
					  powHeight );
	Log.d(TAG,"New texture dimensions:"+powWidth+"x"+powHeight);
    }

    //set dimensions for fit-to-screen video
    //INPUT: video dimensions
    public void setFitToScreenDimensions(int w, int h) {
	int[] newdims = scaleBasedOnScreen(w, h,
					   screenWidth, screenHeight);
	drawWidth = newdims[0]; 
	drawHeight = newdims[1];
	NativeCalls.setDrawDimensions(drawWidth,drawHeight);
	Log.d(TAG,"setupVideoParameters: fit-to-screen video:"+drawWidth+"x"+drawHeight);
	//set video padding
	paddingX = (int) ((float) (screenWidth - drawWidth) / 2.0f);
	paddingY = (int) ((float) (screenHeight - drawHeight) / 2.0f);
	NativeCalls.setScreenPadding(paddingX,paddingY);
    }

    public static int getNextHighestPO2( int n ) {
	n -= 1;
	n = n | (n >> 1);
	n = n | (n >> 2);
	n = n | (n >> 4);
	n = n | (n >> 8);
	n = n | (n >> 16);
	n = n | (n >> 32);
	return n + 1;
    }

    public static int[] scaleBasedOnScreen(int iw, int ih, int sw, int sh) {
	int[] newdims = new int[2];
	//if ( videoWideScreen ) { int t=sw; sw=sh; sh=t; }
	float sf = (float) iw / (float) ih;
	newdims[0] = sw;
	newdims[1] = (int) ((float) sw / sf);
	if ( newdims[1] > sh ) { // new dims too big
	    newdims[0] = (int) ((float) sh * sf);
	    newdims[1] = sh;
	}
	return newdims;
    }	    
}