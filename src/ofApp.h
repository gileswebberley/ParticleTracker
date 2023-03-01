#pragma once

#include "ofMain.h"
#include "expressionstrack.h"

//add in the particle system...thanks as always to the openframeworks community for providing this template
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

//because I want to add in a third type of tracking I think I need to make an enum class like I made for the InputSelector, then call the appropriate method
enum class TRACK_TYPE{
    CLOSEST, FURTHEST, LARGEST
};


class ofApp : public ofBaseApp{


public:
    void setup();
    void update();
    void draw();
    //exit added to delete expressionsTrack pointer
    void exit();

    void keyPressed(int key);

private:
    
    //the target tracker, create new in setup()
    expressionsTrack* tracker;
    //int grabW{640}, grabH{480};
    bool draw_delay{true};
    //replaced by TRACK_TYPE, track_largest{false};
    //track_closest is default when this is false
    TRACK_TYPE track_flag{TRACK_TYPE::LARGEST};
    //don't need to track every frame to be responsive
    int track_delay_time{3};
    //alpha channel for controlling the fade
    const float fade_bg_amount{125};
    //set higher for low contrast images [0..255]
    int difference_threshold{30};

    //add in the particle system...
    //original positions
    ofTexture original_pos;
    //the starting image
    ofImage reference;
    string referenceFile{"yellow-bird.jpg"};
    //the connections to the gpu
    ofShader    updatePos;
    ofShader    updateVel;
    ofShader    updateRender;    
    //our two ping pong FBOs
    pingPongBuffer posPingPong;
    pingPongBuffer velPingPong;
    //the FBO we draw in to
    ofFbo   renderFBO;
    //the image that each particle has
    ofImage sparkImg;
    string imageFile{"droplet2.png"};
    float   timeStep,time0;
 
    ofColor bgColour{40,40,40,fade_bg_amount};
    //width and height of the window set in setup
    int     width, height;
    //width and height of the particle image
    int     imgWidth, imgHeight;
    //total number of particles in the system
    int     numParticles{250000};
    //the bigger the particle the slower it runs
    float   particleSize{2.0f};

    //These are the settings that will make it 'feel' nice and produce different reactions to being given a target vector
    //elasticity of each particles behaviour (max and min)
    float elasMin{0.02},elasMax{0.6};
    //maximum start velocity and then min and max resistance
    float velScale{0.10},resistMin{0.005},resistMax{0.8};

    int     textureRes;

    //finally the mesh that the shaders control (so, all of the points, or particles)
    ofVboMesh mesh;

};
