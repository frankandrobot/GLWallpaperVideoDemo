package frankandrobot.glwallpapervideodemo.com;

public class NativeCalls {
    //ffmpeg
    public static native void initVideo(); //
    public static native void loadVideo(String fileName); //
    public static native void prepareStorageFrame(); //
    public static native void getFrame(); //
    public static native void closeVideo();//
    //opengl
    public static native void initOpenGL(); //
    public static native void drawFrame(); //
    public static native void closeOpenGL(); //
    //getters
    public static native int getVideoHeight();
    public static native int getVideoWidth();
    //setters
    public static native void setScreenPadding(int w,int h); //
    public static native void setDrawDimensions(int drawWidth,//
						int drawHeight);
    public static native void setScreenDimensions(int w, int h); //
    public static native void setTextureDimensions(int tx, //
						   int ty );

    static {  
	System.loadLibrary("avcore");
	System.loadLibrary("avformat");
	System.loadLibrary("avcodec");
	System.loadLibrary("avdevice");
	System.loadLibrary("avfilter");
	System.loadLibrary("avutil");
	System.loadLibrary("swscale");
	System.loadLibrary("video");  
    }  

}
