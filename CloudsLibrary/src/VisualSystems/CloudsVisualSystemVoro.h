//
//  CloudsVisualSystemVoro.h
//
//  Created by Patricio Gonzalez Vivo on 5/27/13.
//
//

#pragma once

#include "CloudsVisualSystemRezanator.h"
#include "VoroParticle.h"
#include "Voro.h"

class CloudsVisualSystemVoro : public CloudsVisualSystemRezanator {
public:
    
    string getSystemName();
    
    void selfSetup();
    void selfSetupGuis();
    
    void selfAutoMode();
    void selfUpdate();
    void selfDrawBackground();
    void selfDrawDebug();
    void selfSceneTransformation();
    void selfDraw();
    void selfExit();
    void selfBegin();
	void selfEnd();
    
    void selfKeyPressed(ofKeyEventArgs & args);
    void selfKeyReleased(ofKeyEventArgs & args);
    
    void selfMouseDragged(ofMouseEventArgs& data);
    void selfMouseMoved(ofMouseEventArgs& data);
    void selfMousePressed(ofMouseEventArgs& data);
    void selfMouseReleased(ofMouseEventArgs& data);
    
    void selfSetupGui();
    void selfGuiEvent(ofxUIEventArgs &e);
    
    void selfSetupSystemGui();
    void guiSystemEvent(ofxUIEventArgs &e);
    
    void selfSetupRenderGui();
    void guiRenderEvent(ofxUIEventArgs &e);
    
    void billBoard();
    
protected:
    vector <VoroParticle *> seedParticles;
    vector<ofMesh>  cellMeshes;
    
    ofTexture dot;
    ofVec3f objectLookAt;
    
    float   atractionPct,repulsionPct;
    float   bornRate, growRate;
    float   MaxNumOfParticles, MaxSize;
    float   initialForce;
    float   zMove;
    int     fps;
    bool    bClear;
    bool    bDrawParticles;
    bool    bDrawGlow;
    
    float   containerSize;
    float   containerHeight;
    float   cellsAlpha;
    bool    containerPeriodic;
    bool    bSphere;
    bool    bCyllinder;
    bool    bDrawVoronoi;
    bool    bDrawVoronoiWireFrames;
    
};