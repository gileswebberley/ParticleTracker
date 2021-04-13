#include "expressionstrack.h"

/*
 * APr 2021, changing the purpose to just get position info
 */

expressionsTrack::expressionsTrack()
{
    //set the live video device up with inputSelector
    vidIn.setupInput(grabW,grabH);
    cout<<"Constructor of tracker\n";
}

expressionsTrack::expressionsTrack(string haarpath)
{
    cout<<"Constructor of tracker\n";
    vidIn.setupInput(grabW,grabH);
    setupTrack(haarpath);
}

expressionsTrack::~expressionsTrack()
{
    //    for(ofImage* im : bodyBoxes){
    //        delete im;
    //    }
}

//setup function for difference tracking method
bool expressionsTrack::setupTrack(int diffThreshold)
{
    diff_threshold = (abs(diffThreshold)>254)?254:abs(diffThreshold);
    grayimg.allocate(grabW,grabH);
    graybg.allocate(grabW,grabH);
    grayabs.allocate(grabW,grabH);
    diff_mode = true;
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
    bgcolour.a = fadeAmount;
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
        grayimg.setFromPixels(vidIn.getPixelRead());
        if(!diffbgset && diff_mode){
            graybg = grayimg;
            diffbgset = true;
            cout<<"difference background image has been set....\n";
            //there'll be no diff as it's just been set so...
            return false;
        }
        if(diff_mode){
            grayabs.absDiff(graybg,grayimg);
            grayabs.threshold(diff_threshold);
            contours.findContours(grayabs,10,10000,maxBlobs,false);
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
                maxBlobCnt = blobCnt;
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
            ofRectangle bb = finder.blobs[i].boundingRect;
            //it seems to remain in order so each tracker should be the same object
            trackBlobs.at(i).updateTrackBlob(finder.blobs[i].centroid, bb.width, bb.height);
        }
        return true;
    }
    return false;
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
