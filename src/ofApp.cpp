#include "ofApp.h"


//CAREFUL NOT TO USE GIMP OR IMAGE VIEWER AS IT MAKES IT CRASH
//ACTUALLY SOMETHING HAS CHANGED AND MAKES IT ONLY RUN ONCE AFTER STARTUP!!
//--------------------------------------------------------------
void ofApp::setup(){
    cout<<"IN SETUP....\n";
    //to use difference mode then set the threshold
    tracker = new expressionsTrack(difference_threshold);
    //to use haar cascades...
    //**not functioning!! tracker = new expressionsTrack("haarcascade_upperbody.xml");
    ofHideCursor();
    ofSetVerticalSync(true);
    ofSetBackgroundAuto(true);
    ofSetFrameRate(60);
    // add in the particle system and split it out later to make it more
    //controllable/reusable-------------------------------------------------------
    //time at startup
    time0 = ofGetElapsedTimef();

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
        updatePos.load(shadersFolder+"/passthru.vert", shadersFolder+"/posUpdate.frag");// shader for updating the texture that store the particles position and elasticity on RGB channels
        updateVel.load(shadersFolder+"/passthru.vert", shadersFolder+"/velUpdate.frag");// shader for updating the texture that store the particles velocity and resistance on RGB channels
    }else{
        updatePos.load("",shadersFolder+"/posUpdate.frag");// shader for updating the texture that store the particles position on RG channels and elasticity on the B channel
        updateVel.load("",shadersFolder+"/velUpdate.frag");// shader for updating the texture that store the particles velocity on RG channels and resistance on the B channel
    }

    // Frag, Vert and Geo shaders for the rendering process of the spark image
    updateRender.setGeometryInputType(GL_POINTS);
    updateRender.setGeometryOutputType(GL_TRIANGLE_STRIP);
    updateRender.setGeometryOutputCount(6);
    updateRender.load(shadersFolder+"/render.vert",shadersFolder+"/render.frag",shadersFolder+"/render.geom");

    // Seting the textures where the information ( position and velocity ) will be
    textureRes = (int)sqrt((float)numParticles);
    numParticles = textureRes * textureRes;

    if(!reference.load(referenceFile))cout<<"REFERENCE IMAGE FAILED TO LOAD\n";
    reference.resize(textureRes,textureRes);

    //arrays of floats as pixels with position and elasticity information
    //float pos0 = 0.5;
    vector<float> pos(numParticles*3);
    for (int x = 0; x < textureRes; x++){
        for (int y = 0; y < textureRes; y++){
            int i = textureRes * y + x;
            //width and height * offset [0.0...1.0]
            //now they are positioned as born we can use a reference picture to set the colours, then hopefully make it reform the picture when it has nothing to track??
            pos[i*3 + 0] = (float)x/textureRes;
            pos[i*3 + 1] = (float)y/textureRes;
            //try to add elasticity
            pos[i*3 + 2] = ofRandom(elasMin,elasMax);
            //cout<<"x: "<<pos[i*3+0]<<" y: "<<pos[i*3+1]<<"\n";
        }
    }
    // Load this information in to the FBO's texture
    posPingPong.allocate(textureRes, textureRes, GL_RGB32F, GL_NEAREST);
    posPingPong.src->getTexture().loadData(pos.data(), textureRes, textureRes, GL_RGB);
    posPingPong.dst->getTexture().loadData(pos.data(), textureRes, textureRes, GL_RGB);
    //and save into the original positions texture
    //original_pos.allocate(textureRes, textureRes,GL_RGB32F);
    original_pos.loadData(pos.data(),textureRes,textureRes,GL_RGB);


    // arrays of float pixels with velocity and resistance information
    //float velScale = 0.4;
    vector<float> vel(numParticles*3);
    for (int i = 0; i < numParticles; i++){
        vel[i*3 + 0] = ofRandom(-velScale,velScale);
        vel[i*3 + 1] = ofRandom(-velScale,velScale);
        //try to add resistance
        vel[i*3 + 2] = ofRandom(resistMin,resistMax);
    }
    // Load this information in to the FBO's texture
    velPingPong.allocate(textureRes, textureRes, GL_RGB32F, GL_LINEAR);
    velPingPong.src->getTexture().loadData(vel.data(), textureRes, textureRes, GL_RGB);
    velPingPong.dst->getTexture().loadData(vel.data(), textureRes, textureRes, GL_RGB);

    // Loading and setings of the variables of the textures of the particles
    sparkImg.load(imageFile);
    imgWidth = sparkImg.getWidth();
    imgHeight = sparkImg.getHeight();

    // Allocate the final renderer
    renderFBO.allocate(width, height, GL_RGB32F);
    renderFBO.begin();
    ofClear(0, 0, 0);
    renderFBO.end();

    //Add all of the points to the ofVboMesh object that reflect
    //each of the textures we produced with velocity etc
    mesh.setMode(OF_PRIMITIVE_POINTS);
    for(int x = 0; x < textureRes; x++){
        for(int y = 0; y < textureRes; y++){
            mesh.addVertex({x,y,0});
            mesh.addTexCoord({x, y});
            //this colour is passed through to the shader pipeline, appears as gl_color I think
            ofColor tc =reference.getColor(x,y);
            //cout<<"colour: "<<reference.getColor(x,y)<<"\n";
            mesh.addColor(ofFloatColor(tc.r/255.0,tc.g/255.0,tc.b/255.0,1.0));
        }
    }

    cout<<"...SETUP COMPLETE\n";
}

//---------------------------UPDATE()-----------------------
void ofApp::update(){
    //do the tracking to get a target point
    //use static variables so they persist between calls
    //they must be in the scope of the update() method
    //and initialised inline (ie not in the header file unless const)
    static int cntr{0};
    //static int fade{0};
    static ofPoint track0{0.0f,0.0f};
    if(cntr < track_delay_time){
        draw_delay = true;
        cntr++;
    }else{
        tracker->doFinding();
        draw_delay = false;
        cntr = 0;
        //using the enum type TRACK_TYPE, I'm pretty sure that a switch can be used for an enum but nervous tbh
        switch(track_flag){
        case TRACK_TYPE::CLOSEST:
            track0 = tracker->getClosestPoint(track0);
            break;
        case TRACK_TYPE::FURTHEST:
            track0 = tracker->getFurthestPoint(track0);
            break;
        case TRACK_TYPE::LARGEST:
            track0 = tracker->getLargestPoint();
            break;
        default:
            track0 = tracker->getClosestPoint(track0);
        }

    }
    //for testing without any tracking occuring
    //track0 = ofPoint(0,0);

    //add in the particle system stuff--------------------------------------------
    //start 'recording' the velocity destination buffer
    velPingPong.dst->begin();
    ofClear(0);

    //timestep is passed to the gpu for smooth animation
    timeStep = ofGetElapsedTimef() - time0;
    time0 = ofGetElapsedTimef();

    //start velocity shader, working on the source buffer
    updateVel.begin();

    // passing the previous velocity information
    updateVel.setUniformTexture("backbuffer", velPingPong.src->getTexture(), 0);
    // passing the position information
    updateVel.setUniformTexture("posData", posPingPong.src->getTexture(), 1);
    //trying to implement going back to original positions if nothing to track
    updateVel.setUniformTexture("originalPos", original_pos,2);
    //use setUniform1i to pass in an integer
    updateVel.setUniform1i("resolution", (int)textureRes);
    //pass in the target position as 2 floats, hence the setUniform2f
    updateVel.setUniform2f("screen", (float)track0.x/width, (float)track0.y/height);
    //pass in the current timestep as one float value
    updateVel.setUniform1f("timestep", (float)timeStep);

    // draw the source velocity texture to be updated, this is where src is
    //processed by the shader and drawn into the dst fbo
    velPingPong.src->draw(0, 0);

    //end communication with the shader
    updateVel.end();

    //now the shader has drawn the changes into the "destination" ofFbo
    velPingPong.dst->end();

    //...then make dst fbo into the src fbo by swapping the pointers
    velPingPong.swap();

    //start "recording" the position destination buffer
    posPingPong.dst->begin();
    ofClear(0);

    //start position shader, working with the src buffer
    updatePos.begin();

    //pass in the last position information
    updatePos.setUniformTexture("prevPosData", posPingPong.src->getTexture(), 0);
    //now pass in the velocity information that we just worked out above
    updatePos.setUniformTexture("velData", velPingPong.src->getTexture(), 1);
    //and finally the timestep
    updatePos.setUniform1f("timestep",(float) timeStep );

    // draw the source position texture to be updated by the shaders
    posPingPong.src->draw(0, 0);

    updatePos.end();
    posPingPong.dst->end();

    posPingPong.swap();


    // Rendering--------------------------------------------

    renderFBO.begin();
    //cover the screen with a filled rectangle to produce trails via the alpha channel of bgColour variable
    ofFill();
    ofSetColor(bgColour);
    ofDrawRectangle(0,0,width,height);

    //try to get the colours shifting
    static ofVec3f colour_shift{0,0,0};//xyz = rgb
    colour_shift.x = fmod(colour_shift.x+=ofRandomuf()*(track0.x/width),255);
    colour_shift.y = fmod(colour_shift.y+=ofRandomuf()*(track0.y/height),255);
    colour_shift.z = fmod(colour_shift.z+=ofRandomuf()*(track0.x/width+track0.y/height)/2,255);

    //then update the rendering shader
    updateRender.begin();
    updateRender.setUniformTexture("posTex", posPingPong.src->getTexture(), 0);
    updateRender.setUniformTexture("sparkTex", sparkImg.getTexture() , 1);
    updateRender.setUniform1i("resolution", (float)textureRes);
    updateRender.setUniform2f("screen", (float)width, (float)height);
    updateRender.setUniform1f("size", (float)particleSize);
    updateRender.setUniform1f("imgWidth", imgWidth);
    updateRender.setUniform1f("imgHeight", imgHeight);
    updateRender.setUniform3f("colourShift",colour_shift);

    ofPushStyle();
    ofEnableBlendMode( OF_BLENDMODE_ALPHA );
    //now draw the mesh through the rendering shaders
    mesh.drawFaces();

    ofDisableBlendMode();
    glEnd();

    updateRender.end();
    renderFBO.end();
    ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::draw(){
    //now draw the rendered particle system
    renderFBO.draw(0,0);
    tracker->drawInput();//for framing
    tracker->drawFindings();//to see tracking boxes
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}
