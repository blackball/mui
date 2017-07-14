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
 **  Summary :  Frameless minimum image window library based on Xlib.
 **  Author  :  Hui Li (bugway / gmail.com)
 **  Created :  2017-07-01
 **  Notes   :  This is library is created to make the UI behaviors under
 **             control while reducing memory and squeezing performance as
 **             much as possible.
 **
 **  TODO    :  Combined with https://github.com/blackball/gui to support
 **             Windows system.
 **
 ***********************************************************************/

#ifndef SUI_H
#define SUI_H

#ifdef __cplusplus
extern "C" {
#endif 

typedef struct Sui Sui;
/**
 *  \brief create a Sui object
 *
 *  \param w width of the window
 *  \param h height of the window
 *  \param mode 1 --> full screen or 0 --> not full screen 
 *  \return return valid pointer of NULL 
 */
Sui* sui_create(int w, int h, int mode);

/**
 *  \brief move window to a new position relative to the root window 
 *
 *  \param ui a valid pointer, if it's NULL, will do nothing
 *  \param nx new x position, should be non-negative 
 *  \param ny new y position, should be non-negative
 *  \return return 0 if OK, else -1 
 */
int  sui_move(Sui *ui, int nx, int ny);

/**
 *  \brief resize window
 *
 *  \param ui a valid pointer, if it's NULL, will do nothing
 *  \param nw new window width
 *  \param nh new window height 
 *  \return return 0 if OK, else -1
 */
int sui_resize(Sui *ui, int nw, int nh);
    
/**
 *  \brief sui callback function type
 *
 *  \param etype event type
 *  \param x x coordinate of mouse cursor
 *  \param y y coordinate of mouse cursor
 *  \param flag extra, not being used now 
 *  \return return void 
 */
typedef void (* sui_callback)(int etype, int x, int y, int flag, void *d);

/**
 *  \brief set the mouse callback to handle mouse events 
 *
 *  \param ui valid pointer, if it's NULL, will do nothing
 *  \param cb callback function pointer, if it's NULL, will use the one with default settings
 *  \param d data pointer used to pass into callback 
 *  \return return 0 if OK, else -1
 */
int  sui_setcallback(Sui *ui, void (* cb)(int, int, int, int, void *), void *d);

/**
 *  \brief display the window 
 *
 *  send the image for displaying 
 *
 *  \param ui valid pointer, if it's NULL, will do nothing
 *  \param imgdata pointer to the real image data
 *  \param w width of the image
 *  \param height of the image
 *  \param ws widthstep of the image
 *  \param cn number of channels of the image 
 *  \return return 0 if OK, else -1
 */
int  sui_show(Sui *ui, const unsigned char *imgdata, int w, int h, int ws, int cn);

/**
 *  \brief wait certen ms while handling each event 
 *
 *  if ms <= 0, it will return immediately after the first key event, else wait
 *
 *  \param ui a valid pointer, if it's NULL, will do nothing 
 *  \return return ascii code of the key during the waiting, else -1
 */
int  sui_wait(Sui *ui, int ms);

/**
 *  \brief release resources, set the pointer to zero 
 *
 *  \param pp pointer to a valid pointer 
 *  \return return 0 if OK, else -1
 */
int  sui_destroy(Sui **pp);


#ifdef __cplusplus
}
#endif 

#endif /* SUI_H */
