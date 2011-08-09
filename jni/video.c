#include <GLES/gl.h>
#include <GLES/glext.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#include <jni.h>  
#include <string.h>  
#include <stdio.h>
#include <android/log.h>  

//ffmpeg video variables
int      initializedVideo=0;
int      initializedFrame=0;
AVFormatContext *pFormatCtx=NULL;
int             videoStream;
AVCodecContext  *pCodecCtx=NULL;
AVCodec         *pCodec=NULL;
AVFrame         *pFrame=NULL;
AVPacket        packet;
int             frameFinished;
float           aspect_ratio;

//ffmpeg video conversion variables
AVFrame         *pFrameConverted=NULL;
int             numBytes;
uint8_t         *bufferConverted=NULL;

//opengl
int textureFormat=PIX_FMT_RGBA;//PIX_FMT_RGB24;
int textureWidth=256;
int textureHeight=256;
int nTextureHeight=-256;
static GLuint textureConverted=0;

//screen dimensions
int screenWidth = 50;
int screenHeight= 50;
//video
int dPaddingX=0,dPaddingY=0;
int drawWidth=50,drawHeight=50;

//file
const char * szFileName;

void Java_frankandrobot_glwallpapervideodemo_com_NativeCalls_initVideo
(JNIEnv * env, jobject this)  {
  initializedVideo = 0;
  initializedFrame = 0;
}

/* list of things that get loaded: */
/* buffer */
/* pFrameConverted */
/* pFrame */
/* pCodecCtx */
/* pFormatCtx */
void Java_frankandrobot_glwallpapervideodemo_com_NativeCalls_loadVideo
(JNIEnv * env, jobject this, jstring fileName)  {
  jboolean isCopy;  
  szFileName = (*env)->GetStringUTFChars(env, fileName, &isCopy);  
  //debug
  __android_log_print(ANDROID_LOG_DEBUG, "NDK: ", "NDK:LC: [%s]", szFileName); 
  // Register all formats and codecs
  av_register_all();
  // Open video file
  if(av_open_input_file(&pFormatCtx, szFileName, NULL, 0, NULL)!=0) {
    __android_log_print(ANDROID_LOG_DEBUG, 
			"video.c", 
			"NDK: Couldn't open file");
    return;
  }
  __android_log_print(ANDROID_LOG_DEBUG, 
		      "video.c", 
		      "NDK: Succesfully loaded file");
  // Retrieve stream information */
  if(av_find_stream_info(pFormatCtx)<0) {
    __android_log_print(ANDROID_LOG_DEBUG, 
			"video.c", 
			"NDK: Couldn't find stream information");
    return;
  }
  __android_log_print(ANDROID_LOG_DEBUG, 
		      "video.c", 
		      "NDK: Found stream info");
  // Find the first video stream
  videoStream=-1;
  int i;
  for(i=0; i<pFormatCtx->nb_streams; i++)
    if(pFormatCtx->streams[i]->codec->codec_type==CODEC_TYPE_VIDEO) {
      videoStream=i;
      break;
    }
  if(videoStream==-1) {
    __android_log_print(ANDROID_LOG_DEBUG, 
			"video.c", 
			"NDK: Didn't find a video stream");
    return;
  }
  __android_log_print(ANDROID_LOG_DEBUG, 
		      "video.c", 
		      "NDK: Found video stream");
  // Get a pointer to the codec contetx for the video stream
  pCodecCtx=pFormatCtx->streams[videoStream]->codec;
  // Find the decoder for the video stream
  pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
  if(pCodec==NULL) {
    __android_log_print(ANDROID_LOG_DEBUG, 
			"video.c", 
			"NDK: Unsupported codec");
    return;
  }
  // Open codec
  if(avcodec_open(pCodecCtx, pCodec)<0) {
    __android_log_print(ANDROID_LOG_DEBUG, 
			"video.c", 
			"NDK: Could not open codec");
    return;
  }
  // Allocate video frame (decoded pre-conversion frame)
  pFrame=avcodec_alloc_frame();
  // keep track of initialization
  initializedVideo = 1;
  __android_log_print(ANDROID_LOG_DEBUG, 
		      "video.c", 
		      "NDK: Finished loading video");
}

//for this to work, you need to set the scaled video dimensions first
void Java_frankandrobot_glwallpapervideodemo_com_NativeCalls_prepareStorageFrame
(JNIEnv * env, jobject this)  {
  // Allocate an AVFrame structure
  pFrameConverted=avcodec_alloc_frame();
  // Determine required buffer size and allocate buffer
  numBytes=avpicture_get_size(textureFormat, 
			      textureWidth,
			      textureHeight);
  bufferConverted=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
  if ( pFrameConverted == NULL || bufferConverted == NULL ) 
    __android_log_print(ANDROID_LOG_DEBUG, "prepareStorage>>>>", "Out of memory");
  // Assign appropriate parts of buffer to image planes in pFrameRGB
  // Note that pFrameRGB is an AVFrame, but AVFrame is a superset
  // of AVPicture
  avpicture_fill((AVPicture *)pFrameConverted, 
		 bufferConverted, 
		 textureFormat,
		 textureWidth, 
		 textureHeight);
  __android_log_print(ANDROID_LOG_DEBUG, "prepareStorage>>>>", "Created frame");
  __android_log_print(ANDROID_LOG_DEBUG,
  		      "prepareStorage>>>>",
  		      "texture dimensions: %dx%d", 
		      textureWidth, textureHeight
  		      );
    initializedFrame = 1;
}

jint Java_frankandrobot_glwallpapervideodemo_com_NativeCalls_getVideoWidth
(JNIEnv * env, jobject this)  {
  return pCodecCtx->width;
}

jint Java_frankandrobot_glwallpapervideodemo_com_NativeCalls_getVideoHeight
(JNIEnv * env, jobject this)  {
  return pCodecCtx->height;
}
 
void Java_frankandrobot_glwallpapervideodemo_com_NativeCalls_getFrame
(JNIEnv * env, jobject this)  {
  // keep reading packets until we hit the end or find a video packet
  while(av_read_frame(pFormatCtx, &packet)>=0) {
    static struct SwsContext *img_convert_ctx;
    // Is this a packet from the video stream?
    if(packet.stream_index==videoStream) {
      // Decode video frame
      /* __android_log_print(ANDROID_LOG_DEBUG,  */
      /* 			  "video.c",  */
      /* 			  "getFrame: Try to decode frame" */
      /* 			  ); */
      avcodec_decode_video(pCodecCtx, 
			   pFrame, 
			   &frameFinished, 
			   packet.data, 
			   packet.size);
      // Did we get a video frame?
      if(frameFinished) {
	if(img_convert_ctx == NULL) {
	  /* get/set the scaling context */
	  int w = pCodecCtx->width;
	  int h = pCodecCtx->height;
	  img_convert_ctx = 
	    sws_getContext(
			   w, h, //source
			   pCodecCtx->pix_fmt,
			   textureWidth,textureHeight,
			   //w, h, //destination
			   textureFormat, 
			   SWS_FAST_BILINEAR,
			   NULL, NULL, NULL
			   );
	  if(img_convert_ctx == NULL) {
	    /* __android_log_print(ANDROID_LOG_DEBUG,  */
	    /* 			"video.c",  */
	    /* 			"NDK: Cannot initialize the conversion context!" */
	    /* 			); */
	    return;
	  }
	} /* if img convert null */
	/* finally scale the image */
	/* __android_log_print(ANDROID_LOG_DEBUG,  */
	/* 			"video.c",  */
	/* 			"getFrame: Try to scale the image" */
	/* 			); */
	sws_scale(img_convert_ctx,
		  pFrame->data,
		  pFrame->linesize, 
		  0, pCodecCtx->height,
		  pFrameConverted->data, 
		  pFrameConverted->linesize);
	/* do something with pFrameConverted */
	/* ... see drawFrame() */
	/* We found a video frame, did something with it, no free up
	   packet and return */
	av_free_packet(&packet);
	return;
      } /* if frame finished */
    } /* if packet video stream */ 
    // Free the packet that was allocated by av_read_frame
    av_free_packet(&packet);
  } /* while */
  //reload video when you get to the end
  av_seek_frame(pFormatCtx,videoStream,0,AVSEEK_FLAG_ANY);
}

void Java_frankandrobot_glwallpapervideodemo_com_NativeCalls_closeVideo
(JNIEnv * env, jobject this) {  
  if ( initializedFrame == 1 ) {
    // Free the converted image
    av_free(bufferConverted); 
    av_free(pFrameConverted); 
    initializedFrame = 0;
  __android_log_print(ANDROID_LOG_DEBUG, "closeVideo>>>>", "Freed converted image");
  }
  if ( initializedVideo == 1 ) {
    /* // Free the YUV frame */
    av_free(pFrame); 
    /* // Close the codec */
    avcodec_close(pCodecCtx); 
    // Close the video file
    av_close_input_file(pFormatCtx); 
  __android_log_print(ANDROID_LOG_DEBUG, "closeVideo>>>>", "Freed video structures");
    initializedVideo = 0;
  }
}

/*--- END OF VIDEO ----*/

/* disable these capabilities. */
static GLuint s_disable_options[] = {
	GL_FOG,
	GL_LIGHTING,
	GL_CULL_FACE,
	GL_ALPHA_TEST,
	GL_BLEND,
	GL_COLOR_LOGIC_OP,
	GL_DITHER,
	GL_STENCIL_TEST,
	GL_DEPTH_TEST,
	GL_COLOR_MATERIAL,
	0
};

void Java_frankandrobot_glwallpapervideodemo_com_NativeCalls_initOpenGL
(JNIEnv * env, jobject this)  {
  __android_log_print(ANDROID_LOG_DEBUG,
		      "NDK:",
		      "initOpenGL()"
		      );
  __android_log_print(ANDROID_LOG_DEBUG,
  		      "NDK initOpenGL()",
  		      "texture dimensions: [%d]x[%d]", 
		      textureWidth, textureHeight
  		      );
  //Disable stuff
  __android_log_print(ANDROID_LOG_DEBUG,
  		      "NDK initOpenGL()",
  		      "disabling some opengl options"
  		      );
  GLuint *start = s_disable_options;
  while (*start) glDisable(*start++);
  //setup textures
  __android_log_print(ANDROID_LOG_DEBUG,
		      "NDK initOpenGL()",
		      "enabling and generating textures"
		      );
  glEnable(GL_TEXTURE_2D);
  glGenTextures(1, &textureConverted);
  glBindTexture(GL_TEXTURE_2D,textureConverted);
  //...and bind it to our array
  __android_log_print(ANDROID_LOG_DEBUG,
		      "NDK initOpenGL()",
		      "binded texture"
		      );
  glTexParameterf(GL_TEXTURE_2D, 
		  GL_TEXTURE_MIN_FILTER, 
		  GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, 
		  GL_TEXTURE_MAG_FILTER, 
		  GL_NEAREST);
  //Different possible texture parameters, e.g. GL10.GL_CLAMP_TO_EDGE
  glTexParameterf(GL_TEXTURE_2D, 
		  GL_TEXTURE_WRAP_S, 
		  GL_CLAMP_TO_EDGE);
  //GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, 
		  GL_TEXTURE_WRAP_T, 
		  GL_CLAMP_TO_EDGE);
  //GL_REPEAT);
  glTexImage2D(GL_TEXTURE_2D,		/* target */
	       0,			/* level */
	       GL_RGBA,			/* internal format */
	       textureWidth,		/* width */
	       textureHeight,		/* height */
	       0,			/* border */
	       GL_RGBA,			/* format */
	       GL_UNSIGNED_BYTE,/* type */
	       NULL);
  //setup simple shading
  glShadeModel(GL_FLAT);
  //check_gl_error("glShademo_comdel");
  glColor4x(0x10000, 0x10000, 0x10000, 0x10000);
}

void Java_frankandrobot_glwallpapervideodemo_com_NativeCalls_drawFrame
(JNIEnv * env, jobject this)  {
  glClear(GL_COLOR_BUFFER_BIT);
  glBindTexture(GL_TEXTURE_2D,textureConverted);
  int rect[4] = {0, textureHeight, textureWidth, nTextureHeight};
  glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, rect);
  //Reference: 
  //http://old.siggraph.org/publications/2006cn/course16/KhronosSpecs/gl_egl_ref_1.1.20041110/glTexSubImage2D.html
  glTexSubImage2D(GL_TEXTURE_2D, /* target */
		  0,		/* level */
		  0,	/* xoffset */
		  0,	/* yoffset */
		  textureWidth,
		  textureHeight,
		  GL_RGBA,	/* format */
		  GL_UNSIGNED_BYTE, /* type */
		  pFrameConverted->data[0]);
  //Reference:
  //http://old.siggraph.org/publications/2006cn/course16/KhronosSpecs/gl_egl_ref_1.1.20041110/glDrawTex.html
  glDrawTexiOES(dPaddingX, dPaddingY, 0, drawWidth, drawHeight);
}

void Java_frankandrobot_glwallpapervideodemo_com_NativeCalls_closeOpenGL
(JNIEnv *env, jobject this) {
  glDeleteTextures(1, &textureConverted); 
}

void Java_frankandrobot_glwallpapervideodemo_com_NativeCalls_setScreenDimensions
(JNIEnv *env, jobject this, jint w, jint h) {
  screenWidth = w;
  screenHeight = h;
}

void Java_frankandrobot_glwallpapervideodemo_com_NativeCalls_setTextureDimensions
(JNIEnv *env, jobject this, jint px, jint py) {
  textureWidth = px;
  textureHeight = py;
  nTextureHeight = -1*py;
}

void Java_frankandrobot_glwallpapervideodemo_com_NativeCalls_setDrawDimensions
(JNIEnv *env, jobject this, jint w, jint h) {
  drawWidth = w;
  drawHeight = h;
}

void Java_frankandrobot_glwallpapervideodemo_com_NativeCalls_setScreenPadding
(JNIEnv *env, jobject this, jint w, jint h) {
  dPaddingX = w;
  dPaddingY = h;
}
