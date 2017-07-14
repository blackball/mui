/***********************************************************************
 **
 **  Licensed under the Apache License, Version 2.0 (the "License");
 **  you may not use this file except in compliance with the License.
 **  You may obtain a copy of the License at
 **
 **  http://www.apache.org/licenses/LICENSE-2.0
 **
 **  Unless required by applicable law or agreed to in writing, software
 **  distributed under the License is distributed on an "AS IS" BASIS,
 **  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 **  See the License for the specific language governing permissions and
 **  limitations under the License.
 **
 ************************************************************************
 **
 **  Summary :  Frameless minimum image window library based on Xlib directly.
 **  Author  :  Hui Li (bugway / gmail.com)
 **  Created :  2017-07-01
 **
 ***********************************************************************/

#include "sui.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>

#ifndef ASSERT
#include <assert.h>
#define ASSERT(expr) assert(expr)
#endif

#ifdef __cplusplus
extern "C" {
#endif 
    
static int
getticks() {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

typedef struct {
    int w, h, ws, cn;
    uint8_t *imgdata;
} sui_image;

static sui_image*
sui_image_create(int w, int h) {
    sui_image *img = (sui_image *)malloc(sizeof(*img) + w * h * 4);
    if (img) {
        img->w = w; img->h = h; img->ws = 4 * img->w; img->cn = 4;
        img->imgdata = (uint8_t *)(img + 1);
    }
    return img;
}

static int
sui_image_destroy(sui_image **pp) {
    if (pp && *pp) {
        free(*pp);
        *pp = NULL;
    }
    return -1;
}

static int
sui_image_set(sui_image *img, uint8_t b, uint8_t g, uint8_t r) {
    uint8_t *p;
    int i, j;
    if (img == NULL || img->imgdata == NULL) return -1;
    p = img->imgdata;
    for (i = 0; i < img->h; ++i) {
        for (j = 0; j < img->w; ++j) {
            *p++ = b;
            *p++ = g;
            *p++ = r;
            ++p;
        }
    }
    return 0;
}

#pragma GCC push_options
#pragma GCC optimize ("unroll-loops")
static int
sui_image_copy(sui_image *img, const uint8_t *imgdata, int w, int h, int ws, int cn) {
    if (cn != 1 && cn != 3 && cn != 4) {
        return -1;
    }
    
    if (img->w == w && img->h == h && img->ws == ws && img->cn == cn) {
        memcpy(img->imgdata, imgdata, sizeof(uint8_t) * ws * h);
    }
    else {
        if (cn == 1) { // gray scale image
            uint8_t *ps = img->imgdata;
            const uint8_t *pd = imgdata;        
            for (int i = 0; i < h; ++i) {
                for (int j = 0; j < w; ++j) {
                    ps[0] = ps[1] = ps[2] = *pd++;
                    ps += 4;
                }
                pd += ws - w;
            }
        }
        else if (cn == 3) {
            uint32_t *psi = (uint32_t *)img->imgdata;
            const uint8_t *pd = imgdata;
            for (int i = 0; i < h; ++i) {
                for (int j = 0; j < w; ++j) {
                    *psi++ = *(const uint32_t*)pd;
                    pd += 3;
                }
                pd += ws - w*3;
            }
        }
    }
    return 0;
}
#pragma GCC pop_options
    
// FIXME(Hui): this table is only made for my laptop 
static uint8_t keycode_to_ascii_lut[256] = {
    0,   1,   2,   3,   4,   5,   6,   7,   8,   27,  '1',  '2',  '3',
    '4',  '5',  '6',  '7',  '8',  '9',  '0',  '-',  '=',  8,  9,  'q',  'w',
    'e',  'r',  't',  'y',  'u',  'i',  'o',  'p',  '[',  ']',  13,  37,  'a',
    's',  'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',  '\'',  '`',  16,  '\\',
    'z',  'x',  'c',  'v',  'b',  'n',  'm',  ',',  '.',  '/',  16,  63,  64,
    32,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,
    78,  79,  80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,
    91,  92,  93,  94,  95,  96,  97,  98,  99, 100, 101, 102, 103,
    104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116,
    117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129,
    130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142,
    143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155,
    156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168,
    169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181,
    182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194,
    195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
    208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220,
    221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233,
    234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246,
    247, 248, 249, 250, 251, 252, 253, 254, 255,
}; 

static uint8_t
keycode_to_ascii(int code) {
    return keycode_to_ascii_lut[code];
}

static void
sui_default_callback(int e, int x, int y, int flag, void *d) {
    ; // do nothing 
}

static void
set_frameless_or_fullscreen(Display *dpy, Window win, int mode) {
    if (mode == 1) { 
        Atom atoms[2] = { XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", 0), None };
        XChangeProperty(dpy, win, XInternAtom(dpy, "_NET_WM_STATE", 0), 4, 32, PropModeReplace, (unsigned char *)atoms, 1);        
    }
    else {
        Atom mwmHintsProperty = XInternAtom(dpy, "_MOTIF_WM_HINTS", 0);
        static uint32_t hints[5] = {2, 0, 0, 0, 0}; // need to be accessed after this function 
        XChangeProperty(dpy, win, mwmHintsProperty, mwmHintsProperty, 32, PropModeReplace, (unsigned char *)hints, 5);
    }
}

typedef struct Sui {
    int w, h, mode;   
    sui_callback cb;
    void *cb_dataptr;
    sui_image *img;
    XImage *ximg;
    Display *display;
    Visual *visual;
    Window window;
    XExposeEvent expose_event;
} Sui;


static XExposeEvent
sui_create_exposeevent(Sui *ui, int x, int y, int w, int h) {
    XExposeEvent e;
    e.type = Expose;
    e.send_event = True;
    e.display = ui->display;
    e.window = ui->window;
    e.x = x; e.y = y;
    e.width = w;
    e.height = h;
    e.count = 0;
    return e;
}

Sui*
sui_create(int w, int h, int mode) {
    Display *display = NULL;
    Visual *visual = NULL;
    XImage *ximg = NULL;
    Sui *ui = NULL;
    Window window;
    
    ASSERT(w > 0 && h > 0);
    
    display = XOpenDisplay(NULL);
    if (display == NULL) {
        goto cleanup;
    }
    
    visual = DefaultVisual(display, 0);    
    if (visual == NULL) {
        goto cleanup;
    }

    if(visual->class != TrueColor) {
        goto cleanup;
    }
    
    // TODO(Hui): handle errors
    window = XCreateSimpleWindow(display, RootWindow(display, 0), 0, 0, w, h, 1, 0, 0);
    set_frameless_or_fullscreen(display, window, mode);
    XSelectInput(display, window, PointerMotionMask|ButtonPressMask|ButtonReleaseMask|ExposureMask|KeyPressMask|KeyReleaseMask);
    XMapWindow(display, window);
        
    ui = (Sui *)malloc(sizeof(*ui));
    if (ui == NULL) {
        XDestroyWindow(display, window);
        goto cleanup;
    }
    else {
        sui_image *img = sui_image_create(w, h);
        if (img == NULL) {
            goto cleanup;
        }
        
        sui_image_set(img, 0x2C, 0x2C, 0x2C);
        ximg = XCreateImage(display, visual, 24, ZPixmap, 0, (char *)img->imgdata, w, h, 32, 0);
        if (ximg == NULL) {
            sui_image_destroy(&img);
            goto cleanup;
        }
        
        ui->w = w; ui->h =h ; ui->mode = mode;
        ui->cb = &sui_default_callback;
        ui->cb_dataptr = NULL;
        ui->img = img;                
        ui->ximg = ximg;
        ui->display = display;
        ui->visual = visual;
        ui->window = window;
        ui->expose_event = sui_create_exposeevent(ui, 0, 0, w, h);
    }

    return ui;
    
 cleanup:    
    if (display) XCloseDisplay(display);
    if (ximg) XDestroyImage(ximg);
    return NULL;
}

int
sui_destroy(Sui **pp) {
    if (pp && *pp) {
        Sui *p = *pp;
        if (p->ximg) {
            p->ximg->data = NULL; // the data belongs to ui->img
            XDestroyImage(p->ximg);
        }
        if (p->display) {
            XDestroyWindow(p->display, p->window);
            XCloseDisplay(p->display);
        }
        if (p->img) sui_image_destroy(&(p->img));
        *pp = NULL;
        return 0;
    }
    return -1; 
}

int
sui_move(Sui *ui, int nx, int ny) {
    if (ui && ui->mode == 0) {
        XMoveWindow(ui->display, ui->window, nx, ny);
        return 0;
    }
    return -1;
}


int
sui_resize(Sui *ui, int nw, int nh) {
    if (ui == NULL || nw <=0 || nh <= 0) return -1;
    if (ui->w == nw && ui->h == nh) return 0;
    else { // recreate images 
        XImage *ximg = NULL;
        sui_image *img = sui_image_create(nw, nh);        
        if (img == NULL) {
            return -1;
        }
        
        sui_image_set(img, 0x2C, 0x2C, 0x2C);        
        ximg = XCreateImage(ui->display, ui->visual, 24, ZPixmap, 0, (char *)img->imgdata, nw, nh, 32, 0);
        
        if (ximg == NULL) {
            sui_image_destroy(&img);
            return -1;
        }

        sui_image_destroy(&(ui->img));
        ui->img = img;
        ui->ximg->data = NULL;
        XDestroyImage(ui->ximg);
        ui->ximg = ximg;
        ui->w = nw; ui->h = nh;
    }
    
    XResizeWindow(ui->display, ui->window, nw, nh);
    return 0;
}
 
int
sui_setcallback(Sui *p, void (* cb)(int, int, int, int, void *), void *dataptr) {
    if (p == NULL) return -1;
    p->cb = cb;
    p->cb_dataptr = dataptr;
    return 0;
}
    
int
sui_show(Sui *ui, const unsigned char *imgdata, int w, int h, int ws, int cn) {
    if (ui == NULL || ui->img == NULL) return -1;    
    if (imgdata && w > 0 && h > 0 && ws > 0 && cn > 0) {
        if (ui->img->w != w || ui->img->h != h) {
            return -1;
        }
        
        if (0 == sui_image_copy(ui->img, imgdata, w, h, ws, cn)) {
            XSendEvent(ui->display, ui->window, False, 0, (XEvent*)&(ui->expose_event));
            return 0;
        }
        else {
            return -1;
        }
    }
    // TODO(Hui): do nothing, better return values ? 
    return -1;
}

int
sui_wait(Sui *ui, int ms) {
    const int start_time = getticks();
    int is_press = 0;
    XEvent event;
    
    if (ui == NULL) {
        return -1;
    }

    for (;;) {
        if (ms > 0 && (getticks() - start_time > ms)) break;        
        usleep(2000); // sleep 2 ms        
        // handle event only when there is
        if (XPending(ui->display)) {
            XNextEvent(ui->display, &event);            
            switch(event.type) {
            case Expose:
                XPutImage(ui->display, ui->window, DefaultGC(ui->display, 0), ui->ximg, 0, 0, 0, 0, ui->w, ui->h);
                break;
            case ButtonPress:
            case ButtonRelease:                
                { 
                    int cvetype = 0, flag = 0;
                    const int x = event.xbutton.x;
                    const int y = event.xbutton.y;                    
                    is_press = event.type == ButtonPress ? 1 : 0;
                    switch(event.xbutton.button) {
                    case Button1: flag = 1; cvetype = is_press ? 1 : 4; break;
                    case Button3: flag = 2; cvetype = is_press ? 2 : 5; break;
                    case Button2: flag = 4; cvetype = is_press ? 3 : 6; break;
                    }
                    ui->cb(cvetype, x, y, flag, ui->cb_dataptr);
                }
                break;            
            case MotionNotify: // mouse motion
                ui->cb(0, event.xbutton.x, event.xbutton.y, 0, ui->cb_dataptr);
                break;            
            case KeyPress:
                return keycode_to_ascii(event.xkey.keycode);
            }
        }
    }
    return 0;
}

#ifdef __cplusplus
}
#endif 
