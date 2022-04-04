#include "expressionstrack.h"

/*
 * APr 2021, changing the purpose to just get position info
 */

expressionsTrack::expressionsTrack()
{
    //set the live video device up with inputSelector
    vidIn.setupInput(grabW,grabH);
    cout<<"Constructor of difference tracker\n";
}

expressionsTrack::expressionsTrack(string haarpath)
{
    cout<<"Constructor of haar cascade tracker\n";
    vidIn.setupInput(grabW,grabH);
    setupTrack(haarpath);
}

expressionsTrack::expressionsTrack(int diff_thresh)
{
    cout<<"Constructor of haar cascade tracker\n";
    vidIn.setupInput(grabW,grabH);
    setupTrack(diff_thresh);
}

expressionsTrack::~expressionsTrack()
{
    //    for(ofImage* im : bodyBoxes){
    //        delete im;
    //    }
}

//setup function for difference tracking method
bool expressionsTrack::setupTrack(int diffThreshold = 128)
{
    diff_threshold = (abs(diffThreshold)>254)?254:abs(diffThreshold);
    grayimg.allocate(grabW,grabH);
    graybg.allocate(grabW,grabH);
    grayabs.allocate(grabW,grabH);
    diff_mode = true;
    trackBlobs.assign(maxBlobs,TrackBlob(grabW,grabH));
    return diff_mode;
}

bool expressionsTrack::setupTrack(string haarpath)
{
    cout<<"TRACKER SETUP\n";
    finder.setup(haarpath);
    //other variables that you can tweek to make finding better
    //this speeds it up enormously!!! :)
    finder.setScaleHaar(1.3);
    //to reduce false positives this is how many rectangles
    //make up a group big enough to count as a real find
    finder.setNeighbors(3);

    grayimg.allocate(grabW,grabH);
    //not drawing so using the texture makes it more efficient I think
    grayimg.setUseTexture(true);
    //bgcolour.a = fadeAmount;
    trackBlobs.assign(maxBlobs,TrackBlob(grabW,grabH));
    return true;
}

bool expressionsTrack::doFinding()
{
    //it's 1 so we don't end up trying to access trackBlobs[-1]
    static int maxBlobCnt = 1;
    static int killCnt = 0;
    if(vidIn.updateInput()){
        //set using a reference to the pixel data of the video
        rgbimg.setFromPixels(vidIn.getPixelRead());
        grayimg.setFromColorImage(rgbimg);
        grayimg.contrastStretch();
        if(diff_mode){
            if(!diffbgset){
                graybg = grayimg;
                diffbgset = true;
                cout<<"difference background image has been set....\n";
                //there'll be no diff as it's just been set so...
                return false;
            }
            grayabs.absDiff(graybg,grayimg);//the difference between the last fram and this one
            graybg = grayimg;//set last one as this one for the next loop
            grayabs.contrastStretch();//adds exposure noise so get rid of?
            grayabs.threshold(diff_threshold);//create a binary(blk or wht) image
            grayabs.blur(5);//blur to get rid of some noise
            contours.findContours(grayabs,minBlobArea,maxBlobArea,maxBlobs,false);//then find the white areas in the difference image
            blobCnt = contours.blobs.size();
        }else{
            //all the hard work done for us...
            finder.findHaarObjects(grayimg);
            blobCnt = finder.blobs.size();
        }
        //if we haven't found anything then give up this time
        if(blobCnt < maxBlobCnt){
            //wait for a bit so it's not twitchy
            killCnt++;
            if(killCnt >= killWait){
                //def. left the tracking...
                trackBlobs.at(maxBlobCnt-1).killTrackBlob();
                killCnt = 0;
                maxBlobCnt -= 1;//this has fixed some blobs not dying
            }
            //return false;
        }else{
            //still finding the same amount of blobs...
            maxBlobCnt = blobCnt;
            killCnt = 0;
        }
        //make sure it's not trying to follow too many for now
        if(blobCnt > maxBlobs) blobCnt = maxBlobs;
        //cout<<"Tracker has found "<<blobCnt<<" blobs\n";
        for(int i=0; i < blobCnt; i++){
            //cout<<"blob #"<<i<<"\n";
            ofRectangle bb = (diff_mode)? contours.blobs[i].boundingRect:finder.blobs[i].boundingRect;
            ofPoint cp = (diff_mode)? contours.blobs[i].centroid : finder.blobs[i].centroid;
            float ar =(diff_mode)?contours.blobs[i].area : finder.blobs[i].area;
            //it seems to remain in order so each tracker should be the same object
            trackBlobs.at(i).updateTrackBlob(cp, bb.width, bb.height);
            //added functionality for tracking the largest blob
            trackBlobs.at(i).setArea(ar);
        }
        return true;
    }
    return false;
}

//add in a getClosestPoint(point) method so it just tracks the closest to point
ofPoint expressionsTrack::getClosestPoint(ofPoint point)
{
    vector<ofPoint> vp = getCentrePoints();
    ofPoint retPt = point;
    if(!vp.empty()){
        float dist = point.distance(vp[0]);
        retPt = vp[0];
        for(int i=1; i<maxBlobs; i++){
            if(trackBlobs.at(i).getInit()){
                if(point.distance(vp[i])<dist){
                    dist = point.distance(vp[i]);
                    retPt = vp[i];
                }
            }
        }
    }
    //cout<<"CLOSEST POINT TO :"<<point.x<<"/"<<point.y<<" is "<<retPt.x<<"/"<<retPt.y<<"\n";
    return retPt;
}

ofPoint expressionsTrack::getLargestPoint(){
    float largest{0};
    int index{0};
    for(int i=0; i<maxBlobs; i++){
        if(trackBlobs.at(i).getInit()){
            float this_area = trackBlobs.at(i).getArea();
            if(this_area>largest){
                index = i;
                largest = this_area;
            }
        }
    }
    return trackBlobs.at(index).getCentrePoint();
}

vector<ofPoint> expressionsTrack::getCentrePoints()
{
    vector<ofPoint> points;
    for(int i=0; i<maxBlobs; i++){
        if(trackBlobs.at(i).getInit()){
            points.push_back(trackBlobs.at(i).getCentrePoint());
        }
    }
    return points;
}

void expressionsTrack::drawInput()
{
    vidIn.drawInput();
}

void expressionsTrack::drawFindings()
{
    drawInput();
    grayabs.draw(0,0);
    ofNoFill();
    ofSetColor(0,255,0);//classic computer green :)
    for(int i=0; i<maxBlobs; i++){
        if(trackBlobs.at(i).getInit()){
            //the position in the video feed
            ofPoint tc = trackBlobs.at(i).getRawCentrePoint();
            //and the bounding box width(x) and height(y)
            ofPoint tb = trackBlobs.at(i).getBoundingBox();
            //and finally draw the rectangle onto the video
            ofDrawRectangle(tc.x-(tb.x/2),tc.y-(tb.y/2),tb.x,tb.y);
            ofDrawBitmapStringHighlight(to_string(i),tc.x,tc.y);
        }

    }
    //reset everything so it doesn't effect drawing done by any other processes
    ofFill();
    ofSetColor(255);
}
