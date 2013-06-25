//
//  LSystemReconstructor.cpp
//  lSystem
//
//  Created by Patricio Gonzalez Vivo on 5/14/13.
//
//

#include "LSystemReconstructor.h"
LSystemReconstructor::LSystemReconstructor(){
    bGrow = false;
    speed = 1.0;
    aNoise = 0.7;
    tNoise = 0.1;
    bornRandom = 2.0;
    mesh.setMode(OF_PRIMITIVE_LINES);
}

void LSystemReconstructor::setup(LSystem &_lsys, int _deep){
    lines.clear();
    lines.push_back(ofPolyline());
    
    nodes.clear();
    
    _lsys.unoise = aNoise;
    _lsys.utime = tNoise;
    _lsys.make(_deep);
    
    if (_lsys.mesh.getVertices().size() > 2){
        lineTo(_lsys.mesh.getVertices()[0]);
        addNode(_lsys.mesh.getVertices()[0]);
        
        for (int i = 1; i < _lsys.mesh.getVertices().size();i++){
            if ( i%2 == 1){
                lineTo(_lsys.mesh.getVertices()[i]);
            } else {
                if ( _lsys.mesh.getVertices()[i] != _lsys.mesh.getVertices()[i-1]){
                    lines.push_back(ofPolyline());
                    lineTo(_lsys.mesh.getVertices()[i]);
                    addNode(_lsys.mesh.getVertices()[i]);
                }
            }
        }
    }
    
    if (nodes.size() > 0){
        nodes[0].startTime = 0;//1.5;
    }
}

void LSystemReconstructor::lineTo(ofPoint &_pnt){
    lines[ lines.size()-1 ].addVertex(_pnt);
}

void LSystemReconstructor::addNode(ofPoint &_pnt){
    for (int i = nodes.size()-1; i >=0; i--) {
        if ( nodes[i] == _pnt ){
            nodes[i].branchesIndex.push_back( lines.size()-1 );
            return;
        }
    }
    
    LNode node;
    node.set(_pnt);
    node.startTime = -1.0;
    node.branchesIndex.push_back( lines.size()-1 );
    nodes.push_back( node );
}

float LSystemReconstructor::getPct(){
    return ofClamp(pct,0.0,1.0);
}

void LSystemReconstructor::start(){
    bGrow = true;
}

void LSystemReconstructor::stop(){
    lastTime = ofGetElapsedTimef();
    pct = time = 0.0;
    bGrow = false;
}

void LSystemReconstructor::pause(){
    bGrow = false;
}

void LSystemReconstructor::update(){
    
    //  Update Time
    //
    float now = ofGetElapsedTimef();
    float diff = now - lastTime;
    if (bGrow){
        
        //  Go foward on time
        //
        time += diff*speed;
        
        //  Render
        //
        mesh.clear();
        activeNodes.clear();
        
        pct = 0.0;
        for(int i = 0; i < nodes.size(); i++){
            if (nodes[i].startTime >= 0.0){
                float relativeTime = time - nodes[i].startTime;
                for (int j = 0; j < nodes[i].branchesIndex.size(); j++){
                    renderBranch( nodes[i].branchesIndex[j], relativeTime, ofNoise(nodes[i].x+ofGetElapsedTimef()*0.01,nodes[i].y));
                }
            }
        }
    }
    
    lastTime = now;
}

void LSystemReconstructor::draw(){
    mesh.draw();
}

void LSystemReconstructor::renderBranch(int _index, float _relativeTime, float _speed){
    
    int totalPoints = lines[_index].size();
    int drawPoints = 0;
    
    for (int k = 0 ; k < totalPoints-1; k++){                  
        float thisTime = _speed*(float)k;
        float nextTime = _speed*((float)k+1.0f);;
        ofFloatColor color = ofFloatColor(1.0, 1.0-(((float)k)/((float)totalPoints))*0.8 );
                
        if ( k == totalPoints-1){
            nextTime = thisTime;
        }
    
        if (_relativeTime > thisTime && _relativeTime < nextTime ){
            float part = _relativeTime - thisTime;
            float whole = nextTime - thisTime;
            float pct = part / whole;
            
            ofPoint A = lines[ _index ][k];
            ofPoint B = lines[ _index ][k+1];
            
            // figure out where we are between a and b
            //
            ofPoint pos = (1.0-pct)*A + (pct)*B;
            
            activeNodes.addVertex(pos);
            
            addLineToMesh( mesh, lines[ _index ][k], pos, color);
            
        } else if ( _relativeTime > thisTime ){
            ofPoint pos = lines[ _index ][k];
            
            addLineToMesh( mesh, pos, lines[ _index ][k+1], color);
            
            //  check if pass over a node
            //
            int index = isNode(pos);
            if (index != -1){
                if ( nodes[index].startTime == -1.0 ){
                    nodes[index].startTime = time+bornRandom*_speed;
                }
            }
            
        } else {
            break;
        }
    }
}

void LSystemReconstructor::addLineToMesh(ofMesh &_mesh, ofPoint &A, ofPoint &B, ofFloatColor &c){
    mesh.addColor(c);
    mesh.addVertex(A);
    mesh.addColor(c);
    mesh.addVertex(B);
}

int LSystemReconstructor::isNode(ofPoint &_pnt){
    for (int i = nodes.size()-1; i >= 0; i--) {
        if ( nodes[i] == _pnt ){
            return i;
        }
    }
    
    return -1;
}