#ifndef EXPRESSIONSTRACK_H
#define EXPRESSIONSTRACK_H

#include "ofxOpenCv.h"
#include "inputselector2.h"
#include "trackblob.h"


class expressionsTrack
{
    //using so it's straight forward to change between cam/file
    InputSelector vidIn{IS_TYPES::VID_DEVICE};
    //setup with the haar cascade that's required (add another for better tracking?)
    ofxCvHaarFinder finder;
    //load the video frame into this so we can check the colour value
    //ofxCvColorImage rgbimg;
    //for the finder it must be a grayscale image
    ofxCvGrayscaleImage grayimg;
    //seperated out the blob behaviour
    vector<TrackBlob> trackBlobs;
    //given it a limit as I was struggling to think of a way to keep track tbh
    int maxBlobs{10}, blobCnt{0}, killWait{30};
    //size for the grabber etc
    int grabW{352}, grabH{288};
    ofColor bgcolour{255,255,255};
    int fadeAmount{60};

public:
    expressionsTrack();
    //constructor that calls setupTrack
    expressionsTrack(string haarFilepath);
    //if we have ptrs around we need to clear up after ourselves
    ~expressionsTrack();
    //initialisation function...
    bool setupTrack(string haarFilepath);
    //self explanatory?
    bool doFinding();
    //draw each of the trackblobs...
    void drawFindings();
    //to use it just for getting the position
    vector<ofPoint> getCentrePoints();
    //so I can test and tweak see what the camera is seeing
    void drawInput();
    //I failed in making this work as I hoped :(
    ofPoint getGrabWH(){return ofPoint{(float)grabW,(float)grabH};}
};

#endif // EXPRESSIONSTRACK_H
