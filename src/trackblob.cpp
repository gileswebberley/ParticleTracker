#include "trackblob.h"
/*
 * This was from the Expressions project but it served a different
 * purpose so I'm going to rip out all the stuff I'm not using here
 */

TrackBlob::TrackBlob()
{/*...*/}

TrackBlob::TrackBlob(int w, int h):worldW{w},worldH{h}
{
    screenWidth = ofGetWidth();
    screenHeight = ofGetHeight();
}

TrackBlob::~TrackBlob()
{/*...*/}

void TrackBlob::updateTrackBlob(ofPoint p, int w = 1, int h = 1)
{
    if(dying){
        killTrackBlob();
        //gracefulKill();
        return;
    }
    if(!getInit()){
        //cout<<"This track blob is ALIVE!!....\n";
        is_init = true;
        //refresh the die time
        dyingDuration = 0;
    }
    //if it's hardly moved save on the processing...
    if(rawPos.distance(p)<minDist)return;
    rawPos = p;
    wBounding = w;
    hBounding = h;
    pos = p;
    //reverse xcoord so that it seems to follow if camera is straight on
    pos.x = worldW-pos.x;
    //believe that ofGetWidth/Height are quite efficient but perhaps better to have as a local var on birth?
    pos.x *= screenWidth/(float)worldW;
    pos.y *= screenHeight/(float)worldH;
}

bool TrackBlob::getInit()
{
    if(dying) gracefulKill();
    return is_init;
}

void TrackBlob::killTrackBlob()
{
    if(is_init){
        //cout<<"Killing a track blob...\n";
        dying = true;
    }//else
    if(dying){
        gracefulKill();
        //is_init = false;
    }
}

void TrackBlob::gracefulKill()
{
    //static int dyingDuration = 0;
    if(dyingTime > dyingDuration){
//        cout<<"dying....\n";
        ++dyingDuration;
    }else{
        //cout<<"blob is dead\n\n";
        dyingDuration = 0;
        pos = {0,0};
        rawPos = pos;
        dying = false;
        is_init = false;
    }
}





