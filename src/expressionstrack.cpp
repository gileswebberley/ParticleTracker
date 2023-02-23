#include "expressionstrack.h"

/*
 * APr 2021, changing the purpose to just get position info
 */

expressionsTrack::expressionsTrack()
{
    //set the live video device up with inputSelector
    vidIn.setupInput(grabW,grabH);
    setupTrack(diff_threshold);
    cout<<"Constructor of default difference tracker\n";
}

expressionsTrack::expressionsTrack(string haarpath)
{
    cout<<"Constructor of haar cascade tracker\n";
    vidIn.setupInput(grabW,grabH);
    setupTrack(haarpath);
}

expressionsTrack::expressionsTrack(int diff_thresh)
{
    cout<<"Constructor of difference tracker\n";
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
bool expressionsTrack::setupTrack(int diffThreshold)
{
    //a little check that the value is valid
    diff_threshold = (abs(diffThreshold)>254)?254:abs(diffThreshold);
    //allocate and set up use of ofTexture for a bit of optimisation
    grayimg.allocate(grabW,grabH);
    grayimg.setUseTexture(true);
    graybg.allocate(grabW,grabH);
    graybg.setUseTexture(true);
    grayabs.allocate(grabW,grabH);
    grayabs.setUseTexture(true);
    rgbimg.allocate(grabW,grabH);
    rgbimg.setUseTexture(true);
    diff_mode = true;
    trackBlobs.assign(maxBlobs,TrackBlob(grabW,grabH));
    return diff_mode;
}

bool expressionsTrack::setupTrack(string haarpath)
{
    diff_mode = false;
    cout<<"TRACKER SETUP\n";
    finder.setup(haarpath);
    //other variables that you can tweek to make finding better
    //this speeds it up enormously!!! :)
    finder.setScaleHaar(1.3);
    //to reduce false positives this is how many rectangles
    //make up a group big enough to count as a real find
    finder.setNeighbors(3);
    rgbimg.allocate(grabW,grabH);

    grayimg.allocate(grabW,grabH);
    //not drawing so using the texture makes it more efficient I think
    grayimg.setUseTexture(true);
    //bgcolour.a = fadeAmount;
    trackBlobs.assign(maxBlobs,TrackBlob(grabW,grabH));
    return true;
}
void expressionsTrack::videoToGrayImage()
{
    //set using a reference to the pixel data of the video returned from input-selector
    rgbimg.setFromPixels(vidIn.getPixelRead());
    //set up the grayscale image which is compared to graybg
    grayimg.setFromColorImage(rgbimg);
    grayimg.dilate();//01-07-22 trying to get rid of noise
    grayimg.contrastStretch();//trying to make the histogram the same, remove little AE shifts
}

bool expressionsTrack::waitForCameraStart(int wait)
{
    //to give the camera time to go through it's auto-exposure routine
    static int wait_for_camera = 1;
    //this has been added to let the feed from the camera settle as I have made it lock in the background and so track anything that's different from that rather than what's changed between frames
    if (wait_for_camera > wait) {
        videoToGrayImage();
        //set up the background as this first frame after waiting
        graybg = grayimg;
        //graybg.contrastStretch();
        //diffbgset = true;
        cout << "difference background image has been set....\n";
        return true;
    }
    else {
        wait_for_camera++;
        return false;
    }
}
//returns true when there are findings ready to be accessed, if false then it's doing some kind of setup or has failed in some way
bool expressionsTrack::doFinding()
{
    //static counters to keep track of what findings we have at any moment
    //it's 1 so we don't end up trying to access trackBlobs[-1]
    static int maxBlobCnt = 1;
    static int killCnt = 0;
    //check whether input-selector video has a new frame
    if(vidIn.updateInput()){
        //are we using contour finder?
        if(diff_mode){
            //check if bg image has been set
            if(!diffbgset){
                diffbgset = waitForCameraStart(10);
                //there'll be no diff as it's just been set so...
                return false;
            }
            videoToGrayImage();
            grayabs.absDiff(graybg,grayimg);//the difference between the last fram and this one
            //01-07-22 removing to make tracking better for exhibition space
            graybg = grayimg;//set last one as this one for the next loop
            //grayabs.contrastStretch();//adds exposure noise so get rid of?
            grayabs.threshold(diff_threshold);//create a binary(blk or wht) image
            grayabs.blur(5);//blur to get rid of some noise
            contours.findContours(grayabs,minBlobArea,maxBlobArea,maxBlobs,true);//use the openCV contour finder
            blobCnt = contours.blobs.size();//then find the white areas in the difference image
        }
        //if not we must be using Haar finder
        else{
            //all the hard work done for us...
            cerr << "Expressions tracker Haar mode is under development\n";
            //finder.findHaarObjects(grayimg);
            //blobCnt = finder.blobs.size();
        }
        //if we haven't found anything kill off the old track blobs
        if(blobCnt < maxBlobCnt){
            //wait for a bit so it's not twitchy
            killCnt++;
            if(killCnt >= killWait){
                //def. left the tracking...
                trackBlobs.at(maxBlobCnt-1).killTrackBlob();
                killCnt = 0;
                maxBlobCnt -= 1;//this has fixed some blobs not dying
            }
        }else{
            //still finding the same amount of blobs or more
            maxBlobCnt = blobCnt;
            killCnt = 0;
        }
        //make sure it's not trying to follow too many for now
        if(blobCnt > maxBlobs) blobCnt = maxBlobs;
        //cout<<"Tracker has found "<<blobCnt<<" blobs\n";
        for(int i=0; i < blobCnt; i++){
            //find the bounding boxes, centre points, and area to pass on to our track blobs
            ofRectangle bb = (diff_mode)? contours.blobs[i].boundingRect : finder.blobs[i].boundingRect;
            ofPoint cp = (diff_mode)? contours.blobs[i].centroid : finder.blobs[i].centroid;

            //it seems to remain in order so each tracker should be the same object - bit of an assumption!?
            trackBlobs.at(i).updateTrackBlob(cp, bb.width, bb.height);
            //added functionality for tracking the largest blob
            float ar = (diff_mode) ? contours.blobs[i].area : finder.blobs[i].area;
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
    //to make sure it clears when there are no trackblobs
    ofPoint retPt{0,0};
    if(!vp.empty()){
        float dist = point.distance(vp[0]);
        retPt = vp[0];
        for(int i=1; i < vp.size(); i++){
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

//adapt getClosestPoint to track furthest
ofPoint expressionsTrack::getFurthestPoint(ofPoint point)
{
    vector<ofPoint> vp = getCentrePoints();
    //to make sure it clears when there are no trackblobs
    ofPoint retPt{0,0};
    if(!vp.empty()){
        float dist = point.distance(vp[0]);
        retPt = vp[0];
        //it seems that maxBlobs is not the same size as vp[] so use the size() method
        for(int i=1; i < vp.size(); i++){
            if(trackBlobs.at(i).getInit()){
                //simply test for greater distance rather than less
                if(point.distance(vp.at(i)) > dist){
                    dist = point.distance(vp[i]);
                    retPt = vp[i];
                }
            }
        }
    }
    //cout<<"FURTHEST POINT TO :"<<point.x<<"/"<<point.y<<" is "<<retPt.x<<"/"<<retPt.y<<"\n";
    return retPt;
}

ofPoint expressionsTrack::getLargestPoint(){
    float largest{0};
    int index{0};
    for(int i=0; i<trackBlobs.size(); i++){
        //firstly check that the track blob is initialised
        if(trackBlobs.at(i).getInit()){
            float this_area = trackBlobs.at(i).getArea();
            //then compare by area (size)
            if(this_area>largest){
                index = i;
                largest = this_area;
            }
        }
    }
    return trackBlobs.at(index).getCentrePoint();
}

//simply returns a vector of all of the trackBlobs centre points
vector<ofPoint> expressionsTrack::getCentrePoints()
{
    vector<ofPoint> points;
    for(int i=0; i< trackBlobs.size(); i++){
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
