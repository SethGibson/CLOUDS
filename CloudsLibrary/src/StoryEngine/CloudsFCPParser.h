/**
 * CLOUDS FCP Parser
 * Class for parsing a set of FCP XML and presenting keywords
 *
 * Copyright 2013 James George
 */

#pragma once
#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "CloudsClip.h"
#include "CloudsLink.h"
#include <set>

class CloudsFCPParser {
  public:
    CloudsFCPParser();
    void setup(string directory);
    void refreshXML();
	void setCombinedVideoDirectory(string directory);

#pragma mark Clips
	CloudsClip getClipWithLinkName( string linkname );
	
#pragma mark Links
	//MANAGE
    void parseLinks(string linkFile);
	vector<CloudsLink>& getLinksForClip(CloudsClip& clip);
    vector<CloudsLink>& getLinksForClip(string clipName);
	
    void addLink(CloudsLink& link);
    void removeLink(string linkName, int linkIndex);
	void removeLink(string linkName, string targetName);
	void saveLinks(string linkFile);

	void suppressConnection(CloudsLink& link);
    void unsuppressConnection(string linkName, int linkIndex);
	void unsuppressConnection(string linkName, string targetName);

	//QUERIES
	//true if A has any out going links at all
	bool clipHasLink(CloudsClip& clip);
	bool clipHasLink(string clipName);
	//true if A links to B
    bool clipLinksTo(string clipNameA, string clipNameB);
	//true if A links to B or B links to A and the index of the link
	bool clipLinksTo(string clipNameA, string clipNameB, int& index);
	//true if A links to B or B links to A
	bool clipsShareLink(string clipNameA, string clipNameB);
	//true if A and B have clips that link to one another
	bool keywordsShareLink(string keyA, string keyB);
	
	//are there any clips that this suppresses?
	bool clipHasSuppressions(CloudsClip& clip);
	bool clipHasSuppressions(string clipName);
	
	//are we suppressing this connection?
	bool linkIsSuppressed(string clipNameA, string clipNameB);
	bool linkIsSuppressed(string clipNameA, string clipNameB, int& index);
	
	float percentOfClipsLinked();
	
	//create a list that maps all of the tags back to closest key theme
	void populateKeyThemes();
	void populateKeyThemes(set<string>& keyThemes);

	string getKeyThemeForTag(string tag);
	
#pragma mark Keywords
    void sortKeywordsByOccurrence(bool byOccurrence);
    vector<string>& getAllKeywords();
    vector<CloudsClip>& getAllClips();
	int getNumberOfClipsWithKeyword(string filterWord);
	vector<CloudsClip> getClipsWithKeyword(string filterWord);
    vector<CloudsClip> getClipsWithKeyword(const vector<string>& filter);
    set<string> getRelatedKeywords(string filterWord);
	int getNumberOfSharedKeywords(CloudsClip& a, CloudsClip& b);
	vector<string> getSharedKeywords(CloudsClip& a, CloudsClip& b);
	int getNumberOfSharedClips(string keywordA, string keywordB);
	
	vector<CloudsClip> getSharedClips(string keywordA, string keywordB);
	
    int occurrencesOfKeyword(string keyword);
    bool operator()(const string& a, const string& b);
    
#pragma mark key themes
	string closestKeyThemeToTag(string searchTag);
	
  protected:
    string xmlDirectory;
    string combinedVideoDirectory;
	
	bool keywordsDirty;
    void refreshKeywordVector();
    
	bool sortedByOccurrence;
	
    void addXMLFile(string xmlFile);
    void parseClipItem(ofxXmlSettings& finalCutXml, string xmlName);
	
    map<string, string> fileIdToPath;
    map<string, string> fileIdToName;
    vector<CloudsClip> allClips;
	set<string> markerLinkNames;

    map<string, int> allKeywords;
    vector<string> keywordVector;

    map<string, vector<CloudsLink> > linkedConnections;
	map<string, vector<CloudsLink> > suppressedConnections;
	map<string, vector<string> > sourceSupression;
	
	set<string> keyThemes;
	map<string,string> tagToKeyTheme;

};