#ifndef TRACKBLOB_H
#define TRACKBLOB_H

#include "ofMain.h"

/*
 * apr 2021, stripping back the way I'm using this vs Expressions project
 */
class TrackBlob
{

    int wBounding, hBounding, minDist{10}, worldW, worldH, dyingTime{30},dyingDuration{0};
    //flag so it can know what state it's in
    bool is_init{false}, dying{false};
    //rawPos is not processed so will be the actual position in the video feed
    ofPoint pos{0,0}, rawPos{0,0};

public:
    TrackBlob();
    TrackBlob(int w, int h);
    ~TrackBlob();
    void updateTrackBlob(ofPoint p, int w, int h);
    void killTrackBlob();
    bool getInit();
    void gracefulKill();
    ofPoint getCentrePoint(){return pos;}
    ofPoint getRawCentrePoint(){return rawPos;}
    ofPoint getBoundingBox(){return ofPoint(wBounding,hBounding);}
};

#endif // TRACKBLOB_H
