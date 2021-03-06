#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	
	ofSetVerticalSync(true);
	ofSetFrameRate(60);
	
	ofToggleFullscreen();
    ofEnableSmoothing();
	ofBackground(0,0,0);

	parser.loadFromFiles();
	

	
	hud.setup();
}

//--------------------------------------------------------------
void testApp::update(){
	hud.update();
}

//--------------------------------------------------------------
void testApp::draw(){

	ofDrawBitmapString(ofToString(ofGetFrameRate()), 20,20);
	hud.draw();
}

//--------------------------------------------------------------
void testApp::exit(){
	hud.saveGuiSettings();
}

//--------------------------------------------------------------
void testApp::guiEvent(ofxUIEventArgs &e){
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    
	if(key == 'C'){
		hud.respondToClip( parser.getRandomClip() );
	}
	
	if(key == 'E'){
		vector<int> projectExampleIndecs;
		for(int i = 0; i < parser.getAllClips().size(); i++){
			if(parser.getAllClips()[i].hasProjectExample){
				projectExampleIndecs.push_back(i);
			}
		}
		
		if(projectExampleIndecs.size() > 0){
			int exampleIndex = projectExampleIndecs[ ofRandom(projectExampleIndecs.size()) ];
			hud.respondToClip( parser.getAllClips()[exampleIndex] );
			cout << "SENT CLIP " << parser.getAllClips()[exampleIndex].getLinkName() << " WITH EXAMPLE " << parser.getAllClips()[exampleIndex].projectExampleTitle << endl;
		}
	}
	
	if(key == ' '){
		hud.animateOn(CLOUDS_HUD_FULL);
	}
    
    if(key == '1'){
		hud.animateOn(CLOUDS_HUD_LOWER_THIRD);
	}
    
    if(key == '2'){
		hud.animateOn(CLOUDS_HUD_QUESTION);
	}
    
    if(key == '3'){
		hud.animateOn(CLOUDS_HUD_MAP);
	}
    
    if(key == '4'){
		hud.animateOn(CLOUDS_HUD_PROJECT_EXAMPLE);
	}
    

	
    if(key == 'h'){
		hud.toggleGuis();
//        storyEngine.toggleGuis();
    }
	
    else if(key =='f'){
        ofToggleFullscreen();
    }
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}