#include "ofApp.h"


//CAREFUL NOT TO USE GIMP OR IMAGE VIEWER AS IT MAKES IT CRASH
//ACTUALLY SOMETHING HAS CHANGED AND MAKES IT ONLY RUN ONCE AFTER STARTUP!!
//--------------------------------------------------------------
void ofApp::setup(){
    cout<<"IN SETUP....\n";
    tracker = new expressionsTrack();
    //to use haar cascades...
    //tracker->setupTrack("haarcascade_upperbody.xml");
    //to use difference mode then set the threshold
    tracker->setupTrack(128);
    //ofSetFrameRate(30);
    ofHideCursor();
    ofSetVerticalSync(true);
    //ofBackground(255);
    ofSetBackgroundAuto(false);
    cout<<"...SETUP COMPLETE\n";

    // add in the particle system and split it out later to make it more
    //controllable/reusable-------------------------------------------------------
    //the bigger the particles the slower it runs
    particleSize = 10.0f;
    time0 = ofGetElapsedTimef();
    numParticles = 5000;

    // Width and Heigth of the windows
    width = ofGetWindowWidth();
    height = ofGetWindowHeight();

    string shadersFolder;
    if(ofIsGLProgrammableRenderer()){
        shadersFolder="shaders_gl3";
        //cout<<"gl3\n";
    }else{
        shadersFolder="shaders";
    }

    // Loading the Shaders
    if(ofIsGLProgrammableRenderer()){
        updatePos.load(shadersFolder+"/passthru.vert", shadersFolder+"/posUpdate.frag");// shader for updating the texture that store the particles position on RG channels
        updateVel.load(shadersFolder+"/passthru.vert", shadersFolder+"/velUpdate.frag");// shader for updating the texture that store the particles velocity on RG channels
    }else{
        updatePos.load("",shadersFolder+"/posUpdate.frag");// shader for updating the texture that store the particles position on RG channels
        updateVel.load("",shadersFolder+"/velUpdate.frag");// shader for updating the texture that store the particles velocity on RG channels
    }

    // Frag, Vert and Geo shaders for the rendering process of the spark image
    updateRender.setGeometryInputType(GL_POINTS);
    updateRender.setGeometryOutputType(GL_TRIANGLE_STRIP);
    updateRender.setGeometryOutputCount(6);
    updateRender.load(shadersFolder+"/render.vert",shadersFolder+"/render.frag",shadersFolder+"/render.geom");

    // Seting the textures where the information ( position and velocity ) will be
    textureRes = (int)sqrt((float)numParticles);
    numParticles = textureRes * textureRes;

    // 1. Making arrays of float pixels with position information
    float pos0 = 0.5;
    vector<float> pos(numParticles*3);
    for (int x = 0; x < textureRes; x++){
        for (int y = 0; y < textureRes; y++){
            int i = textureRes * y + x;
            //width and height * offset [0.0...1.0]
            pos[i*3 + 0] = pos0;//ofRandom(pos0); //x*offset;
            pos[i*3 + 1] = pos0;//ofRandom(pos0); //y*offset;
            //try to add elasticity
            pos[i*3 + 2] = ofRandom(0.1,0.5);
        }
    }
    // Load this information in to the FBO's texture
    posPingPong.allocate(textureRes, textureRes, GL_RGB32F);
    posPingPong.src->getTexture().loadData(pos.data(), textureRes, textureRes, GL_RGB);
    posPingPong.dst->getTexture().loadData(pos.data(), textureRes, textureRes, GL_RGB);


    // 2. Making arrays of float pixels with velocity information and the load it to a texture
    float velScale = 0.4;
    vector<float> vel(numParticles*3);
    for (int i = 0; i < numParticles; i++){
        vel[i*3 + 0] = ofRandom(-velScale,velScale);
        vel[i*3 + 1] = ofRandom(-velScale,velScale);
        //try to add resistance
        vel[i*3 + 2] = ofRandom(0.3,0.99);
    }
    // Load this information in to the FBO's texture
    velPingPong.allocate(textureRes, textureRes, GL_RGB32F, GL_LINEAR);
    velPingPong.src->getTexture().loadData(vel.data(), textureRes, textureRes, GL_RGB);
    velPingPong.dst->getTexture().loadData(vel.data(), textureRes, textureRes, GL_RGB);

    // Loading and setings of the variables of the textures of the particles
    sparkImg.load("droplet2.png");
    imgWidth = sparkImg.getWidth();
    imgHeight = sparkImg.getHeight();

    // Allocate the final
    renderFBO.allocate(width, height, GL_RGB32F);
    renderFBO.begin();
    ofClear(0, 0, 0, 255);
    renderFBO.end();


    mesh.setMode(OF_PRIMITIVE_POINTS);
//    glPointSize(10);
    for(int x = 0; x < textureRes; x++){
        for(int y = 0; y < textureRes; y++){

            mesh.addVertex({x,y,0});
            mesh.addTexCoord({x, y});
            mesh.addColor(ofFloatColor(0,ofRandom(0.1,0.8),((y+128)%255)/255.0));
        }
    }

}

//--------------------------------------------------------------
void ofApp::update(){
    //tracker->doFinding();
        static int cntr = 0;
        static ofPoint track0;
        if(cntr < track_delay_time){
            draw_delay = true;
            cntr++;
        }else{
            tracker->doFinding();
            draw_delay = false;
            cntr = 0;
            track0 = tracker->getClosestPoint(track0);
        }
        if(track0.x <= 0) track0.x = mouseX;//2;
        if(track0.y <= 0) track0.y = mouseY;//2;

        //add in the particle system stuff--------------------------------------------
        //start 'recording' the destination buffer
        velPingPong.dst->begin();
        ofClear(0);
        timeStep = ofGetElapsedTimef() - time0;
        time0 = ofGetElapsedTimef();
        //start shader, working on the source buffer
        updateVel.begin();
        // passing the previus velocity information
        updateVel.setUniformTexture("backbuffer", velPingPong.src->getTexture(), 0);
        // passing the position information
        updateVel.setUniformTexture("posData", posPingPong.src->getTexture(), 1);
        updateVel.setUniform1i("resolution", (int)textureRes);
        //pass in the target position
        updateVel.setUniform2f("screen", (float)track0.x/width, (float)track0.y/height);
        updateVel.setUniform1f("timestep", (float)timeStep);

        // draw the source velocity texture to be updated, this is where src is
        //processed by the shader and drawn into the dst fbo
        velPingPong.src->draw(0, 0);

        updateVel.end();
        velPingPong.dst->end();
        //...then make dst fbo into the src fbo by swapping the pointers
        velPingPong.swap();


        // Positions PingPong
        //
        // With the velocity calculated updates the position
        //
        posPingPong.dst->begin();
        ofClear(0);
        updatePos.begin();
        updatePos.setUniformTexture("prevPosData", posPingPong.src->getTexture(), 0); // Previus position
        updatePos.setUniformTexture("velData", velPingPong.src->getTexture(), 1);  // Velocity
        updatePos.setUniform1f("timestep",(float) timeStep );

        // draw the source position texture to be updated by the shaders
        posPingPong.src->draw(0, 0);

        updatePos.end();
        posPingPong.dst->end();

        posPingPong.swap();


        // Rendering
        //
        // 1.   Sending this vertex to the Vertex Shader.
        //      Each one it's draw exactly on the position that match where it's stored on both vel and pos textures
        //      So on the Vertex Shader (that's is first at the pipeline) can search for it information and move it
        //      to it right position.
        // 2.   Once it's in the right place the Geometry Shader make 6 more vertex in order to make a billboard
        // 3.   that then on the Fragment Shader is going to be filled with the pixels of sparkImg texture
        //
        renderFBO.begin();
        ofClear(0,0,0,0);
        updateRender.begin();
        updateRender.setUniformTexture("posTex", posPingPong.dst->getTexture(), 0);
        updateRender.setUniformTexture("sparkTex", sparkImg.getTexture() , 1);
        updateRender.setUniform1i("resolution", (float)textureRes);
        updateRender.setUniform2f("screen", (float)width, (float)height);
        updateRender.setUniform1f("size", (float)particleSize);
        updateRender.setUniform1f("imgWidth", imgWidth);
        updateRender.setUniform1f("imgHeight", imgHeight);

        ofPushStyle();
        ofEnableBlendMode( OF_BLENDMODE_ALPHA );
        //ofSetColor(255);
        //draw the mesh through the rendering shaders
        mesh.drawFaces();

        ofDisableBlendMode();
        glEnd();

        updateRender.end();
        renderFBO.end();
        ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::draw(){
//    ofSetColor(255);
//    /*if(!draw_delay)*/tracker->drawFindings();
    ofBackground(28);
    //now draw the rendered particle system
    renderFBO.draw(0,0);
    tracker->drawInput();//for framing
    tracker->drawFindings();//to see tracking boxes
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}
