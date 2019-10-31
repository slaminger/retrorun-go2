#include "video.h"

#include "libretro.h"

#include <stdlib.h>
#include <stdio.h>
#include <exception>
#include <string.h>

#include <go2/display.h>

#define EGL_EGLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <drm/drm_fourcc.h>


#define ALIGN(val, align)	(((val) + (align) - 1) & ~((align) - 1))


go2_display_t* display;
go2_surface_t* surface;
go2_surface_t* display_surface;
go2_frame_buffer_t* frame_buffer;
go2_presenter_t* presenter;
float aspect_ratio;
uint32_t color_format;

bool isOpenGL = false;
int GLContextMajor = 0;
int GLContextMinor = 0;
int hasStencil = false;

extern retro_hw_context_reset_t retro_context_reset;


void video_configure(const struct retro_game_geometry* geom)
{
	printf("video_configure: base_width=%d, base_height=%d, max_width=%d, max_height=%d, aspect_ratio=%f\n",
        geom->base_width, geom->base_height,
        geom->max_width, geom->max_height,
        geom->aspect_ratio);

    
    display = go2_display_create();
    presenter = go2_presenter_create(display, DRM_FORMAT_RGB565, 0xff080808);  // ABGR


    if (isOpenGL)
    {
        throw std::exception();

        retro_context_reset();
    }
    else
    {
        if (surface) abort();

        int aw = ALIGN(geom->max_width, 32);
        int ah = ALIGN(geom->max_height, 32);
        printf ("video_configure: aw=%d, ah=%d\n", aw, ah);

        surface = go2_surface_create(display, aw, ah, color_format);
        if (!surface)
        {
            printf("go2_surface_create failed.\n");
            throw std::exception();
        }
        

        aspect_ratio = geom->aspect_ratio;        
        //printf("video_configure: rect=%d, %d, %d, %d\n", y, x, h, w);
    }
}

void video_deinit()
{

}


uintptr_t core_video_get_current_framebuffer()
{
    printf("core_video_get_current_framebuffer\n");
    throw std::exception();

    //return fbo;
}

void core_video_refresh(const void * data, unsigned width, unsigned height, size_t pitch)
{
    //printf("core_video_refresh: width=%d, height=%d, pitch=%d (format=%d)\n", width, height, pitch, format);

    if (isOpenGL)
    {
        throw std::exception();
    }
    else
    {
        uint8_t* src = (uint8_t*)data;
        uint8_t* dst = (uint8_t*)go2_surface_map(surface);
        int bpp = go2_drm_format_get_bpp(go2_surface_format_get(surface)) / 8;

        int yy = height;
        while(yy > 0)
        {
            memcpy(dst, src, width * bpp);
            
            src += pitch;
            dst += go2_surface_stride_get(surface);
            
            --yy;
        }

        // go2_surface_blit(surface, 0, 0, width, height,
        //                  display_surface, 0, 0, go2_display_width_get(display), go2_display_height_get(display),
        //                  GO2_ROTATION_DEGREES_270);

        int x;
        int y;
        int w;
        int h;
        if (aspect_ratio >= 1.0f)
        {
            h = go2_display_width_get(display);
            
            w = h * aspect_ratio;
            w = (w > go2_display_height_get(display)) ? go2_display_height_get(display) : w;

            x = (go2_display_height_get(display) / 2) - (w / 2);
            y = 0;
        }
        else
        {
            x = 0;
            y = 0;
            w = go2_display_height_get(display);
            h = go2_display_width_get(display);
        }

        go2_presenter_post(presenter,
                           surface,
                           0, 0, width, height,
                           y, x, h, w,
                           GO2_ROTATION_DEGREES_270);
    }
}
