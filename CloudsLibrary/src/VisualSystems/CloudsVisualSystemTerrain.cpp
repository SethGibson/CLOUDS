//
//  CloudsVisualSystemTerrain.cpp
//  VSTerrain
//
//  Created by Patricio Gonzalez Vivo on 5/28/13.
//
//

#include "CloudsVisualSystemTerrain.h"

string CloudsVisualSystemTerrain::getSystemName()
{
	return "Terrain";
}

void CloudsVisualSystemTerrain::selfSetup()
{
    terrainResolution = 1.0;
    setResolution(200, 200);
    
//    noiseShader.load("", getDataPath()+"shaders/VisualSystems/Terrain/noise.fs");
    noiseShader.load("", getDataPath()+"shaders/VisualSystems/Terrain/mNoise.fs");
    normalsShader.load("", getDataPath()+"shaders/VisualSystems/Terrain/normals.fs");
    patternShader.load("", getDataPath()+"shaders/VisualSystems/Terrain/pattern.fs");
    
    patternScale = 50.0;
    
    stripeAlpha = 1.0;
    hexAlpha = 1.0;
    dotsAlpha = 1.0;
    
    noiseSpeed = 0.0;
    
    bChange = true;
}

void CloudsVisualSystemTerrain::selfSetupSystemGui()
{
    sysGui->addLabel("Noise");
    sysGui->addSlider("noise_zoom", 0.0, 10.0, &noiseZoom);
    sysGui->addSlider("noise_speed", 0.0, 1.0, &noiseSpeed);
    
    sysGui->addLabel("Terrain");
    sysGui->addSlider("Terrain_Size", 10, 200, &size);
    sysGui->addSlider("Terrain_Altitud", 0, 2, &terrainHeight);
    sysGui->addSlider("Terrain_Resolution", 1, 10, &terrainResolution);
}

void CloudsVisualSystemTerrain::selfSetupRenderGui()
{
    rdrGui->addLabel("Patern");
    rdrGui->addSlider("Hex", 0.0, 1.0, &hexAlpha);
    rdrGui->addSlider("Stripes", 0.0, 1.0, &stripeAlpha);
    rdrGui->addSlider("Dots", 0.0, 1.0, &dotsAlpha);
}

void CloudsVisualSystemTerrain::guiSystemEvent(ofxUIEventArgs &e)
{
    string name = e.widget->getName();
    
    if ( name == "Terrain_Size" |name == "Terrain_Width" || name == "Terrain_Height" || name == "Terrain_Resolution"){
        setResolution(size, size);
    }
    
    bChange = true;
}

void CloudsVisualSystemTerrain::setResolution( int _width, int _height ){
    int width = _width;
    int height = _height;
    
    noiseFbo.allocate(width, height);
    normalsFbo.allocate(width, height);
    patternFbo.allocate(width*patternScale, height*patternScale);
}

void CloudsVisualSystemTerrain::selfKeyPressed(ofKeyEventArgs & args){
    if (args.key == OF_KEY_UP){
        camPosition.y += 1;
    } else if (args.key == OF_KEY_DOWN){
        camPosition.y -= 1;
    } else if (args.key == OF_KEY_LEFT){
        camPosition.x += 1;
    } else if (args.key == OF_KEY_RIGHT){
        camPosition.x -= 1;
    }
    
    bChange = true;
}

void CloudsVisualSystemTerrain::selfUpdate()
{
    
    if ( bChange || noiseSpeed > 0.0){
    
        //  NOISE
        //
        int width = noiseFbo.getWidth();
        int height = noiseFbo.getHeight();
        noiseFbo.begin();
        ofClear(0);
        noiseShader.begin();
        noiseShader.setUniform2f("resolution", width,height);
        noiseShader.setUniform2f("position", camPosition.x, camPosition.y);
        noiseShader.setUniform1f("zoom", noiseZoom);
        noiseShader.setUniform1f("time", ofGetElapsedTimef()*noiseSpeed);
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(0, 0, 0);
        glTexCoord2f(width, 0); glVertex3f(width, 0, 0);
        glTexCoord2f(width, height); glVertex3f(width, height, 0);
        glTexCoord2f(0,height);  glVertex3f(0,height, 0);
        glEnd();
        noiseShader.end();
        noiseFbo.end();
        
        //  Normals
        //
        normalsFbo.begin();
        normalsShader.begin();
        noiseFbo.draw(0, 0);
        normalsShader.end();
        normalsFbo.end();
        
        makeTerrain(noiseFbo.getTextureReference());
        
        //  Pattern
        //
        patternFbo.begin();
        ofClear(0);
        patternShader.begin();
        patternShader.setUniformTexture("tex0", noiseFbo, 0);
        patternShader.setUniform1f("textureScale", patternScale);
        patternShader.setUniform1f("scale", 0.48);
        
        patternShader.setUniform1f("hexAlpha", hexAlpha);
        patternShader.setUniform1f("stripeAlpha", stripeAlpha);
        
        patternShader.setUniform1f("pointsGap", 10.0);
        patternShader.setUniform1f("pointsAlpha", dotsAlpha);
        
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(0, 0, 0);
        glTexCoord2f(width*patternScale, 0); glVertex3f(width*patternScale, 0, 0);
        glTexCoord2f(width*patternScale, height*patternScale); glVertex3f(width*patternScale, height*patternScale, 0);
        glTexCoord2f(0,height*patternScale);  glVertex3f(0,height*patternScale, 0);
        glEnd();
        
        patternShader.end();
        patternFbo.end();
        
        bChange = false;
    }
}


void CloudsVisualSystemTerrain::makeTerrain( ofTexture &_heightMap ){
    
    //  Set VARIABLES
    //
    int width = _heightMap.getWidth();
    int height = _heightMap.getHeight();
    
    float flResolution = (int)terrainResolution;
    float flHeightScale = terrainHeight*50;
    float textureScale = patternScale;
    nVertexCount = (int) ( width * height * 6 / ( flResolution * flResolution ) );
    
    pVertices	= new ofVec3f[nVertexCount];		// Allocate Vertex Data
    pTexCoords	= new ofVec2f[nVertexCount];		// Allocate Tex Coord Data
    pNormals	= new ofVec3f[nVertexCount];		// Allocate Normals
    
    //  texture -> pixels
    //
    ofFloatPixels heightMap;
    heightMap.allocate(_heightMap.getWidth(),
                       _heightMap.getHeight(),
                       OF_PIXELS_RGBA);
    _heightMap.readToPixels(heightMap);
    
    //  normals -> pixels;
    //
    ofFloatPixels normalMap;
    normalMap.allocate(normalsFbo.getWidth(),
                       normalsFbo.getHeight(),
                       OF_PIXELS_RGBA);
    normalsFbo.getTextureReference().readToPixels(normalMap);
    
    camAltitud = ofLerp( camAltitud, heightMap.getColor(width*0.5, height*0.5).r * flHeightScale + 7, 0.1);
//    cam.setPosition(0, camAltitud, 0);
    
    //  Construct the VBO
    //
    int nIndex = 0;
    for( int nY = 0; nY < height-flResolution ; nY += (int) flResolution ){
        for( int nX = 0; nX < width-flResolution ; nX += (int) flResolution ){
            
            for( int nTri = 0; nTri < 6; nTri++ ){
                
                // Using This Quick Hack, Figure The X,Y Position Of The Point
                //
                float flX = (float) nX + ( ( nTri == 1 || nTri == 2 || nTri == 5 ) ? flResolution : 0.0f );
                float flZ = (float) nY + ( ( nTri == 2 || nTri == 4 || nTri == 5 ) ? flResolution : 0.0f );
                
                // Set The Data, Using PtHeight To Obtain The Y Value
                //
                pVertices[nIndex].x = flX - ( width * 0.5 );
                pVertices[nIndex].y = 1.0 - heightMap.getColor((int)flX, (int)flZ).r * flHeightScale;
                pVertices[nIndex].z = flZ - ( height * 0.5 );
                
                
                // 3	0 --- 1		nTri reference
                // | \	  \	  |
                // |   \	\ |
                // 4 --- 5	  2
                
                // Stretch The Texture Across The Entire Mesh
                //
                pTexCoords[nIndex].x = flX * textureScale;
                pTexCoords[nIndex].y = flZ * textureScale;
                
                // Normals by vert
                //
                pNormals[nIndex].x = normalMap.getColor((int)flX, (int)flZ).r * 2.0 - 1.0;
                pNormals[nIndex].y = normalMap.getColor((int)flX, (int)flZ).g * 2.0 - 1.0;
                pNormals[nIndex].z = normalMap.getColor((int)flX, (int)flZ).b * 2.0 - 1.0;
                // Increment Our Index
                //
                nIndex++;
            }
        }
    }
    
    terrainVbo.setVertexData(pVertices, nVertexCount, GL_STREAM_DRAW);
    terrainVbo.setNormalData(pNormals, nVertexCount, GL_STREAM_DRAW);
    terrainVbo.setTexCoordData(pTexCoords, nVertexCount, GL_STREAM_DRAW);
    
    // Our Copy Of The Data Is No Longer Necessary, It Is Safe In The Graphics Card
    delete [] pVertices; pVertices = NULL;
    delete [] pTexCoords; pTexCoords = NULL;
    delete [] pNormals; pNormals = NULL;
}

void CloudsVisualSystemTerrain::selfDraw()
{
    mat->begin();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    
    ofPushMatrix();
    
    patternFbo.getTextureReference().bind();

    ofSetColor(255);
    glEnable(GL_SMOOTH);
	glShadeModel(GL_SMOOTH);
    terrainVbo.draw(GL_TRIANGLES , 0, nVertexCount);
    
    patternFbo.getTextureReference().unbind();
    
    ofPopMatrix();
    
    glDisable(GL_NORMALIZE);
    glDisable(GL_DEPTH_TEST);
    mat->end();
}

void CloudsVisualSystemTerrain::billBoard()
{
    
}

void CloudsVisualSystemTerrain::selfSetupGuis()
{
    
}

void CloudsVisualSystemTerrain::selfAutoMode()
{
    
}


void CloudsVisualSystemTerrain::selfDrawBackground()
{
    
}

void CloudsVisualSystemTerrain::selfDrawDebug()
{
    
}

void CloudsVisualSystemTerrain::selfSceneTransformation()
{
    
}

void CloudsVisualSystemTerrain::selfExit()
{
    
}

void CloudsVisualSystemTerrain::selfBegin()
{
    
}

void CloudsVisualSystemTerrain::selfEnd()
{
    
}

void CloudsVisualSystemTerrain::selfKeyReleased(ofKeyEventArgs & args)
{
    
}

void CloudsVisualSystemTerrain::selfMouseDragged(ofMouseEventArgs& data)
{
    
}

void CloudsVisualSystemTerrain::selfMouseMoved(ofMouseEventArgs& data)
{
    
}

void CloudsVisualSystemTerrain::selfMousePressed(ofMouseEventArgs& data)
{
    
}

void CloudsVisualSystemTerrain::selfMouseReleased(ofMouseEventArgs& data)
{
    
}

void CloudsVisualSystemTerrain::selfSetupGui()
{
    
}

void CloudsVisualSystemTerrain::selfGuiEvent(ofxUIEventArgs &e)
{
    
}

void CloudsVisualSystemTerrain::guiRenderEvent(ofxUIEventArgs &e)
{
    bChange = true;
}