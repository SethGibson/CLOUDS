#pragma once

#include "ofMain.h"
#include "CloudsStoryEngine.h"
#include "CloudsFCPParser.h"
#include "CloudsPlaybackController.h"
#include "CloudsSound.h"

class testApp : public ofBaseApp{
  public:
	void setup();
	void update();
	void draw();
	
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
	
	CloudsFCPParser parser;
	CloudsVisualSystemManager visualSystems;
	CloudsPlaybackController player;
	CloudsStoryEngine storyEngine;
	CloudsSound sound;
	CloudsRun run; //temp
	
	bool useScratch;
	
	ofxTLFlags* soundTrack;
	
	void timelineEventFired(ofxTLBangEventArgs& bang);
	void audioRequested(float * output, int bufferSize, int nChannels);
};
