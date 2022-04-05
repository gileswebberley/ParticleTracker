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


public:
    void setup();
    void update();
    void draw();

    void keyPressed(int key);

private:
    //the target tracker, create new in setup()
    expressionsTrack* tracker;
    //int grabW{640}, grabH{480};
    bool draw_delay{true}, track_largest{true};//track_closest is default when this is false
    int track_delay_time{5};
    int difference_threshold{148};

    //add in the particle system...
    //original positions
    ofTexture original_pos;
    ofImage reference;
    string referenceFile{"reference_face.jpg"};
    ofShader    updatePos;
    ofShader    updateVel;
    ofShader    updateRender;
    pingPongBuffer posPingPong;
    pingPongBuffer velPingPong;
    ofFbo   renderFBO;
    ofImage sparkImg;
    string imageFile{"droplet2.png"};
    float   timeStep,time0;
    ofColor bgColour{255};
    //width and height of the window set in setup
    int     width, height;
    //width and height of the particle image
    int     imgWidth, imgHeight;
    //total number of particles in the system
    int     numParticles{15000};
    //the bigger the particle the slower it runs
    float   particleSize{5.0f};
    //initial x/y position max
    //float pos0{0.5};
    //elasticity max and min
    float elasMin{0.1},elasMax{0.3};
    //maximum velocity and then min and max resistance
    float velScale{0.2},resistMin{0.3},resistMax{0.99};

    int     textureRes;

    ofVboMesh mesh;

};
