//
//  CloudsStoryEngine.cpp
//  CloudsFCPParser
//
//  Created by James George on 3/17/13.
//
//

#include "CloudsStoryEngine.h"
#include "CloudsAct.h"

bool logsort(pair<float,string> a, pair<float,string> b ){
    return a.first > b.first;
}

CloudsStoryEngine::CloudsStoryEngine(){
	parser = NULL;
	visualSystems = NULL;
    
	isSetup = false;
	printDecisions = true;
	combinedClipsOnly = false;
	printCriticalDecisions = false;
	
//	fixedClipDelay = 5;
	maxTimesOnTopic = 4;
    
    systemMaxRunTime = 60*2;
    maxVisualSystemGapTime = 60*3;
    longClipThreshold = 30;
    longClipFadeInPercent = .5;
    actLength = 10 * 60;
    
    topicsInCommonMultiplier = 10;
    topicsinCommonWithPreviousMultiplier = 5;
    samePersonOccuranceSuppressionFactor = 4;
    dichomoiesFactor = 2;
    linkFactor =20 ;
    maxTimeWithoutQuestion =120;
    gapLengthMultiplier = 0.5;
    preRollDuration = 5;
    minClipDurationForStartingOffset = 30;
    initGui();
    displayGui(false);
    
    
}

CloudsStoryEngine::~CloudsStoryEngine(){
    delete gui;
    delete clipGui;
}

void CloudsStoryEngine::setup(){
	if(!isSetup){
		isSetup = true;
        
        KeywordDichotomy d;
        dichotomyThreshold = 3;
        
		dichotomies.clear();
		
        d.left = "#art";
        d.right = "#tech";
        d.balance =0;
        dichotomies.push_back(d);
        
        d.left = "#emotional";
        d.right = "#logical";
        d.balance =0;
        dichotomies.push_back(d);
        
        d.left = "#breakthrough";
        d.right = "#obstacle";
        d.balance =0;
        dichotomies.push_back(d);
        
        d.left = "#inspiring";
        d.right = "#discouraging";
        d.balance =0;
        dichotomies.push_back(d);
        
        d.left = "#fun";
        d.right = "#serious";
        d.balance =0;
        dichotomies.push_back(d);
        
        d.left = "#sincere";
        d.right = "#ironic";
        d.balance =0;
        dichotomies.push_back(d);
        
        d.left = "#mindblowing";
        d.right = "#mundane";
        d.balance =0;
        dichotomies.push_back(d);
        
        d.left = "#rational";
        d.right = "#surreal";
        d.balance =0;
        dichotomies.push_back(d);
	}
}

void CloudsStoryEngine::initGui(){
    clipGui = new ofxUISuperCanvas("CLIP SCORE PARAMS :", OFX_UI_FONT_SMALL);
    clipGui->setPosition(0,0);
	clipGui->addSpacer();
    clipGui->addSlider("CURRENT TOPICS IN COMMON MULTIPLIER", 0, 50, topicsInCommonMultiplier);
    clipGui->addSlider("TOPICS IN COMMON WITH HISTORY MULTIPLIER", 0, 10, topicsinCommonWithPreviousMultiplier);
    clipGui->addSlider("SAME PERSON SUPPRESSION FACTOR", 0, 10, samePersonOccuranceSuppressionFactor);
    clipGui->addSlider("LINK FACTOR",0,50,linkFactor);
    clipGui->addSlider("DICHOTOMIES FACTOR", 0,10,dichomoiesFactor);
    
    clipGui->autoSizeToFitWidgets();
    
    gui = new ofxUISuperCanvas("ACT BUILDER GENERAL PARAMS :", OFX_UI_FONT_SMALL);
    gui->setPosition(clipGui->getRect()->x + 100, clipGui->getRect()->y + 10);
    gui->addSpacer();
    gui->addSlider("MAX TIME W/O QUESTION", 60, 600, &maxTimeWithoutQuestion);
    gui->addSlider("GAP LENGTH MULTIPLIER", 0.01, 0.1, &gapLengthMultiplier);
    gui->addSlider("ACT LENGTH", 60, 1200,&actLength);
    gui->addSlider("PREROLL FLAG TIME", 1, 10, &preRollDuration);
    gui->addSlider("MIN CLIP DURATION FOR START OFFSET", 0, 100, &minClipDurationForStartingOffset);
    gui->addButton("BUILD ACT", false);
    
    gui->autoSizeToFitWidgets();
    
    vsGui = new ofxUISuperCanvas("ACT BUILDER VS PARAMS :", OFX_UI_FONT_SMALL);
    vsGui->setPosition(gui->getRect()->x + 100, gui->getRect()->y + 10);
    vsGui->addSpacer();
    vsGui->addSlider("MAX VS RUNTIME", 0, 480,&systemMaxRunTime);
    vsGui->addSlider("MAX VS GAPTIME", 0, 60, &maxVisualSystemGapTime);
    vsGui->addSlider("LONG CLIP THRESHOLD", 0, 100,&longClipThreshold);
    vsGui->addSlider("LONG CLIP FAD IN %", 0.0, 1.0, &longClipFadeInPercent);

    vsGui->autoSizeToFitWidgets();
    
    
    string filePath = getDataPath() +"storyEngineParameters/gui.xml";
    if(ofFile::doesFileExist(filePath))gui->loadSettings(filePath);
    
    string filePath2 = getDataPath() +"storyEngineParameters/clipGui.xml";
    if(ofFile::doesFileExist(filePath2))clipGui->loadSettings(filePath2);
    
    string filePath3 = getDataPath() +"storyEngineParameters/vsGui.xml";
    if(ofFile::doesFileExist(filePath3))clipGui->loadSettings(filePath3);
    
    ofAddListener(gui->newGUIEvent, this, &CloudsStoryEngine::guiEvent);
    ofAddListener(clipGui->newGUIEvent, this, &CloudsStoryEngine::guiEvent);
    ofAddListener(vsGui->newGUIEvent, this, &CloudsStoryEngine::guiEvent);
    
}

void CloudsStoryEngine::guiEvent(ofxUIEventArgs &e)
{
    string name = e.widget->getName();
    ofxUIButton* b = (ofxUIButton*) e.widget;
    if(name == "BUILD ACT" &&  b->getValue() ){
        CloudsClip& clip = parser->getRandomClip(false,false);
		buildAct( clip );
    }
}

void CloudsStoryEngine:: saveGuiSettings(){
    
    string filePath = getDataPath() +"storyEngineParameters/gui.xml";
    string filePath2 = getDataPath() +"storyEngineParameters/clipGui.xml";
    string filePath3 = getDataPath() +"storyEngineParameters/vsGui.xml";
    gui->saveSettings(filePath);
    clipGui->saveSettings(filePath2);
    vsGui->saveSettings(filePath3);
    
}

void CloudsStoryEngine:: displayGui(bool display){
    if(display){
        gui->enable();
        clipGui->enable();
        vsGui->enable();
    }
    else{
        clipGui->disable();
        gui->disable();
        vsGui->disable();
    }
}

CloudsAct* CloudsStoryEngine::buildAct(CloudsClip& seed){
    return buildAct(seed, seed.getKeywords()[ ofRandom(seed.getKeywords().size()) ]);
}

CloudsAct* CloudsStoryEngine::buildAct(CloudsClip& seed, string topic){
    
    CloudsAct* act = new CloudsAct();
    float seconds = actLength;
    float totalSecondsEnqueued = 0;
    bool freeTopic = false;
    bool deadEnd = false;
    
    vector<CloudsClip> clipHistory;
    vector<CloudsClip> clipQueue;

    //VS Stuff
    float lastVisualSystemEnded = 0;
    bool systemRunning = false;
    float visualSystemStartTime =0;
    string previousTopic = " ";
    float visualSystemDuration  = 0;
    //VS Stuff
    
    
    scoreBuffer.clear();
    scoreStream.clear();
    CloudsClip clip = seed;
    
    int timeForNewQuesiton = 0;
    
    scoreStream<<"Selected Clip,Current Topic, Potential Next Clip,Total Score,topicsInCommonScore,topicsInCommonWithPreviousScore,samePersonOccuranceScore,dichotomiesScore"<<endl;
    
    clearDichotomiesBalance();
    
	float preRollFlagTime  = 0;
	float clipHandleDuration = getHandleForClip(clip);
	act->addClipPreRollFlag(preRollFlagTime, clipHandleDuration, clip.getLinkName());
	totalSecondsEnqueued = preRollDuration;
	
    act->addClip(clip, topic, totalSecondsEnqueued, clipHandleDuration);
    totalSecondsEnqueued += clip.getDuration()+( gapLengthMultiplier * clip.getDuration() ) * clipHandleDuration*2;
    
    vector<string> topicHistory;
    topicHistory.push_back(topic);
    
    int timesOnCurrentTopic = 0;
    while( totalSecondsEnqueued < seconds ){
        scoreStream<<clip.getLinkName()<<","<<topic<<","<<" "<<","<<" "<<","<<" "<<","<<" "<<","<<" "<<endl;
        
        freeTopic |= timesOnCurrentTopic > maxTimesOnTopic;
        
        if(freeTopic){
            string newTopic = selectTopic(act, clip, topicHistory, topic);
            if(newTopic == topic){
                break;
                
            }
            
            topic = newTopic;
            timesOnCurrentTopic = 1;
            freeTopic = false;
            topicHistory.push_back(topic);
            
        }
        
        cout<<"Times on topic :"<< topic<<", "<<timesOnCurrentTopic<<endl;
        //storing a copy of current topic for each clip
        
        //get all meta data options
        vector<CloudsClip> nextOptions = parser->getClipsWithKeyword(topic);
        
        //add all manual links
        vector<CloudsLink>& links = parser->getLinksForClip( clip );
        for(int i = 0; i < links.size(); i++){
            
            bool valid = true;
            for(int c = 0; c < nextOptions.size(); c++){
                if(nextOptions[c].getLinkName() == links[i].targetName){
                    //don't add...
                    valid = false;
                    break;
                }
            }
            
            if(valid){
                nextOptions.push_back( parser->getClipWithLinkName(links[i].targetName) );
            }
        }
        
        
        //remove suppressions
        vector<CloudsLink>& suppressions = parser->getSuppressionsForClip( clip );
        for(int i = 0; i < suppressions.size(); i++){
            for(int nt = nextOptions.size()-1; nt >= 0; nt--){
                if(nextOptions[nt].getLinkName() == suppressions[i].targetName){
                    nextOptions.erase(nextOptions.begin() + nt);
                }
            }
        }
        
        vector<pair<int,string> > scoreLogPairs;
        //remove clips that share just one keyword...
        int topScore = 0;
        for(int i = 0; i < nextOptions.size(); i++){
            CloudsClip& nextClipOption = nextOptions[ i ];
            string log = "";
            
            int score = scoreForClip(act->getAllClips(), nextClipOption, topic,log);
            //give additional weight to clips that are better suited for voice overs if a VS is playing.
            if( systemRunning && nextClipOption.hasSpecialKeyword("#vo") ){
                //TODO: Figure out with James what this value should be.
                score += 15;
                
            }
            scoreLogPairs.push_back( make_pair(score,log));
            totalPoints += score;
            topScore = MAX(topScore, score);
            nextClipOption.currentScore = score;
        }
        
        sort(scoreLogPairs.begin(), scoreLogPairs.end(), logsort);
        for (int i=0; i<scoreLogPairs.size(); i++) {
            scoreStream<<scoreLogPairs[i].second;
        }
        
        if(topScore == 0){
            //Dead end!
            cout << "Dead end, no clips left" << endl;
            freeTopic = true;
            continue;
        }
        
        vector<CloudsClip> winningClips;
        for(int i = 0; i < nextOptions.size(); i++){
            if(nextOptions[i].currentScore == topScore){
                winningClips.push_back(nextOptions[i]);
            }
        }
        
        //select next clip
        clip = winningClips[ofRandom(winningClips.size())];
        
        updateDichotomies(clip);
        
        //adding all option clips with questions
        for(int k=0; k<nextOptions.size(); k++){
            
            if(nextOptions[k].getQuestionsVector().size() > 0 &&
			   parser->clipLinksTo(clip.getLinkName(), nextOptions[k].getLinkName())){
				act->addQuestion(nextOptions[k], totalSecondsEnqueued);
            }
        }
        
		clipHandleDuration = getHandleForClip(clip);
        act->addClip(clip,topic,totalSecondsEnqueued, clipHandleDuration);
		float preRollFlagTime  = totalSecondsEnqueued - preRollDuration;
        act->addClipPreRollFlag(preRollFlagTime, clipHandleDuration, clip.getLinkName());
      
        
        // populating Visual Systems
        //topic = act->getTopicForClip(clip);
    
        float clipStartTime = act->getItemForClip(clip).startTime;
        float clipEndTime = act->getItemForClip(clip).endTime;
        
        if( systemRunning ) {
            
			//TODO: check if indefinite
			//preset.indefinite;  if not, use preset.duration
			//
            visualSystemDuration = clipStartTime - visualSystemStartTime;
            if(visualSystemDuration > systemMaxRunTime || topic != previousTopic){
                if(clip.getDuration() > longClipThreshold){
                    visualSystemDuration += clip.getDuration()*longClipFadeInPercent;
                }
                //TODO: Make non-random
                CloudsVisualSystemPreset& preset = visualSystems->getRandomVisualSystem();
                act->addVisualSystem(preset, visualSystemStartTime, visualSystemDuration);
                act->removeQuestionAtTime(visualSystemStartTime, visualSystemDuration);
                systemRunning = false;
                lastVisualSystemEnded = visualSystemStartTime + visualSystemDuration;
            }
        }
        else {
            float timeSinceLastVisualSystem = clipEndTime - lastVisualSystemEnded;
            
            //if the clip is shorter than the 30 seconds dont start the VS during the clip.
            if(timeSinceLastVisualSystem > maxVisualSystemGapTime && clip.getDuration() > longClipThreshold ){
                
                visualSystemStartTime  = clipStartTime + clip.getDuration()*longClipFadeInPercent;
                systemRunning = true;
            }
        }
        
        previousTopic = topic;
        totalSecondsEnqueued += clip.getDuration() + ( gapLengthMultiplier * clip.getDuration() ) + clipHandleDuration*2;
        timesOnCurrentTopic++;
        
        
        //Decide if a question is to be asked
    }
	
    //TODO: be aware if you have ended on a fixed duration VS to respect its duration
    if(systemRunning){
        float clipStartTime = act->getItemForClip(act->getClip(act->getAllClips().size()-1)).startTime;
        float clipEndTime = act->getItemForClip(act->getClip(act->getAllClips().size()-1)).endTime;
        act->addVisualSystem(visualSystems->getRandomVisualSystem(), visualSystemStartTime, visualSystemDuration);
        systemRunning = false;
        lastVisualSystemEnded = visualSystemStartTime + visualSystemDuration;
    }
    
    
    // do the same thing again for Visual Systems
//    float lastVisualSystemEnded = 0;
//    bool systemRunning = false;
//    
//    float visualSystemStartTime;
//    //    string currentTopic = act.getTopicInHistory(0);
//    string previousTopic = currentTopic;
//    float visualSystemDuration  = 0;
//    
//    for(int i=0; i < act->getAllClips().size(); i++){
//
//        CloudsClip& currentClip = act->getClip(i);
//        topic = act->getTopicForClip(currentClip);
//        //        string currentTopic = act.getTopicInHistory(i);
//        float clipStartTime = act->getItemForClip(currentClip).startTime;
//        float clipEndTime = act->getItemForClip(currentClip).endTime;
//        
//        if( systemRunning ) {
//            
//            visualSystemDuration = clipStartTime - visualSystemStartTime;
//            if(visualSystemDuration > systemMaxRunTime || topic != previousTopic){
//                if(currentClip.getDuration() > longClipThreshold){
//                    visualSystemDuration += currentClip.getDuration()*longClipFadeInPercent;
//                }
//                CloudsVisualSystemPreset& preset = visualSystems->getRandomVisualSystem();
//                act->addVisualSystem(preset, visualSystemStartTime, visualSystemDuration);
//                
//                act->removeQuestionAtTime(visualSystemStartTime, visualSystemDuration);
//                systemRunning = false;
//                lastVisualSystemEnded = visualSystemStartTime + visualSystemDuration;
//            }
//        }
//        else {
//            float timeSinceLastVisualSystem = clipEndTime - lastVisualSystemEnded;
//            
//            //if the clip is shorter than the 30 seconds dont start the VS during the clip.
//            if(timeSinceLastVisualSystem > maxVisualSystemGapTime && currentClip.getDuration() > longClipThreshold){
//                
////                if (currentClip.getDuration() > longClipThreshold) {
//                //Add some fade in time 
//                    visualSystemStartTime  = clipStartTime + currentClip.getDuration()*longClipFadeInPercent;
////                }
////                else{
////                    visualSystemStartTime = clipStartTime;
////                }
//                systemRunning = true;
//            }
//        }
//        
//        previousTopic = topic;
        
//    }
    

    
//    float questionStartTime = 0;
//    float lastQuestionTime = 0;
//    float currentTime = 0;
    
    //Leaving this in for now incase we decide to re implement it later
    
    //questions shouldnt start during visual systems
    //        for(int k=0; k < act->getAllVisualSystems().size(); k++){
    //            CloudsVisualSystemPreset& preset = act->getVisualSystemInAct(k);
    //
    //            float presetStartTime = act->getItemForVisualSystem(preset).startTime;
    //            float presetEndTime = act->getItemForVisualSystem(preset).endTime;
    //
    //            float timeSinceLastQuestion = presetStartTime - lastQuestionTime;
    //
    //
    //            if(timeSinceLastQuestion > maxTimeWithoutQuestion){
    //
    //                CloudsClip& clip = act->getClipAtTime(presetStartTime);
    //
    //                float clipStartTime = act->getItemForClip(clip).startTime;
    //                float clipEndTime = act->getItemForClip(clip).endTime;
    //
    //                string topic =act->getTopicForClip(clip);
    //                vector<CloudsClip> clips = parser->getClipsWithQuestionsForTopic(topic);//getClipsWithKeyword(topic);
    //
    //                //if a visual system is already playing and is going to end in the clip.
    //                //ask the question after the system stops playing.
    //                if(clipStartTime > presetStartTime && clipEndTime > presetEndTime){
    //                    questionStartTime = presetEndTime;
    //
    //                }
    //                else{
    //                    questionStartTime = clipStartTime;
    //                }
    //
    //                for(int i = 0; i<clips.size(); i++){
    //                  if(){
    //                      act->addQuestion(clips[i], questionStartTime);
    //
    //                  }
    //                  else if(i == clips.size()-1){
    //
    //                      clips[i].setStartingQuestion("Dummy question from story engine? Yes.");
    //                      act->addQuestion(clips[i], questionStartTime);
    //                      cout<<"No question found!"<<endl;
    //                  }
    //                }
    //		                lastQuestionTime = questionStartTime;
    //            }
    //
    //        }
    //                           if(clips[i].hasStartingQuestion() && clips[i].getLinkName() != clip.getLinkName() ){
    //                               cout<<"Adding Starting Question "<<clips[i].startingQuestion<< " from clip" << clip.getLinkName()<< endl;
    //                               act->addQuestion(clips[i], questionStartTime);
    //                               break;
    //                           }
    act->populateTime();
    
    scoreBuffer.set(scoreStream);
    ofBufferToFile("score.csv", scoreBuffer);
    
    CloudsActEventArgs args(act);
    ofNotifyEvent(events.actCreated, args);
    
    return act;
}

float CloudsStoryEngine::getHandleForClip(CloudsClip& clip){
	// if clip is longer than minimum length for long clip allow the 2 second intro
//	if (clip.getDuration()>minClipDurationForStartingOffset) {
//		return 0;
//	}
	return 1;			
}

void CloudsStoryEngine::clearDichotomiesBalance(){
    
    for(int i=0; i <dichotomies.size();i++){
        dichotomies[i].balance =0;
    }
}

void CloudsStoryEngine::updateDichotomies(CloudsClip& clip){
    vector<string> specialkeywords= clip.getSpecialKeywords();
    
    for(int i=0; i <dichotomies.size();i++){
        for(int j=0; j<specialkeywords.size();j++){
            
            if(dichotomies[i].left == specialkeywords[j]){
                dichotomies[i].balance -= 1;
                cout<<dichotomies[i].left<<": +1"<<endl;
                
            }
            else if (dichotomies[i].right == specialkeywords[j]){
                cout<<dichotomies[i].right<<": +1"<<endl;
                dichotomies[i].balance += 1;
            }
            
        }
    }
}

vector<KeywordDichotomy> CloudsStoryEngine::getCurrentDichotomyBalance(){
    return dichotomies;
}

//TODO: use coehsion and map distance to fix dead ends
string CloudsStoryEngine::selectTopic(CloudsAct* act, CloudsClip& clip, vector<string>& topicHistory, string topic){
    
    vector<string>& topics = clip.getKeywords();
    vector<float> topicScores;
    topicScores.resize(topics.size());
    float topicHighScore = 0;
    for(int i = 0; i < topics.size(); i++){
        topicScores[i] = scoreForTopic(topicHistory, act->getAllClips(), topic, topics[i]);
        topicHighScore = MAX(topicHighScore,topicScores[i]);
    }
    
    if(topicHighScore == 0){
        //Dead end!
        cout << "Dead end, no topics left" << endl;
        return topic;
    }
    
    vector<string> winningTopics;
    for(int i = 0; i < topics.size(); i++){
        if(topicScores[i] == topicHighScore){
            winningTopics.push_back(topics[i]);
        }
    }
    
    cout << "topic changed from " << topic;
    topic = winningTopics[ ofRandom(winningTopics.size()) ];
    cout << " to " << topic << endl;
    
    return topic;
}

float CloudsStoryEngine::scoreForTopic(vector<string>& topicHistory, vector<CloudsClip>& history,
                                       string currentTopic, string newTopic)
{
    if(currentTopic == newTopic){
        if(printDecisions) cout << "	REJECTED Topic " << currentTopic << ": same as new topic" << endl;
        return 0;
    }
    
    if(ofContains(topicHistory, newTopic)){
        if(printDecisions) cout << "	REJECTED Topic " << newTopic << ": has already been explored" << endl;
        return 0;
    }
    
    int score = 5;
    if(history.size() > 1 && ofContains( history[history.size()-2].getKeywords(), newTopic) ){
        if(printDecisions) cout << "	LAST CLIP " << history[history.size()-2].getLinkName() << " shares topic " << newTopic << endl;
        score += 10;
    }
    
    if(history.size() > 2 && ofContains( history[history.size()-3].getKeywords(), newTopic) ){
        if(printDecisions) cout << "	TWO CLIPS AGO " << history[history.size()-3].getLinkName() << " shares topic " << newTopic << endl;
        score += 10;
    }
    
    int sharedClips = parser->getNumberOfSharedClips(currentTopic,newTopic);
    int totalClips = parser->getNumberOfClipsWithKeyword(newTopic);
    float relevancyScore = (1.0 * sharedClips / totalClips) * 10;
    score += relevancyScore;
    cout << "	TOPIC " << newTopic << " SCORE " << relevancyScore << " : " << sharedClips << "/" << totalClips << endl;
    
    float cohesionScore;//Add cohesion score to favor topics
    
    return score;
}

float CloudsStoryEngine::scoreForClip(vector<CloudsClip>& history, CloudsClip& potentialNextClip, string topic, string& log){
    
    CloudsClip& currentlyPlayingClip = history[history.size()-1];
    
    //rejection criteria -- flat out reject clips on some basis
    if(combinedClipsOnly && !potentialNextClip.hasCombinedVideo){
        if(printDecisions) cout << "	REJECTED Clip " << potentialNextClip.getLinkName() << ": no combined video file" << endl;
        return 0;
    }
    
    
    //    if(potentialNextClip.cluster.Id == ""){
    //		if(printDecisions) cout << "	REJECTED Clip " << potentialNextClip.getLinkName() << ": disconnect from cluster map =(" << endl;
    //        return 0;
    //    }
    
    bool link = parser->clipLinksTo( currentlyPlayingClip.getLinkName(), potentialNextClip.getLinkName() );
    if(!link && potentialNextClip.person == currentlyPlayingClip.person){
        if(printDecisions) cout << "	REJECTED Clip " << potentialNextClip.getLinkName() << ": same person" << endl;
        return 0;
    }
    
    //reject any nodes we've seen already
    if(historyContainsClip(potentialNextClip, history)){ //TODO discourage...
        if(printDecisions) cout << "	REJECTED Clip " << potentialNextClip.getLinkName() << ": already visited" << endl;
        return 0;
    }
    
    int occurrences = occurrencesOfPerson(potentialNextClip.person, 20, history);
    if(occurrences > 4){
        if(printDecisions) cout << "	REJECTED Clip " << potentialNextClip.getLinkName() << ": person appeared more than 4 times in the last 20 clips" << endl;
        return 0;
    }
    
    //Base score
    int totalScore = 0;
    int topicsInCommonScore = 0;
    
    int topicsInCommonWithPreviousScore = 0;
    
    int linkScore =0 ;
    
    int samePersonOccuranceScore =0;
    
    int dichotomiesScore = 0;
    
    
    int topicsInCommon = parser->getSharedKeywords(currentlyPlayingClip, potentialNextClip).size();
    
    
    topicsInCommonScore +=topicsInCommon*topicsInCommonMultiplier;
    
    if(history.size() > 1){
        int topicsInCommonWithPrevious = parser->getSharedKeywords(history[history.size()-2], potentialNextClip ).size();
        //		score += topicsInCommonWithPrevious * 5;
        
        topicsInCommonWithPreviousScore += topicsInCommonWithPrevious * topicsinCommonWithPreviousMultiplier;
    }
    
    //If this clip is a link weight it highly
    if( link ){
        linkScore += linkFactor;
    }
    
    //penalize for the person occurring
    //	score -= occurrences*4;
    samePersonOccuranceScore += occurrences * samePersonOccuranceSuppressionFactor;
    
    //history should contain #keywords dichotomies, and then augment score
    vector<string> specialKeywords = potentialNextClip.getSpecialKeywords();
    
    for(int i=0; i< dichotomies.size(); i++){
        for( int k=0; k<specialKeywords.size(); k++){
            if(dichotomies[i].right == specialKeywords[k]){
                
                if(dichotomies[i].balance > dichotomyThreshold){
                    dichotomies[i].balance = -1;
                }
                else{
                    dichotomiesScore += dichotomies[i].balance * dichomoiesFactor;
                }
                
            }
            else if(dichotomies[i].left == specialKeywords[k]){
                
                if(dichotomies[i].balance < -dichotomyThreshold){
                    dichotomies[i].balance = 1;
                }
                else{
                    dichotomiesScore += -dichotomies[i].balance * dichomoiesFactor;
                }
                
            }
        }
    }
    totalScore = topicsInCommonScore + topicsInCommonWithPreviousScore - samePersonOccuranceScore + dichotomiesScore;
    
    stringstream ss;
    string linkName =potentialNextClip.getLinkName();
    ofStringReplace(linkName, ",", ":");
    
    ss<<" "<<","<<" "<<","<<linkName<<","<< totalScore<<","<<topicsInCommonScore<<","<<topicsInCommonWithPreviousScore<<","<<samePersonOccuranceScore<<","<<dichotomiesScore<<endl;
    
    log = ss.str();
    
    if(printDecisions) {
        cout << "	ACCEPTED " << (link ? "LINK " : "") << totalScore << " Clip " << potentialNextClip.getLinkName() << " occurrences " << occurrences << " and " << topicsInCommon << " topics in common" << endl;
        
        //        cout<<" Score Breakdown: " << totalScore <<" = "<<topicsInCommonScore<< " + " << topicsInCommonWithPreviousScore << " - " << samePersonOccuranceScore << " + "<<dichotomiesScore<<endl;
    }
    
    
    return MAX(totalScore, 0);
}


bool CloudsStoryEngine::historyContainsClip(CloudsClip& m, vector<CloudsClip>& history){
    string clipLinkName = m.getLinkName();
    for(int i = 0; i < history.size(); i++){
        if(clipLinkName == history[i].getLinkName()){
            return true;
        }
    }
    return false;
}

int CloudsStoryEngine::occurrencesOfPerson(string person, int stepsBack, vector<CloudsClip>& history){
    int occurrences = 0;
    int startPoint = history.size() - MIN(stepsBack, history.size() );
    //	cout << "finding occurrences ... " << person << " " << clipHistory.size() << " steps back " << stepsBack << " start point " << startPoint << "/" << clipHistory.size() << endl;
    for(int i = startPoint; i < history.size()-1; i++){ // -1 because the current clip is part of history
        //		cout << "COMPARING " << clipHistory[i].person << " to " << person << endl;
        if(history[i].person == person){
            occurrences++;
        }
    }
    return occurrences;
}

CloudsEvents& CloudsStoryEngine::getEvents(){
    return events;
}
