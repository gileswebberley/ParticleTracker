#pragma once

#include "ofMain.h"
#include "expressionstrack.h"

//add in the particle system...
struct pingPongBuffer {
public:
    void allocate( int _width, int _height, int _internalformat = GL_RGBA, int mag_filter = GL_NEAREST){
        // Allocate
        for(int i = 0; i < 2; i++){
            FBOs[i].allocate(_width,_height, _internalformat );
            //essentially these buffers are scaled up so the second
            //param (NEAREST/LINEAR) makes it more or less uniform behaviour
            FBOs[i].getTexture().setTextureMinMagFilter(GL_NEAREST, mag_filter);
        }

        //Assign
        src = &FBOs[0];
        dst = &FBOs[1];

        // Clean
        clear();
    }

    void swap(){
        std::swap(src,dst);
    }

    void clear(){
        for(int i = 0; i < 2; i++){
            FBOs[i].begin();
            ofClear(0,255);
            FBOs[i].end();
        }
    }

    ofFbo& operator[]( int n ){ return FBOs[n];}
    ofFbo   *src;       // Source       ->  Ping
    ofFbo   *dst;       // Destination  ->  Pong

private:
    ofFbo   FBOs[2];    // Real addresses of ping/pong FBO«s
};


class ofApp : public ofBaseApp{

    //int grabW{640}, grabH{480};
    bool draw_delay{true};
    int track_delay_time{5};
    int difference_threshold{128};

public:
    //create new in setup()
    expressionsTrack* tracker;
    void setup();
    void update();
    void draw();

    void keyPressed(int key);

    //add in the particle system...
    ofShader    updatePos;
    ofShader    updateVel;
    ofShader    updateRender;

    pingPongBuffer posPingPong;
    pingPongBuffer velPingPong;

    ofFbo   renderFBO;

    ofImage sparkImg;

    float   timeStep,time0;
    float   particleSize;

    int     width, height;
    int     imgWidth, imgHeight;
    int     numParticles;
    int     textureRes;


    ofVboMesh mesh;

};
