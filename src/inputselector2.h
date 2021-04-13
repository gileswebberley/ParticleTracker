#ifndef INPUTSELECTOR_H
#define INPUTSELECTOR_H
#include "ofMain.h"

/*
 * This is the second version, the original is
 * in the Pixelator project
 *
 * ++ I think it would be good to be able to load
 * a folder of images and use them as the input type?
 * rem. to think about what to do with the changing size
 * ? rotate to the original ? resample ? place in a
 * square frame??
 * rem. think about the loading of images - we really only
 * want a few frames in memory at any one time, certainly
 * don't want to load them all up before running! I guess
 * a kinda fifo queue, ie delete them once the've been
 * used?
 * create an isFrameNew() func for it, check if the next
 * one is loaded etc
 */

//just to make things more explicit...
enum class IS_TYPES{
    NO_INPUT, VID_FILE, VID_DEVICE
};

class InputSelector
{    
    ofVideoGrabber vidGrabber;
    ofVideoPlayer vidPlayer;
    ofPixels currentPixels;
    int isWidth, isHeight;
    //+new to allow frame jog as an update method
    bool is_jog{false};
    //bool scaling{false};

    IS_TYPES type_flag;
public:

    InputSelector();
    explicit InputSelector(IS_TYPES type);
    //+new if the default constructor is used
    void setType(IS_TYPES type);
    //no filepath means video grabber
    bool setupInput();
    //use a video player
    //+new use a video player with frame shuttle, default value so no refactoring
    bool setupInput(string filepath, bool shuttle=false);
    //+new only functions with player atm...maybe use for image sequence too
    void toggleJog();
    //+new set the grabber dimensions and deviceId
    bool setupInput(int w, int h, int deviceId=0);
    //call the update() on our IS_TYPE or moves to next frame
    bool updateInput();
    //read the pixels that have been updated
    const ofPixels& getPixelRead();
    //for video players + grabbers
    void drawInput();
    //+new s is whether to scale the video to the window's width
    void drawInput(bool s);
    //
    int getWidth(){return isWidth;}
    int getHeight(){return isHeight;}

};

#endif // INPUTSELECTOR_H
