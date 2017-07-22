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
 **  Summary :  Mouse oriented UI library, currently depends on opencv .
 **  Author  :  Hui Li (bugway / gmail.com)
 **  Created :  2017-07-07
 **  Notes   :  With OpenCV's crappy highgui implementation, some functions 
 **             have different behaviors in different UI backend.
 **             In order to squeeze the performance and make the behaviour
 **             consistent, Sui is created directly based on Xlib, if you
 **             have Xlib in your machine, enable it with USE_SUI.
 **
 ***********************************************************************/

#ifndef MUI_H
#define MUI_H

//#define USE_SUI
#if defined(USE_SUI)
#include "sui.h"
#endif

#ifndef ASSERT
#include <assert.h>
#define ASSERT(expr) assert(expr);
#endif
#include <stdio.h>
#include <stdint.h>
#include <string>
#include <stdarg.h>
#include <opencv2/opencv.hpp>

namespace mui {

using std::string;
using cv::Mat;
using cv::Scalar;
using cv::Rect;
using cv::Size;
using cv::Point;
typedef unsigned int uint;

static const int CLICKED  = 0x1;
static const int IDLE     = 0x2;
static const int HOVERED  = 0x4;
static const int PRESSED  = 0x8;
static const int DISABLED = 0x16; 
static const int CHANGED  = 0x32; 
static const int INIT     = 0x64; 
static const int ALIGN_LEFT   = 0x100;
static const int ALIGN_CENTER = 0x101;
static const int ALIGN_RIGHT  = 0x102;
static const int KB_FULL = 0x103;
static const int KB_CHAR = 0x104;
static const int KB_NUM  = 0x105;

struct Mouse
{
    Mouse() {pressed = justReleased = false; x = y = 0;}
    bool pressed, justReleased; 
    int x, y;    
    bool isInside(const Rect &r) {
        return x >= r.x && x <= (r.x + r.width) && y >= r.y && y <= (r.y + r.height);
    }
};
static Mouse g_mouse;
 
static void
mouseCallback(int e, int x, int y, int flags, void *p) {
    g_mouse.x = x;
    g_mouse.y = y;
    g_mouse.justReleased = false;
    if (e == cv::EVENT_LBUTTONDOWN) g_mouse.pressed = true;
    else if (e == cv::EVENT_LBUTTONUP) {g_mouse.pressed = false; g_mouse.justReleased = true;}
}

static int
mouseStatus(const Rect &r, uint mask = (uint)(-1), int defaultV = IDLE) {
    int s = IDLE;
    if (g_mouse.isInside(r)) {
        if (g_mouse.justReleased) {                
            g_mouse.justReleased = false; // reset
            s = CLICKED;
        }
        else if (g_mouse.pressed) s = PRESSED;        
        else s = HOVERED;
    }    
    s = s & mask;
    return s == 0 ? defaultV : s;
}

static uint
colorAdd(uint v, uint m) {
    return ((((v >> 16) & 0xFF) + m) << 16) +
           ((((v >> 8)  & 0xFF) + m) << 8)  +
           (((v)       & 0xFF) + m);
}

static Scalar
toScalar(uint v) {
    return Scalar(v & 0xFF, (v >> 8) & 0xFF, (v >> 16) & 0xFF);
}

static void
circle(Mat &area, const Point center, int radius, uint color, int size = 1) {
    cv::circle(area, center, radius, toScalar(color), size, CV_AA);
}

static void
line(Mat &area, const Point a, const Point b, uint color, int size = 1) {
    cv::line(area, a, b, toScalar(color), size, CV_AA);
}

static void
rectangle(Mat &area, const Rect &r, uint color, int size = 1) {
    cv::rectangle(area, r, toScalar(color), size, CV_AA);
}

static void
fill(Mat &area, const Rect &r, uint color) {
    area(r) = toScalar(color);
}

static void
fill(Mat &area, const Point center, int radius, uint color) {
    cv::circle(area, center, radius, toScalar(color), -1, CV_AA);
}

struct Font
{
    Font() {
        color          = 0xE3E3E3;
        color_disabled = colorAdd(color, -0xA3);
        type           = cv::FONT_HERSHEY_SIMPLEX;
        AA             = CV_AA;
        scale          = 0.45f;
    }
    
    Point getTextPosition(const string &text, const Rect &roi, int align) {
        Point pos;
        const Size tsz = cv::getTextSize(text, type, scale, 1, NULL);
        pos.y = roi.y + (roi.height + tsz.height)/2 - 1;
        switch(align) {
        case ALIGN_LEFT:  pos.x =  roi.x + 1; break;
        case ALIGN_CENTER:pos.x =  roi.x +(roi.width - tsz.width)/2; break;
        case ALIGN_RIGHT: pos.x =  roi.x + roi.width - tsz.width; break;
        }
        return pos;
    }
    
    void putText(Mat &area, const Rect &roi, const string &text, bool disabled = false, int align = ALIGN_CENTER) {
        const uint c = disabled ? color_disabled : color;
        const Point pos = getTextPosition(text, roi, align);
        cv::putText(area, text, pos, type, scale, toScalar(c), 1, AA);
    }
    
    void putText(Mat &area, const string &text, bool disable = false, int align = ALIGN_CENTER) {
        putText(area, Rect(0,0,area.cols, area.rows), text, disable, align);
    }
    
    int color;
    int color_disabled;
    int type;
    int AA;
    float scale;    
};

struct Screen
{
    Screen() {
#if defined(USE_SUI)
        sui = NULL;
#endif
    }    
    Screen(int w, int h, int mode = 0) {
#if defined(USE_SUI)
        sui = NULL;
#endif
        init(w,h,mode);
    }
    
    int init(int w, int h, int mode = 0) {
        color  = 0x1E2027;
        width  = w;
        height = h;
        bg = Mat(Size(w, h), CV_8UC3, toScalar(color));
#if defined(USE_SUI)
        if (sui) sui_destroy(&sui);
        sui = sui_create(w, h, mode);
        sui_setcallback(sui, &mouseCallback, NULL);
#else 
        wname = "Mui";
        cv::namedWindow(wname, CV_WINDOW_AUTOSIZE);
        cv::setMouseCallback(wname, &mouseCallback, NULL);
        cv::setWindowProperty(wname, CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
#endif
        return 0;
    }
    
#if defined(USE_SUI)
    ~Screen() {sui_destroy(&sui);}
#endif
    
    void move(int nx, int ny) {
#if defined(USE_SUI)
        sui_move(sui, nx, ny);
#else
        cv::moveWindow(wname, nx, ny);
#endif
    }
    void clear() {bg = toScalar(color);}
    void clear(const Rect &roi) {bg(roi) = toScalar(color);}
    
    int show(int ms = 20) {
#if defined(USE_SUI)
        sui_show(sui, bg.data, bg.cols, bg.rows, bg.step, bg.channels());
        return sui_wait(sui, ms);
#else
        cv::imshow(wname, bg);
        return cv::waitKey(ms);
#endif
    }

#if defined(USE_SUI)
    Sui *sui;
#else
    string wname;
#endif 
    Mat bg;
    int width, height;
    int color;        
};

struct Button
{
    Button() {
        color          = 0x33353C;
        color_hovered  = 0x43454C;
        color_pressed  = 0x31313F;
        color_clicked  = 0x43454C;
        color_disabled = colorAdd(color, -0x13);
        
        status   = INIT;
        align    = ALIGN_CENTER;
        disabled = false; // disable the component 
    }

    void reset() {status = INIT; disabled = false;}
    void disable(bool v){disabled = v;}
    void redraw() {status = CHANGED;}
    
    int operator()(Screen &screen, const string &text, int x, int y, int w, int h) {
        const Rect roi(x,y,w,h);
        const int s = disabled ? DISABLED : mouseStatus(roi);
        if (s != status) {
            status = s;
            area = screen.bg(roi);
            area = getBgColor(s);
            font.putText(area, text, s == DISABLED, align);
        }
        
        return status;
    }

    Scalar getBgColor(const int s) {
        uint c = color;
        switch(s) {
        case DISABLED: c = color_disabled; break;
        case IDLE:     c = color; break;
        case HOVERED:  c = color_hovered; break;
        case PRESSED:  c = color_pressed; break;
        case CLICKED:  c = color_clicked; break;
        default: ASSERT(0); break;
        }
        return toScalar(c);
    }
    
    Font font;
    uint color;
    uint color_hovered;
    uint color_pressed;
    uint color_clicked;
    uint color_disabled;

    int status;
    int align;
    bool disabled;    
    Mat area;
};

struct Label : Button
{
    Label() : Button() {
        color_clicked = color_pressed = color_hovered = color;
    }
    
    int operator()(Screen &screen, const string &text, int x, int y, int w, int h) {
        const Rect roi(x,y,w,h);
        const int s = disabled ? DISABLED : mouseStatus(roi, IDLE | CLICKED);
        if (s != status) {
            status = s;
            area = screen.bg(roi);
            area = getBgColor(s);
            font.putText(area, text, s == DISABLED, align);
        }
        return status;
    }
};

static void
copyTo(const Mat &img, Mat &area, Mat *buff = NULL) {
    const int imgType = img.type(), areaType = area.type();
    const Size imgSize = img.size(), areaSize = area.size();
    
    ASSERT(imgType == CV_8UC1 || imgType == CV_8UC3);
    
    if (imgType == areaType && imgSize == areaSize) {
        img.copyTo(area);
    }
    else if (imgType == areaType) { // size not the same 
        cv::resize(img, area, areaSize);
    }
    else if (imgSize == areaSize) { // type not the same 
        cv::cvtColor(img, area, cv::COLOR_GRAY2BGR);
    }
    else {
        if (buff) {
            cv::resize(img, *buff, areaSize);
            cv::cvtColor(*buff, area, cv::COLOR_GRAY2BGR);
        }
        else {
            Mat tmp;
            cv::resize(img, tmp, areaSize);
            cv::cvtColor(tmp, area, cv::COLOR_GRAY2BGR);
        }
    }
}

static void
cloneTo(const Mat &src, Mat &dst) {
    if (dst.type() != src.type() || dst.size() != src.size()) {
        dst = src.clone();
    }
    else if (dst.type() == src.type() && src.size() == dst.size()) {
        src.copyTo(dst);
    }
    else ASSERT(0);
}

struct ImageLabel
{
    ImageLabel() {
        status = INIT;
        disabled = false;
        color = 0x202020;
    }

    void reset() {status = INIT; disabled = false;}
    void disable(bool v){disabled = v;}
    void redraw() {status = CHANGED;}
        
    int operator()(Screen &screen, const Mat &img, int x, int y, int w, int h) {
        const Rect roi(x,y,w,h);
        const int s = disabled ? DISABLED : mouseStatus(roi, IDLE);
        if (s != status) {
            status = s; 
            area = screen.bg(roi);
            if (img.empty()) area = toScalar(color);
            else copyTo(img, area, &buff);
        }
        return status;
    }

    int status;
    bool disabled;    
    uint color;
    Mat area, buff;
};

struct CheckBox
{
    CheckBox() {
        status = INIT;
        disabled = false;
        checked = false;
        color = 0x1E2027;
        color_disabled = colorAdd(color, -0x7);
        outer_color = 0x43454C;
        outer_size = 1;
        inner_color = 0x2670AF;
        align = ALIGN_LEFT;
    }

    void reset() {status = INIT; disabled = false;}
    void disable(bool v){disabled = v;}
    void redraw() {status = CHANGED;}

    int operator()(Screen &screen, const string &text, bool &isChecked, int x, int y, int w, int h) {
        const Rect roi(x, y, w, h);
        const int s = disabled ? DISABLED : mouseStatus(roi, IDLE | CLICKED);        
        if (s != status) {
            status = s; 
            if (s == CLICKED) checked = !checked;
            const int boxSize = std::min(w, h) - (1 + outer_size) * 2;
            const Rect outer(1+outer_size, 1+outer_size, boxSize, boxSize);
            const Rect inner(outer.x+3, outer.y+3, outer.width-6, outer.height-6);
            const Rect fontArea(outer.x + outer.width + 3, outer.y, w - outer.width - 4, outer.height);                        
            area = screen.bg(roi);
            area = toScalar(color);  
            rectangle(area, outer, s == DISABLED ? color_disabled : outer_color, outer_size);
            if (checked) fill(area, inner, s == DISABLED ? color_disabled : inner_color);            
            font.putText(area, fontArea, text, s == DISABLED, align);            
        }        
        isChecked = checked;
        return status;
    }
        
    bool checked;
    bool disabled;
    int status;

    Font font;
    uint color;
    uint color_disabled;
    uint outer_color;
    uint inner_color;    
    int outer_size;
    int align;
    Mat area;
};

struct RadioBox : CheckBox
{    
    int operator() (Screen &screen, const string &text, const int uid, int &checkedId, int x, int y, int w, int h) {
        const Rect roi(x,y,w,h);
        const bool cc = checkedId == uid;
        const int s = disabled ? DISABLED : mouseStatus(roi, IDLE|CLICKED);
        if (s != status || cc != checked) {
            status = s; checked = cc;
            if (s == CLICKED) {checkedId = uid;}
            const int boxSize = std::min(w, h) - (1 + outer_size) * 2;
            const Rect outer(1+outer_size, 1+outer_size, boxSize, boxSize);
            const Point center((1+outer_size+boxSize)/2, (1+outer_size+boxSize)/2);
            const int outer_radius = boxSize/2 - 1;
            const int inner_radius = outer_radius - 3;
            const Rect fontArea(outer.x + outer.width + 3, outer.y, w - outer.width - 4, outer.height);            

            area = screen.bg(roi);
            area = toScalar(color);            
            circle(area, center, outer_radius, s == DISABLED ? color_disabled : outer_color, outer_size);
            if (checked) fill(area, center, inner_radius, s == DISABLED ? color_disabled : inner_color);
            font.putText(area, fontArea, text, s == DISABLED, align);            
        }
        return status;
    }  
};

struct RangeBox : CheckBox
{
    RangeBox() : CheckBox() {
        align = ALIGN_CENTER;
    }
    
    int operator()(Screen &screen, float &val, float minval, float maxval, float step,
                   int x, int y, int w, int h) {
        const Rect roi(x, y, w, h);
        const int s = disabled ? DISABLED : mouseStatus(roi, IDLE|PRESSED);
        bool val_changed = false;
        if (s == PRESSED) {
            val += step;
            val = val < minval ? minval : (val > maxval ? minval : val);
            val_changed = true;
        }
                    
        if (s != status || val_changed) {
            status = s;            
            char text[16];
            snprintf(text, 16, "%0.2lf", val);                      
            const Rect outer(1+outer_size, 1+outer_size, w-2*(1+outer_size), h-2*(1+outer_size));
            const Rect inner(outer.x+3, outer.y+3, outer.width-6, outer.height-6);
            const float percent = (val - minval) / (maxval - minval);
            const Rect filled(inner.x, inner.y, inner.width * percent, inner.height);
            
            area = screen.bg(roi);
            area = toScalar(color);            
            rectangle(area, outer, disabled ? color_disabled : outer_color, outer_size); 
            fill(area, filled, disabled ? color_disabled : inner_color);
            font.putText(area, inner, text, s==DISABLED, align);
        }
        return status;
    }
};

struct Line {
    Line() {
        status = INIT;
        disabled = false;
        color = 0x33353C;
        color_disabled = 0x303030;
    }

    void reset() {status = INIT; disabled = false;}
    void disable(bool v){disabled = v;}
    void redraw() {status = CHANGED;}
    
    int operator()(Screen &screen, int thickness, int x0, int y0, int x1, int y1) {
        int s = disabled ? DISABLED : IDLE;
        if (s != status) {
            status = s;
            line(screen.bg, Point(x0, y0), Point(x1, y1), s == DISABLED ? color_disabled : color, thickness);
        }
        return status;
    }
    
    int status;
    uint color;
    uint color_disabled;
    bool disabled;
};

struct Keyboard
{
    Keyboard() {
        color = 0x23252C;
        num_disabled = false;
        char_disabled = false;
        entered = false;
        textLabel.color = colorAdd(color, -10);
        maxLen = 32;
    }
    
    int operator() (Screen &screen, std::string &input, int x, int y, int w, int type=KB_FULL, int maxlen = 32) {
        btnGap = (int)(w * 0.1f * 0.1f);
        btnSize = (int)((w + btnGap) * 0.1f - btnGap);
        const Rect kbroi(x, y, 10*(btnSize + btnGap) - btnGap + 2, 5*(btnSize + btnGap) - 2 * btnGap + 2);
        
        switch(type) {
        case KB_FULL: num_disabled = false; char_disabled = false; break;
        case KB_NUM:  num_disabled = false; char_disabled = true; break;
        case KB_CHAR: num_disabled = true;  char_disabled = false; break;
        default: ASSERT(0); break;
        }
        
        area = screen.bg(kbroi);
        cloneTo(area, prevKBRoiImg);
        area = toScalar(color);
        startX = kbroi.x + 1; startY = kbroi.y + 1;        
        entered = false;
        inputPtr = &input;
        screenPtr = &screen;
        maxLen = maxlen;
        reset();
        
        while (!entered) {
            if (CLICKED == textLabel(screen, input, startX, startY, kbroi.width-2, btnSize)-2) {
                input.clear();
                textLabel.redraw();
            }
                
            keyId = 0;
            K("1");K("2");K("3");K("4");K("5");K("6");K("7");K("8");K("9");K("0");
            K("Q");K("W");K("E");K("R");K("T");K("Y");K("U");K("I");K("O");K("P");
            K("A");K("S");K("D");K("F");K("G");K("H");K("J");K("K");K("L");K("Del");
            K("Z");K("X");K("C");K("V");K("B");K("N");K("M");K("_");K("@");K("En");

            if (27 == screen.show()) {
                break;
            }
        }
        
        copyTo(prevKBRoiImg, area);
        return IDLE;
    }

    void reset() {
        textLabel.reset();
        for (int i = 0; i < 40; ++i) {
            b[i].status = INIT;
            b[i].disabled = false;
        }
    }

private:
    inline void K(const string &text) {
        const int r = keyId / 10, c = keyId % 10;        
#define PUTBUTTON(btn) (btn)(*screenPtr, text,                          \
                             startX + c*(btnSize + btnGap),             \
                             startY + btnSize + r*(btnSize + btnGap), btnSize, btnSize) 
        
        if (keyId != 29 && keyId != 39) {
            b[keyId].disable(keyId < 10 ? num_disabled : char_disabled);
            if (CLICKED == PUTBUTTON(b[keyId]) && inputPtr->size() < maxLen) {
                inputPtr->append(text);
                textLabel.redraw();
            }
        }
        else if (keyId == 29) { // Del
            if (CLICKED == PUTBUTTON(b[keyId])) {
                const int sz = inputPtr->size();
                if (sz > 0) {
                    inputPtr->erase(sz-1);
                    textLabel.redraw();
                }
            }                
        }
        else if (keyId == 39) { // Enter
            if (CLICKED == PUTBUTTON(b[keyId])) {
                entered = true;
            }
        }        
        
        ++keyId;
#undef PUTBUTTON
    }
    
    uint color;
    int startX, startY;
    int btnSize, btnGap;
    bool num_disabled;
    bool char_disabled;
    bool entered;
    Label textLabel;
    Screen *screenPtr;
    string *inputPtr;
    int keyId; 
    Button b[40];
    Mat area, prevKBRoiImg;
    int maxLen;
};

struct MessageBox
{
    MessageBox() {
        status = INIT;
        msgLabel.font.color = 0xE3D567;
        msgLabel.font.scale = 0.7f;
    }

    void reset() {
        status = INIT;
        msgLabel.reset();
        okBtn.reset();
    }

    int operator()(Screen &screen, const string &msg, int x, int y, int w, int h) {
        const Rect roi(x,y,w,h);
        reset();
        area = screen.bg(roi);
        cloneTo(area, prevRoiImg);
        area(Rect(2, 2, w-4, 143)) = toScalar(0x161616);
        while (1) {
            msgLabel(screen, msg, x+3, y+3, w-6, 60);            
            if (okBtn(screen, "OK", x+3, y+80+3, w-6, 60) == CLICKED) break;            
            if (27 == screen.show()) break;
        }        
        copyTo(prevRoiImg, area);
        return 0;
    }

private:
    int status;
    uint color;
    Label msgLabel;
    Button okBtn;
    Mat area, prevRoiImg;
};

} // namespace mui 

#endif /* MUI_H */
