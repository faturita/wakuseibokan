#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GL_GLEXT_PROTOTYPES 1
#include <GLUT/glut.h>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
#include <libavformat/avformat.h>
}


/**
  g++ -Wall --verbose -I/System/Library/Frameworks/OpenGL.framework/Headers  -I/usr/local/include -I/usr/include -I/usr/include/GL -framework OpenGL -framework GLUT -L/usr/local/lib writevideosample.cpp `pkg-config --cflags --libs libavcodec --libs libavutil --libs libavformat --libs libavframe` -lavutil -lavcodec -lavformat -lavdevice -lavfilter -ldl  -lz -lswscale -lm  -o out
 * @brief The Constants enum
 */

enum Constants { SCREENSHOT_MAX_FILENAME = 256 };
static GLubyte *pixels = NULL;
static GLuint fbo;
static GLuint rbo_color;
static GLuint rbo_depth;
static const unsigned int HEIGHT = 100;
static const unsigned int WIDTH = 100;
static int offscreen = 1;
static unsigned int max_nframes = 100;
static unsigned int nframes = 0;
static unsigned int time0;

/* Model. */
static double angle;
static double delta_angle;

/* Adapted from: https://github.com/cirosantilli/cpp-cheat/blob/19044698f91fefa9cb75328c44f7a487d336b541/ffmpeg/encode.c */
static AVCodecContext *c = NULL;
static AVFrame *frame;
static AVPacket pkt;
static FILE *file;
static struct SwsContext *sws_context = NULL;
static uint8_t *rgb = NULL;

static void ffmpeg_encoder_set_frame_yuv_from_rgb(uint8_t *rgb) {
    const int in_linesize[1] = { 4 * c->width };
    sws_context = sws_getCachedContext(sws_context,
            c->width, c->height, AV_PIX_FMT_RGB32,
            c->width, c->height, AV_PIX_FMT_YUV420P,
            0, NULL, NULL, NULL);
    sws_scale(sws_context, (const uint8_t * const *)&rgb, in_linesize, 0,
            c->height, frame->data, frame->linesize);
}

void ffmpeg_encoder_start(const char *filename, int codec_id, int fps, int width, int height) {
    AVCodec *codec;
    int ret;
    avcodec_register_all();
    codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!codec) {
        fprintf(stderr, "Codec not found\n");
        exit(1);
    }
    c = avcodec_alloc_context3(codec);
    if (!c) {
        fprintf(stderr, "Could not allocate video codec context\n");
        exit(1);
    }
    c->bit_rate = 400000;
    c->width = width;
    c->height = height;
    c->time_base.num = 1;
    c->time_base.den = fps;
    c->gop_size = 10;
    c->max_b_frames = 1;
    c->pix_fmt = AV_PIX_FMT_YUV420P;
    if (codec_id == AV_CODEC_ID_H264)
        av_opt_set(c->priv_data, "preset", "slow", 0);
    if (avcodec_open2(c, codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        exit(1);
    }
    file = fopen(filename, "wb");
    if (!file) {
        fprintf(stderr, "Could not open %s\n", filename);
        exit(1);
    }
    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }
    frame->format = c->pix_fmt;
    frame->width  = c->width;
    frame->height = c->height;
    ret = av_image_alloc(frame->data, frame->linesize, c->width, c->height, c->pix_fmt, 32);
    if (ret < 0) {
        fprintf(stderr, "Could not allocate raw picture buffer\n");
        exit(1);
    }
}

void ffmpeg_encoder_finish(void) {
    uint8_t endcode[] = { 0, 0, 1, 0xb7 };
    int got_output, ret;
    do {
        fflush(stdout);
        ret = avcodec_encode_video2(c, &pkt, NULL, &got_output);
        if (ret < 0) {
            fprintf(stderr, "Error encoding frame\n");
            exit(1);
        }
        if (got_output) {
            fwrite(pkt.data, 1, pkt.size, file);
            av_packet_unref(&pkt);
        }
    } while (got_output);
    fwrite(endcode, 1, sizeof(endcode), file);
    fclose(file);
    avcodec_close(c);
    av_free(c);
    av_freep(&frame->data[0]);
    av_frame_free(&frame);
}

void ffmpeg_encoder_encode_frame(uint8_t *rgb) {
    int ret, got_output;
    ffmpeg_encoder_set_frame_yuv_from_rgb(rgb);
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;
    ret = avcodec_encode_video2(c, &pkt, frame, &got_output);
    if (ret < 0) {
        fprintf(stderr, "Error encoding frame\n");
        exit(1);
    }
    if (got_output) {
        fwrite(pkt.data, 1, pkt.size, file);
        av_packet_unref(&pkt);
    }
}

void ffmpeg_encoder_glread_rgb(uint8_t **rgb, GLubyte **pixels, unsigned int width, unsigned int height) {
    size_t i, j, k, cur_gl, cur_rgb, nvals;
    const size_t format_nchannels = 4;
    nvals = format_nchannels * width * height;
    *pixels = (unsigned char *)realloc(*pixels, nvals * sizeof(GLubyte));
    *rgb = (unsigned char *)realloc(*rgb, nvals * sizeof(uint8_t));
    /* Get RGBA to align to 32 bits instead of just 24 for RGB. May be faster for FFmpeg. */
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, *pixels);
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            cur_gl  = format_nchannels * (width * (height - i - 1) + j);
            cur_rgb = format_nchannels * (width * i + j);
            for (k = 0; k < format_nchannels; k++)
                (*rgb)[cur_rgb + k] = (*pixels)[cur_gl + k];
        }
    }
}

static int model_init(void) {
    angle = 0;
    delta_angle = 1;
}

static int model_update(void) {
    angle += delta_angle;
    return 0;
}

static int model_finished(void) {
    return nframes >= max_nframes;
}

static void init(void)  {
    int glget;

    if (offscreen) {
        /*  Framebuffer */
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        /* Color renderbuffer. */
        glGenRenderbuffers(1, &rbo_color);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo_color);
        /* Storage must be one of: */
        /* GL_RGBA4, GL_RGB565, GL_RGB5_A1, GL_DEPTH_COMPONENT16, GL_STENCIL_INDEX8. */
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB, WIDTH, HEIGHT);
        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo_color);

        /* Depth renderbuffer. */
        glGenRenderbuffers(1, &rbo_depth);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo_depth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, WIDTH, HEIGHT);
        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_depth);

        glReadBuffer(GL_COLOR_ATTACHMENT0);

        /* Sanity check. */
        assert(glCheckFramebufferStatus(GL_FRAMEBUFFER));
        glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &glget);
        assert(WIDTH * HEIGHT < (unsigned int)glget);
    } else {
        glReadBuffer(GL_BACK);
    }

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glEnable(GL_DEPTH_TEST);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glViewport(0, 0, WIDTH, HEIGHT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);

    time0 = glutGet(GLUT_ELAPSED_TIME);
    model_init();
    ffmpeg_encoder_start("tmp.mpg", AV_CODEC_ID_MPEG1VIDEO, 25, WIDTH, HEIGHT);
}

static void deinit(void)  {
    printf("FPS = %f\n", 1000.0 * nframes / (double)(glutGet(GLUT_ELAPSED_TIME) - time0));
    free(pixels);
    ffmpeg_encoder_finish();
    free(rgb);
    if (offscreen) {
        glDeleteFramebuffers(1, &fbo);
        glDeleteRenderbuffers(1, &rbo_color);
        glDeleteRenderbuffers(1, &rbo_depth);
    }
}

static void draw_scene(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glRotatef(angle, 0.0f, 0.0f, -1.0f);
    glBegin(GL_TRIANGLES);
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f( 0.0f,  0.5f, 0.0f);
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-0.5f, -0.5f, 0.0f);
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f( 0.5f, -0.5f, 0.0f);
    glEnd();
}

static void display(void) {
    char extension[SCREENSHOT_MAX_FILENAME];
    char filename[SCREENSHOT_MAX_FILENAME];
    draw_scene();
    if (offscreen) {
        glFlush();
    } else {
        glutSwapBuffers();
    }
    frame->pts = nframes;
    ffmpeg_encoder_glread_rgb(&rgb, &pixels, WIDTH, HEIGHT);
    ffmpeg_encoder_encode_frame(rgb);
    nframes++;
    //if (model_finished())
      //  exit(EXIT_SUCCESS);
}

static void idle(void) {
    while (model_update());
    glutPostRedisplay();
}

int main(int argc, char **argv) {
    GLint glut_display;
    glutInit(&argc, argv);
    if (argc > 1)
        offscreen = 0;
    if (offscreen) {
        /* TODO: if we use anything smaller than the window, it only renders a smaller version of things. */
        /*glutInitWindowSize(50, 50);*/
        glutInitWindowSize(WIDTH, HEIGHT);
        glut_display = GLUT_SINGLE;
    } else {
        glutInitWindowSize(WIDTH, HEIGHT);
        glutInitWindowPosition(100, 100);
        glut_display = GLUT_DOUBLE;
    }
    glutInitDisplayMode(glut_display | GLUT_RGBA | GLUT_DEPTH);
    glutCreateWindow(argv[0]);
    if (offscreen) {
        /* TODO: if we hide the window the program blocks. */
        /*glutHideWindow();*/
    }
    init();
    glutDisplayFunc(display);
    glutIdleFunc(idle);
    atexit(deinit);
    glutMainLoop();
    return EXIT_SUCCESS;
}
