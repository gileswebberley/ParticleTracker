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
    //or for the difference tracking method
    ofxCvContourFinder contours;
    //load the video frame into this so we can check the colour value
    ofxCvColorImage rgbimg;
    //for the finder it must be a grayscale image
    ofxCvGrayscaleImage grayimg, graybg, grayabs;
    //seperated out the blob behaviour
    vector<TrackBlob> trackBlobs;
    //given it a limit as I was struggling to think of a way to keep track tbh
    int maxBlobs{10}, blobCnt{0}, killWait{5};
    //size for the grabber etc
    int grabW{352}, grabH{288};
    //the minimum blob area to be registered in findContours()
    float minBlobArea = (grabW*grabH)*0.003;
    //the maximum blob area to be registered in findContours()
    float maxBlobArea = (grabW*grabH)*0.05;
    //difference threshold for tracking default, set with setUpTrack(int)
    int diff_threshold{100};
    //if using diff method is the background image set
    bool diffbgset{false},diff_mode{false};

public:

    expressionsTrack();
    //constructors that call setupTrack()
    //for a haar cascade tracker
    expressionsTrack(string haarFilepath);
    //for a difference tracker
    expressionsTrack(int diff_thresh);
    //if we have ptrs around we need to clear up after ourselves
    ~expressionsTrack();
    //initialisation function...
    bool setupTrack(string haarFilepath);
    //initialisation for difference method
    bool setupTrack(int diffThreshold);
    //self explanatory?
    bool doFinding();
    //draw each of the trackblobs...
    void drawFindings();
    //to use it just for getting the position
    vector<ofPoint> getCentrePoints();
    //in use I want to get one point that is the closest to the last one
    ofPoint getClosestPoint(ofPoint point);
    //to get the trackblob with the largest area
    ofPoint getLargestPoint();
    //so I can test and tweak see what the camera is seeing
    void drawInput();
    //I failed in making this work as I hoped :(
    ofPoint getGrabWH(){return ofPoint{(float)grabW,(float)grabH};}

private:


};

#endif // EXPRESSIONSTRACK_H
