#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){
    //ofSetupOpenGL(1024,768,OF_FULLSCREEN);			// <-------- setup the GL context
    ofGLWindowSettings settings;
//    settings.setGLVersion(2, 1);  // Fixed pipeline
    settings.setGLVersion(3, 2);  // Programmable pipeline
    settings.setSize(1920,1080);
    settings.windowMode = OF_FULLSCREEN;
    ofCreateWindow(settings);
    if(!ofGLCheckExtension("GL_ARB_geometry_shader4") && !ofGLCheckExtension("GL_EXT_geometry_shader4") && !ofIsGLProgrammableRenderer()){
        ofLogFatalError() << "geometry shaders not supported on this graphics card";
        return 1;
    }
    // this kicks off the running of my app
    // can be OF_WINDOW or OF_FULLSCREEN
    // pass in width and height too:
    ofRunApp(new ofApp());

}
