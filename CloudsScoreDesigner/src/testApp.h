#pragma once

#include "ofMain.h"
#include "CloudsStoryEngine.h"
#include "CloudsFCPParser.h"
#include "ofxUI.h"
#include "CloudsAct.h"
#include "CloudsSound.h"
#include "CloudsRGBDVideoPlayer.h"

class testApp : public ofBaseApp{
  public:
	void setup();
	void update();
	void draw();
    void exit();
	
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
	CloudsStoryEngine storyEngine;
	CloudsAct* currentAct;
	CloudsRun run;
	CloudsSound sound;
	CloudsRGBDVideoPlayer player;
//	CloudsWebSocketConnection websockets;
	
	//clip events
	void actCreated(CloudsActEventArgs& args);
	void actBegan(CloudsActEventArgs& args);
	void actEnded(CloudsActEventArgs& args);
	void clipBegan(CloudsClipEventArgs& args);
	void visualSystemBegan(CloudsVisualSystemEventArgs& args);
	void visualSystemEnded(CloudsVisualSystemEventArgs& args);
    void questionAsked(CloudsQuestionEventArgs& args);
	void topicChanged(CloudsTopicEventArgs& newTopic);
    void preRollRequested(CloudsPreRollEventArgs& clip);
		
	bool rebuildAct;
    ofxUISuperCanvas *gui;
    ofxUISuperCanvas *clipGui;
    bool displayGui;
	
    void audioRequested(float * output, int bufferSize, int nChannels);
    
    void guiEvent(ofxUIEventArgs &e);
    
    bool withVideo;

};
