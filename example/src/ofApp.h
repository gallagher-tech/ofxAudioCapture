#pragma once

#include "ofMain.h"
#include "ofxAudioCapture.h"

class ofApp : public ofBaseApp
{

public:
	void setup();
	void update();
	void draw();
	void toggleCapture();

	void keyReleased( int key );

	ofxAudioCapture audioCapture;
	bool isRecording = false;

	string lastCapturePath = "";
};
