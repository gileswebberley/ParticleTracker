#ifndef TRACKBLOB_H
#define TRACKBLOB_H

#include "ofMain.h"

/*
 * apr 2021, stripping back the way I'm using this vs Expressions project
 */
class TrackBlob
{
    //dyingTime is for tracking smoothing, this relates to the number of frames
    int wBounding, hBounding, minDist{10}, worldW, worldH, dyingTime{30},dyingDuration{0};
    //to save grabbing screen width and height each update call
    float screenWidth, screenHeight;
    //flag so it can know what state it's in
    bool is_init{false}, dying{false};
    //rawPos is not processed so will be the actual position in the video feed
    ofPoint pos{0,0}, rawPos{0,0};
    float tb_area{0.0f};

public:
    TrackBlob();
    TrackBlob(int w, int h);
    ~TrackBlob();
    void updateTrackBlob(ofPoint p, int w, int h);
    void setArea(float a){tb_area = a;}
    float getArea(){return tb_area;}
    void killTrackBlob();
    bool getInit();
    void gracefulKill();
    ofPoint getCentrePoint(){return pos;}
    ofPoint getRawCentrePoint(){return rawPos;}
    ofPoint getBoundingBox(){return ofPoint(wBounding,hBounding);}
};

#endif // TRACKBLOB_H
