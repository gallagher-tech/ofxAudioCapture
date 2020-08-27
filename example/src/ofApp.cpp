#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
	ofBackground( 50 );

	ofxAudioCapture::printDeviceList();
	ofSoundStream soundStream;
	auto devices = soundStream.getDeviceList( ofSoundDevice::Api::MS_DS );

	// audio capture
	ofSoundStreamSettings soundSettings;
	soundSettings.setInDevice( devices[0] );
	soundSettings.sampleRate        = 48000;
	soundSettings.numOutputChannels = 0;
	soundSettings.numInputChannels  = 1;
	soundSettings.bufferSize        = 512;
	soundSettings.numBuffers        = 4;
	audioCapture.setupAudioInputRecorder( soundSettings );
}

//--------------------------------------------------------------
void ofApp::update()
{
}

//--------------------------------------------------------------
void ofApp::draw()
{
	// recording / volume indicator
	ofSetColor( isRecording ? ofColor::red : ofColor::green );

	float radius = 50;

	// volume adjusts radius
	auto volumes = audioCapture.getVolumeSmoothed();
	if ( volumes.size() ) {
		radius = ofMap( volumes[0], 0, 0.07, 50, 500 );
	}

	ofDrawCircle( ofGetWindowSize() * .5, radius );

	ofSetColor( 255 );

	if ( !lastCapturePath.empty() ) {
		string text = isRecording ? "Current audio capture:\n" : "Last audio capture:\n";
		text += lastCapturePath;
		ofDrawBitmapStringHighlight( text, 20, 20 );
	}
}

//--------------------------------------------------------------
void ofApp::toggleCapture()
{

	if ( !isRecording ) {
		lastCapturePath = ofGetTimestampString() + ".wav";			// unique wav file name
		lastCapturePath = ofToDataPath( lastCapturePath, true );	// absolute path to bin/data/file.wav

		audioCapture.beginRecording( lastCapturePath );
		isRecording = true;
	} else {
		audioCapture.endRecording();
		isRecording = false;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased( int key )
{
	if ( key == ' ' ) {
		toggleCapture();
	}
}