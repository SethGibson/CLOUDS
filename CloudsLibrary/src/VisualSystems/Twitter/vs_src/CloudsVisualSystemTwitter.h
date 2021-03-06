//
// CLOUDS Interactive Documentary
//
// VISUAL SYSTEMS
//
// Welcome to the EMPTY CloudsVisualSystem
//
//
//

#pragma once

#include "CloudsVisualSystem.h"
#include "Tweeter.h"
#include "ofxFTGL.h"
#include "ofxBillboard.h"
//--------------------------------------------------------------
//--------------------------------------------------------------
class CloudsVisualSystemTwitter : public CloudsVisualSystem
{
  public:
    
	//This determines your data path so name it at first!
	//ie getVisualSystemDataPath() uses this
    string getSystemName() {
		return "Twitter";
	}

	//These methods let us add custom GUI parameters and respond to their events
	void selfSetDefaults();
	
    void selfSetupGui();
    void selfGuiEvent(ofxUIEventArgs &e);
    void selfSetupSystemGui();
    void guiSystemEvent(ofxUIEventArgs &e);
    void selfSetupRenderGui();
    void guiRenderEvent(ofxUIEventArgs &e);
    void selfSetup();
    void selfBegin();
	void selfPresetLoaded(string presetPath);
    void selfSceneTransformation();
	void selfUpdate();
    void selfDraw();
	void selfDrawRGBD();
	void selfDrawDebug();
	void selfDrawBackground();
	void selfEnd();
    void selfExit();

    void selfKeyPressed(ofKeyEventArgs& args);
    void selfKeyReleased(ofKeyEventArgs& args);
    void selfMouseDragged(ofMouseEventArgs& data);
    void selfMouseMoved(ofMouseEventArgs& data);
    void selfMousePressed(ofMouseEventArgs& data);
    void selfMouseReleased(ofMouseEventArgs& data);
    
    void initSystem(string filePath);
    
    //twitter feed
    void drawFeed();
    void updateCurrentSelection(int index,bool firstTime );
    
    //i/o stuff

//    void addUsersFromMentions(ofVec2f& curActivityMapCoord, int activityMapWidth );
    void createPajekNetwork(string outputFileName);
    void parseClusterNetwork(string fileName);
//    void createNewGraph(string outputFileName, string inputDataFolder);
    
    //data stuff
    int getUserIdByName(string name);
    vector<Tweeter*> getTweetersForDate(int index);
    Tweeter* getTweeterByID(int _id );
    void drawTweetsForDate(int index);
    void CompareDates(Date d1,Date d2);
    void loadGraphFromPath(string filePath);
    void clearData();
    void sortTweetsByDate();

    //shader stuff
    void reloadShaders();

    ofImage tweetDeckMenu;
    
    //twitter feed params
    int numberOfTweets;
    int currentTweetFeedIndex;
    float heightOffset;
    float avatarTweetGap;
    float textHeightOffset;
    float timeTillNextUpdate;
    float minTimeGapForNextTweet;
    float randomRangeMin,randomRangeMax;
    int avatarSize;
    float tweetDeckWidth;
    float tweetDeckHeight;
    float tweetDeckHeightOffset;
    float tweetDeckWidthOffset;
    float tweetDeckLineOffset;

    
    float animationLerpAmt;
    float animationLerpRate;
    float feedStringWidthCap;
    
    //text stuff
    ofxFTGLSimpleLayout font;
    ofxFTGLSimpleLayout tweetFont;
    ofxFTGLSimpleLayout tweetFontSmall;
    ofxFTGLSimpleLayout twitterHandleFont;
    ofxFTGLTextAlignment textAlign;
    float stringWidth;
    float fontSize;
    float tweetFontSize;
    float tweetLineLength;
    float tweetHandleFontSize;
    float tweetHandleLineLength;
    
    int minUserMentions;
    
    //draw stuff
    void loadMesh();
    void updateActiveTweeters(int index);
    void setActiveTweeters(int index );
    Tweet* csvParseTweet(vector<string>& line, Tweeter* curTweeter);
    void updateMesh();
    void drawText(string text, ofVec3f pos, float alpha);
    void drawText2D(string text, ofVec2f pos);

    //helpers
    set<pair<int,int> > links;
    map<pair<string, string>, pair<int, int> >lineIndexPairs;

    Date getDateFromString(const string& dString);

    int currentDateIndex;
    float dateIndexMin, dateIndexMax;
    void updateLabelWithCurrentMeshName(string name);

    // Ma boooooools
    bool bRenderMesh;
    bool bRenderText;
    bool bRenderFeed;
    bool bAnimate;
    bool rotateModel;
    bool bAnimateFeed;
    bool bAnimateSpriteSize;
    bool bStaticNameDraw;
    bool bOldData;
    
    //static void loadJSONData(string folderName, vector<Tweeter>& curTweeters);
    //static vector<Tweeter>& getOldTweeterData();
    //static vector<Tweeter>& getNewTweeterData();
    static string getDateAsString(Date d);    

    void allocateActivityMap();
  protected:
    ofDirectory meshDir;
    vector<string> meshStrings;
    int currentMeshIndex;
    
    int maxUserLinks;
    ofColor listColor;
    vector<Tweeter*> tweeters;
    float sizeMultiplier;
    float maxAlphaTweetFeed;
    float lineAlpha;
    float avatarAlpha;
    
    float minSize;
    float maxSize;
    
	ofFloatImage activityMap;

    ofFloatColor spriteBaseColorHSV;
    ofFloatColor spritePopColorHSV;
    stringstream ss;
    
    ofVbo edgeMeshVbo;
    ofVbo nodeMeshVbo; 
    vector<ofVec3f> edgeMeshNormals;
    vector<ofVec3f> nodeMeshNormals;
    int edgeMeshNumVertices;
    int nodeMeshNumVertices;
    ofVec3f min,max;
    
    int refreshRate;
    int activeTweeterRefreshRate;
    float edgeDecayRate;

    //point sprite stuff
    ofImage sprite;
    ofShader lineShader;
    ofRange pointsSize;
    
    ofShader pointsShader;
    float meshExpansion;
    float pointSize;
    float activityMapDamping;
    float synapseLevel;
    float normalDecay;
    ofxUISuperCanvas* clusterGui;
    ofxUISuperCanvas* spriteGui;
    ofxUISuperCanvas* textGui;
    ofxUISuperCanvas* twitterFeedGui;
    
    vector<Tweeter*> activeTweeters;
    vector<string*> activeTweets;
    vector<pair<string*, string*> >  activeTweetPairs;
    vector<pair<string*, string*> > currentSelection;
    
    map<string, ofImage> avatars;

    void loadAvatars();
    void loadCSVData();
    void addColorToGui(ofxUISuperCanvas* gui, string prefix, ofFloatColor& col, bool doAlpha = true);
    map< ofFloatColor*, ofxUILabel*> labelColors;
	map< string, Date> dateMap;

	float edgeInterpolateExponent;
	ofFloatColor getRGBfromHSV(ofFloatColor& hsv);
	//this is the base color of the lines close to the nodes
	ofFloatColor lineNodeBaseHSV;
	//this is the base color of the lines at the midpoint
	ofFloatColor lineEdgeBaseHSV;
	
	//this is the pop color of the lines close to the nodes
	ofFloatColor lineNodePopHSV;
	//this is the pop color of the lines at the midpoint
    ofFloatColor lineEdgePopHSV;

    //synapse color
    ofFloatColor synapseColorHSV;

	//this is the base color of the node
	ofFloatColor nodeBaseColorHSV;
	//this is the pop color of the node
	ofFloatColor nodePopColorHSV;
	   
    ofFloatColor textColorHSV;
    
    ofFloatColor tweetDeckColorHSV;
    
    string currentMeshFilePath;
    string currentMeshFileName;
    int theme;
    float xScale,yScale, zScale;
    int currentRefreshRate;
    int rotationRate;
    float rotation ;
    float rotationAmount;

    ofRectangle tweetFeedRect;
    
    Tweeter dummyTweet;
	
};
