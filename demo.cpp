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
 **  Summary :  demostrate how to use Mui.
 **  Author  :  Hui Li (bugway / gmail.com)
 **  Created :  2017-07-08
 **
 ***********************************************************************/

#include "mui.h"

struct App
{
    enum {QUIT, HOME};    
    App() {
        status = HOME;
        range = 34.f; // default value
        radioId = 1;
    }
    
    int status;
    std::string name;
    float range;
    bool boxChecked;
    int radioId;
};

struct UI
{
    UI() {
        screen.init(850, 550);        
        dog = cv::imread("dog.png", 1);                
        screen.move(50, 50);    
        label_mui.font.scale = 2.4f;
        label_mui.disable(true); 
    }
    
    void home(App &app) {        
        std::string name = "Enter your name!!";
        while (1) {
            label(screen, "I am Label, there is a dog!", 30, 30, dog.cols, 30);
            imglabel(screen, dog, 30, 100, dog.cols, dog.rows);        
            if (mui::PRESSED == btn_darkit(screen, "Press me to dark the dog!", 30, 300, dog.cols, 30)) {
                dog -= cv::Scalar(1,1,1);
                imglabel.redraw(); 
            }
        
            if (mui::PRESSED == btn_lightit(screen, "Press me to light up the dog!", 30, 340, dog.cols, 30)) {
                dog += cv::Scalar(1,1,1);
                imglabel.redraw();
            }

            if (mui::CLICKED == label_edit(screen, name, 30, 380, dog.cols, 30)) {
                std::string userInput;
                if (name != "Enter your name!!") userInput = name;
                keyboard(screen, userInput, 445, 300, 400, mui::KB_CHAR);
                if (userInput.size() > 0) {
                    app.name = userInput;
                    name = userInput;
                    label_edit.redraw();
                }
            }
        
            rangebox(screen, app.range, 0.f, 100.f, 1.0f, 30, 420, dog.cols, 25);  
            checkbox_one(screen, "One", 330, 300, 100, 25); app.boxChecked = checkbox_one.checked;
            radiobox_male  (screen, app.radioId, 1, "Male",   330, 340, 100, 25); 
            radiobox_female(screen, app.radioId, 2, "Female", 330, 380, 100, 25);
            radiobox_animal(screen, app.radioId, 3, "Animal", 330, 420, 100, 25);
        
            lineh(screen, 1, 5, 535, 450, 535);
            linev(screen, 1, 440, 5, 440, 545);        
            label_mui(screen, "MUI", 445, 10, 400, 100);

            if (mui::CLICKED == btn_exit(screen, "Exit",    330, 480, 80, 30)) {
                app.status = App::QUIT;
                break;
            }
                        
            if (27 == screen.show()) {
                app.status = App::QUIT;
                break;
            }
        }
    }

    cv::Mat dog;
    mui::Screen screen;
    mui::Button btn_exit, btn_darkit, btn_lightit;
    mui::Label label, label_edit, label_mui;
    mui::ImageLabel imglabel;
    mui::CheckBox checkbox_one;
    mui::RadioBox radiobox_male;
    mui::RadioBox radiobox_female;
    mui::RadioBox radiobox_animal;
    mui::RangeBox rangebox;
    mui::Line lineh, linev;
    mui::Keyboard keyboard;    
};


int
main(int argc, char *argv[]) {
    UI ui;
    App app;
    while (1) {
        switch(app.status) {
        case App::QUIT: return 0;
        case App::HOME:  ui.home(app); break;
        }
    }
    return 0;
}
