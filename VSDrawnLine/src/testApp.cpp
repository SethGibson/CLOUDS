#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	ofSetVerticalSync(true);
	
	drawnLine.setup();
	drawnLine.playSystem();
}


//--------------------------------------------------------------
void testApp::update(){

    
    //cout << ofGetFrameRate() << endl;
}


//--------------------------------------------------------------
void testApp::draw(){
	
    
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
	if(key == 'f'){
//		ofToggleFullscreen();
	}
}

//--------------------------------------------------------------
void testApp::exit(){
    drawnLine.exit();
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
