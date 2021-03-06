
#include "CloudsPlaybackController.h"

#ifdef KINECT_INPUT
#include "CloudsInputKinectOSC.h"
#endif

bool listsort(pair<int,string> a, pair<int,string> b){
    return a.first > b.first;
}

//--------------------------------------------------------------------
CloudsPlaybackController::CloudsPlaybackController(){
	
	loading = false;
	loadPercent = 0.0;
	loadFinished = false;
	currentAct = NULL;
	currentClip = NULL;
	numClipsPlayed = 0;
	
	shouldLoadAct = shouldPlayAct = shouldClearAct = shouldPlayClusterMap = showingClusterMapNavigation = false;
	selectedQuestion = NULL;
	selectedQuestionClip = NULL;
	forceCredits = false;
	
    numActsCreated = 0;
    
    cachedTransition = false;
    currentVisualSystem = NULL;
	rgbdVisualSystem = NULL;
	introSequence = NULL;
	clusterMap = NULL;
	interludeSystem = NULL;
    interludeStartTime = 0.0;
	
	forceInterludeReset = false;
	loadingAct = false;
	currentPresetIndex = 0;
    actCreatedTime = 0.0;
	crossfadeValue = 0.0;
	
	eventsRegistered = false;
	returnToIntro = false;

	showingIntro = false;
	showingVisualSystem = false;
	showingClusterMap = false;
    showingInterlude = false;
    exitedInterlude = false;

	bQuestionAsked = false;
	interludeExitBarWidth = 0.0;
	interludeHoveringContinue = false;
	interludeHoveringReset = false;
	interludeBarHoverStartTime = 0.0;
	interludeBarHoverHoldTime = 0.0;
	interludeBarHoverPercentComplete = 0.0;
	interludeContinueSelected = false;
	interludeResetSelected = false;
	interludeTimedOut = false;
	interludeArcRadius = 0.0;
	interludeArcBaseAlpha = 0.0;
	interludeForceOnTimer = 0.0;

	interludeStartTime = 0;
    numActsCreated = 0;
    crossfadeValue = 0;
    loadingAct = false;
	shouldLoadAct = false;
	shouldPlayClusterMap = false;
	
	loading = false;
	loadPercent = 0.0;
	
    returnToIntro = false;
    cachedTransition = false;
    showedClusterMapNavigation = false;

	resetInterludeVariables();
	
    interludeInterfaceFont.loadFont(GetCloudsDataPath()+"font/Blender-MEDIUM.ttf", 15);

}

void CloudsPlaybackController::resetInterludeVariables(){
	
	interludeExitBarWidth = 200;
	interludeBarHoverHoldTime = 3.0; //seconds
	interludeArcRadius = 40.;
	interludeArcBaseAlpha = 1.0;

	interludeHoveringContinue = false;
	interludeHoveringReset = false;
	interludeBarHoverStartTime = 0;
	interludeBarHoverPercentComplete = 0;
	interludeContinueSelected = false;
	interludeResetSelected = false;
	interludeTimedOut = false;
	interludeForceOnTimer = 50;
	
}

//--------------------------------------------------------------------
void CloudsPlaybackController::clearAct(){
	
	if(currentAct == NULL){
		return;
	}
	
	vector<CloudsVisualSystemPreset>& currentPresets = currentAct->getAllVisualSystemPresets();
    if( CloudsVisualSystemManager::HasSystemRegistered(currentVisualSystem) ){
        currentVisualSystem = NULL;
    }
    showingVisualSystem = false;
    
    visualSystems.DeallocateSystems();
    
	currentAct->unregisterEvents(this);
	currentAct->unregisterEvents(&run);
	currentAct->unregisterEvents(&hud);
	currentAct->unregisterEvents(&oscSender);
	
    delete currentAct;
    currentAct = NULL;
	clusterMap->setAct(NULL);
    
	//hack to clear frame buffer
	CloudsVisualSystem::getStaticRenderTarget().begin();
	ofClear(0,0,0);
	CloudsVisualSystem::getStaticRenderTarget().end();
	
    numActsCreated++;
}

//--------------------------------------------------------------------
void CloudsPlaybackController::exit(ofEventArgs & args){
	
	waitForThread(true);
	
	if(eventsRegistered){
		
		eventsRegistered = false;
		
		ofUnregisterMouseEvents(this);
		ofUnregisterKeyEvents(this);
		
		ofRemoveListener(ofEvents().exit, this, &CloudsPlaybackController::exit);
		ofRemoveListener(storyEngine.getEvents().actCreated, this, &CloudsPlaybackController::actCreated);
        
		ofRemoveListener(ofEvents().update, this, &CloudsPlaybackController::update);
		ofRemoveListener(ofEvents().draw, this, &CloudsPlaybackController::draw);
        
        ofRemoveListener(CloudsIntroSequence::events.portalHoverBegan, this, &CloudsPlaybackController::portalHoverBegan);
        ofRemoveListener(CloudsIntroSequence::events.portalHoverEnded, this, &CloudsPlaybackController::portalHoverEnded);
        
        ofRemoveListener(CloudsVisualSystemRGBD::events.portalHoverBegan, this, &CloudsPlaybackController::portalHoverBegan);
        ofRemoveListener(CloudsVisualSystemRGBD::events.portalHoverEnded, this, &CloudsPlaybackController::portalHoverEnded);
	}
}

//--------------------------------------------------------------------
void CloudsPlaybackController::setup(){
	
	loading = true;
	
    if(!eventsRegistered){
		
		eventsRegistered = true;
		
		ofRegisterKeyEvents(this);
		ofRegisterMouseEvents(this);
		
		ofAddListener(ofEvents().exit, this, &CloudsPlaybackController::exit);
        ofAddListener(storyEngine.getEvents().actCreated, this, &CloudsPlaybackController::actCreated);
		
		ofAddListener(ofEvents().update, this, &CloudsPlaybackController::update);
		ofAddListener(ofEvents().draw, this, &CloudsPlaybackController::draw);
        
        ofAddListener(CloudsIntroSequence::events.portalHoverBegan, this, &CloudsPlaybackController::portalHoverBegan);
        ofAddListener(CloudsIntroSequence::events.portalHoverEnded, this, &CloudsPlaybackController::portalHoverEnded);
        
        ofAddListener(CloudsVisualSystemRGBD::events.portalHoverBegan, this, &CloudsPlaybackController::portalHoverBegan);
        ofAddListener(CloudsVisualSystemRGBD::events.portalHoverEnded, this, &CloudsPlaybackController::portalHoverEnded);
        
	}
		
	cout << "*****LOAD STEP*** STARTING INTRO" << endl;
	introSequence = new CloudsIntroSequence();
	introSequence->setup();
	introSequence->setDrawToScreen(false);
	
	cout << "*****LOAD STEP*** STARTING RGBD" << endl;
	rgbdVisualSystem = new CloudsVisualSystemRGBD();
	rgbdVisualSystem->setup();
	rgbdVisualSystem->setDrawToScreen(false);
	
	cout << "*****LOAD STEP*** STARTING CLUSTER" << endl;
	clusterMap = new CloudsVisualSystemClusterMap();
	clusterMap->setup();
	clusterMap->setDrawToScreen(false);
	

	cout << "*****LOAD STEP*** STARTING HUD" << endl;
	hud.setup();

#ifdef OCULUS_RIFT
    // Link the HUD.
    rgbdVisualSystem->hud = &hud;
    rgbdVisualSystem->setupHUDGui();
    
    introSequence->hud = &hud;
    introSequence->setupHUDGui();
#endif
	
	cout << "*****LOAD STEP*** SHOWING INTRO" << endl;

	showIntro();

	cout << "*****LOAD STEP*** STARTING THREAD" << endl;
	startThread();
}

//--------------------------------------------------------------------
void CloudsPlaybackController::threadedFunction(){
	
	loadPercent = 0.0;
	
	cout << "*****LOAD STEP PARSER" << endl;

	///START THREADED
	parser.loadFromFiles();
	if(!isThreadRunning()) return;
	
	cout << "*****LOAD STEP MEDIA" << endl;
	if(ofFile::doesFileExist(GetCloudsDataPath() + "CloudsMovieDirectory.txt")){
		parser.setCombinedVideoDirectory(ofBufferFromFile(GetCloudsDataPath() + "CloudsMovieDirectory.txt").getText());
	}
	else{
		string defaultFilePath = GetCloudsDataPath(true) + "media/";
		if(ofFile::doesFileExist(defaultFilePath)){
			parser.setCombinedVideoDirectory(defaultFilePath);
		}
		else{
			ofSystemAlertDialog("Could not find movie file path. \
								Create a file called CloudsMovieDirectory.txt \
								that contains one line, the path to your movies folder");	
		}
	}

	if(!isThreadRunning()) return;
	
	visualSystems.loadPresets();
	visualSystems.loadCachedDataForSystems();
	
	if(!isThreadRunning()) return;
	
	///SOUND
	cout << "*****LOAD STEP SOUND" << endl;
	mixer.setup();
	sound.setup(storyEngine);

	sound.enterTunnel();
	
	if(!isThreadRunning()) return;
	
#ifndef  OCULUS_RIFT
	////COMMUNICATION
	oscSender.setup();
#endif
	
	clusterMap->buildEntireCluster(parser);
	
	populateRGBDPresets();
	
	//END THREADED
	loading = false;
	loadFinished = true;
}

//--------------------------------------------------------------------
void CloudsPlaybackController::finishSetup(){
	
	clusterMap->allocateFlickerTexture();
	
	storyEngine.parser = &parser;
	storyEngine.visualSystems = &visualSystems;
	storyEngine.printDecisions = false;
	storyEngine.combinedClipsOnly = true;
	storyEngine.setup();
	
	startingNodes = storyEngine.getStartingQuestions();
	introSequence->setStartQuestions(startingNodes);

}

//--------------------------------------------------------------------
void CloudsPlaybackController::populateRGBDPresets(){
#ifdef OCULUS_RIFT
	if(CloudsVisualSystem::getOculusRift().isHD()){
		basePreset = "RGBD_OC_BASE";
	}
	else{
		basePreset = "RGBD_OC_BASE_SD";
	}

	backgroundPresets.push_back("RGBD_OC_ACT1");
	pointcloudPresets.push_back("RGBD_OC_ACT1");
	
	backgroundPresets.push_back("RGBD_OC_ACT2");
	pointcloudPresets.push_back("RGBD_OC_ACT2");
	
	backgroundPresets.push_back("RGBD_OC_ACT3");
	pointcloudPresets.push_back("RGBD_OC_ACT3");
#else
	basePreset = "RGBD_BASE";
	
	backgroundPresets.push_back("RGBD_ACT1");
	pointcloudPresets.push_back("RGBD_ACT1");
	
	backgroundPresets.push_back("RGBD_ACT2");
	pointcloudPresets.push_back("RGBD_ACT2");
	
	backgroundPresets.push_back("RGBD_ACT3");
	pointcloudPresets.push_back("RGBD_ACT3");
#endif

}

//--------------------------------------------------------------------
CloudsRGBDVideoPlayer& CloudsPlaybackController::getSharedVideoPlayer(){
	return CloudsVisualSystem::getRGBDVideoPlayer();
}

//--------------------------------------------------------------------
void CloudsPlaybackController::showIntro(){
	
    float ftime = 0.1;
    ofNotifyEvent(GetCloudsAudioEvents()->fadeAudioUp, ftime);

	resetInterludeVariables();
	
#ifdef OCULUS_RIFT
    if (CloudsVisualSystem::getOculusRift().isHD()) {
	    introSequence->loadPresetGUISFromName("Oculus");
    }
    else {
        introSequence->loadPresetGUISFromName("OculusSD");
    }
#else
	introSequence->loadPresetGUISFromName("TunnelWarp");
#endif
	introSequence->playSystem();
	
	currentVisualSystem = introSequence;
	
	//HACCCK!
//	currentVisualSystem->isInterlude = true;
//	interludeSystem = currentVisualSystem;
//	interludeStartTime = ofGetElapsedTimef();
	//////////
	
	numActsCreated = 0;
    
	showingVisualSystem = true;
	showingIntro = true;
    
    oscSender.reset();
    
    hud.clearQuestion();
	hud.setHomeEnabled(false);
	hud.animateOff();
}

//--------------------------------------------------------------------
void CloudsPlaybackController::loadCurrentAct(){
	
	currentPresetIndex = 0;
	loadingAct = true;
}

//--------------------------------------------------------------------
void CloudsPlaybackController::updateLoadingAct(){
	
	if(currentAct == NULL){
		ofLogError("CloudsPlaybackController::updateLoadingAct") << "Current Act is NULL";
		return;
	}
	
	if(currentPresetIndex < currentAct->getAllVisualSystemPresets().size()){
		CloudsVisualSystemPreset& preset = currentAct->getAllVisualSystemPresets()[currentPresetIndex];
		
		preset.system = CloudsVisualSystemManager::InstantiateSystem(preset.systemName);
		if(preset.system != NULL){
			preset.system->setup();
		}
		else{
			ofLogError("CloudsPlaybackController::updateLoadingAct") << preset.systemName << " NULL right after instantiaton.";
		}

		currentPresetIndex++;
	}
	
	if(currentPresetIndex == currentAct->getAllVisualSystemPresets().size()){
		loadingAct = false;
		shouldPlayAct = true;
	}
}

//--------------------------------------------------------------------
void CloudsPlaybackController::playCurrentAct(){
	
	currentAct->registerEvents(this);
    currentAct->registerEvents(&run);
	currentAct->registerEvents(&hud);
	currentAct->registerEvents(&oscSender);
	
	currentAct->play();
}

//--------------------------------------------------------------------
void CloudsPlaybackController::keyPressed(ofKeyEventArgs & args){
	
//	if(args.key == 'Q'){
//		for(int i = 0; i < fakeQuestions.size(); i++){
//			rgbdVisualSystem->addQuestion(fakeQuestions[i],
//										  fakeQuestions[i]->getTopicsWithQuestions()[0],
//										  fakeQuestions[i]->getQuestions()[0]);
//		}
//	}
    
    
	if(args.key == '\\'){
		if(showingIntro){
			introSequence->autoSelectQuestion();
		}
        else if(showingClusterMap){
            clusterMap->getTimeline()->stop();
        }
        else if(showingInterlude){
            cout << "***** FORCING INTERLUDE OFF" << endl;
            interludeSystem->getTimeline()->stop();
        }
        else{
			if(currentAct == NULL){
				return;
			}
			
            for(int i = 0; i < currentAct->getAllClips().size(); i++){
                CloudsClip* clip = currentAct->getAllClips()[i];
                for(int i = 0; i < clip->getKeywords().size(); i++){
                    run.accumuluatedTopics[clip->getKeywords()[i]]++;
                }
                run.clipHistory.push_back( clip );
            }
            for(int i = 0; i < currentAct->getAllVisualSystemPresets().size(); i++){
                run.presetHistory.push_back( currentAct->getAllVisualSystemPresets()[i].getID() );
            }
            for(int i = 0; i < currentAct->getAllTopics().size(); i++){
                run.topicHistory.push_back( currentAct->getAllTopics()[i] );
            }
            CloudsVisualSystem::getRGBDVideoPlayer().stop();
            currentAct->getTimeline().stop();
        }
	}
    
#ifdef OCULUS_RIFT
    if(args.key == OF_KEY_RETURN){
        if(showingInterlude){
            interludeSystem->getTimeline()->stop();
        }
        else if(showingClusterMap){
            clusterMap->getTimeline()->stop();
        }
        else if(!showingIntro){
            returnToIntro = true;
            CloudsVisualSystem::getRGBDVideoPlayer().stop();
            currentAct->getTimeline().stop();
        }
    }
#endif
	
#ifdef CLOUDS_SCREENING
	if(args.key == 'Q'){
		forceCredits = true;
	}
#endif
	
    if(args.key == 'R'){
        oscSender.reset();
    }
	
	if(args.key == 'B'){
		GetCloudsAudioEvents()->respawn = true;
	}
    
}
//--------------------------------------------------------------------
void CloudsPlaybackController::createInterludeSoundQueue(){
    CloudsSoundCue cue;
	vector<int> validInterludePresetIndices;
	ofRange validRange(56,65);

//#ifdef RTC_MIX
	for(int i = 0; i < sound.presets.size(); i++){
		if(validRange.contains( sound.presets[i].slotnumber) ){
			validInterludePresetIndices.push_back(i);
		}
	}
	if(validInterludePresetIndices.size() == 0){
		ofLogError("CloudsPlaybackController::createInterludeSoundQueue") << "No Valid presets for interlude";
		return;
	}
	
	lukePreset& interludePreset = sound.presets[ validInterludePresetIndices[ ofRandom(validInterludePresetIndices.size()) ] ];
	cue.startTime = 0;
	cue.duration = 60*2;
	cue.mixLevel = 2;
    if(LUKEDEBUG) cout << "===============" << endl;
    if(LUKEDEBUG) cout << "INTERLUDE MUSIC!!!" << endl;
    if(LUKEDEBUG) cout << "===============" << endl;

    if(LUKEDEBUG) cout << "TOTAL DURATION: " << cue.duration+5.0 << endl;
    else cout << "SOUND: MUSIC STARTED." << endl;
	
    
#ifdef RTCMIX
	sound.startMusicFX(0, cue.duration+5.0);
#else
    float ftime = 0.1;
    ofNotifyEvent(GetCloudsAudioEvents()->fadeAudioUp, ftime);
#endif
	
//    if(LUKEDEBUG) cout << "   preset: " << interludePreset.slotnumber << endl;
//	sound.schedulePreset(interludePreset, cue.startTime, cue.duration, cue.mixLevel);
//#endif

	sound.schedulePreset(interludePreset, cue.startTime, cue.duration, cue.mixLevel, 0);

    if(LUKEDEBUG) cout << "====================" << endl;
    if(LUKEDEBUG) cout << "DONE INTERLUDE MUSIC!!!" << endl;
    if(LUKEDEBUG) cout << "====================" << endl;

}

//--------------------------------------------------------------------
void CloudsPlaybackController::keyReleased(ofKeyEventArgs & args){
	
}

void CloudsPlaybackController::mouseDragged(ofMouseEventArgs & args){
	
}

void CloudsPlaybackController::mouseMoved(ofMouseEventArgs & args){
    //	cursorMovedTime = ofGetElapsedTimef();
}

void CloudsPlaybackController::mousePressed(ofMouseEventArgs & args){

}

void CloudsPlaybackController::mouseReleased(ofMouseEventArgs & args){
	
}

//--------------------------------------------------------------------
void CloudsPlaybackController::update(ofEventArgs & args){

	///TESTING HACK
//	updateInterludeInterface();
	/////

    GetCloudsInput()->bUserBegan = (!showingIntro) || (showingIntro && introSequence->userHasBegun());

	if(loading){
		return;
	}
	
	if(loadFinished){
		finishSetup();
		loadFinished = false;
	}
	
    ////////////////////
	//OS CURSOR
#ifdef CLOUDS_RELEASE
    ofHideCursor();
#endif
    
	////////////////////
	//INTRO
	if(showingIntro){
    
		if(introSequence->isStartQuestionSelected()){
			
			CloudsPortal* q = introSequence->getSelectedQuestion();
			CloudsClip* clip = q->clip;
			
			map<string,string> questionsAndTopics = clip->getAllQuestionTopicPairs();
			if(questionsAndTopics.size() > 0){
				transitionController.transitionFromIntro(1.0);
			}
			else{
				ofLogError("CloudsPlaybackController::update") << "Somehow selected an intro question with no topics " << clip->getLinkName();
			}
		}
	}
	
	////////////////////
	//CLUSTER MAP
	else if(shouldPlayClusterMap){
		shouldPlayClusterMap = false;
		transitionController.transitionToClusterMap(1.0);
	}
	else if(showingClusterMap){
        if( (showingClusterMapNavigation && clusterMap->isQuestionSelected() ) ||
            (!showingClusterMapNavigation && clusterMap->finishedTraversing) )
        {
            if(showingClusterMapNavigation){
                run.questionsAsked++;
            }
            showingClusterMap = false;
            transitionController.transitionFromClusterMap(1.0);
        }
	}
    
    ////////////////////
	//INTERLUDE
	else if(showingInterlude){

		bool stopInterlude = updateInterludeInterface();
		
        if(stopInterlude){

            sound.stopMusic();
            
            if(interludeContinueSelected){
                transitionController.transitionFromInterlude(1.0);
            }
            else{
                transitionController.transitionToIntro(1.0);
            }
            
            showingInterlude = false;
        }
    }
    
	////////////////////
	// RGBD SYSTEM
    if(currentVisualSystem == rgbdVisualSystem){
        if(!transitionController.isTransitioning() && !bQuestionAsked && rgbdVisualSystem->isQuestionSelected()){
            
            bQuestionAsked = true;
            run.questionsAsked++;
            transitionController.transitionWithQuestion(2.0, 0.1);
			
        }
    }
    
    if(returnToIntro){
        returnToIntro = false;
        transitionController.transitionToIntro(1.0);
    }

	if(!showingClusterMap && !showingInterlude){
		hud.update();
	}
	
	if(shouldLoadAct){
		loadCurrentAct();
		shouldLoadAct = false;
	}
	if(loadingAct){
		updateLoadingAct();
	}
	if(shouldPlayAct){
		playCurrentAct();
		shouldPlayAct = false;
	}
	
	updateTransition();
    
}

//--------------------------------------------------------------------
void CloudsPlaybackController::updateTransition(){
	
	transitionController.update();
    
//	if(transitionController.getCurrentState() != TRANSITION_IDLE && ofGetFrameNum() % 5 == 0){
//        cout << "CURRENT STATE IS " << transitionController.getCurrentStateDescription() << " PREVIOUS STATE IS " << transitionController.getPreviousStateDescription() <<  " CROSSFADE IS " << crossfadeValue << endl;
//    }
    
	string topic;
	if(transitionController.isStateNew()){
        
        if(transitionController.fadedOut() && shouldClearAct){
            shouldClearAct = false;
            clearAct();
        }
        
        switch (transitionController.getCurrentState()) {
                
            case TRANSITION_INTERVIEW_OUT:
                if(bQuestionAsked){
                    currentAct->getTimeline().stop();
					rgbdVisualSystem->startTransitionOut( CloudsVisualSystem::QUESTION );
				}
				else{
                    rgbdVisualSystem->startTransitionOut( currentVisualSystem->getTransitionType() );
				}
                
                hud.animateOff(CLOUDS_HUD_FULL);
                break;
                
            case TRANSITION_INTRO_OUT:
                
                showingIntro = false;
                break;
                
			case TRANSITION_INTRO_IN:
                
                if(currentVisualSystem == interludeSystem){
                    cleanupInterlude();
                }
                else if(currentVisualSystem == rgbdVisualSystem){
					rgbdVisualSystem->transtionFinished();
                    rgbdVisualSystem->stopSystem();
				}
				
                if(introSequence != NULL){
                    delete introSequence;
                }

				clusterMap->clearTraversal();
				
                run.clear();
                
                introSequence = new CloudsIntroSequence();
                introSequence->setup();
				introSequence->setStartQuestions(startingNodes);
#ifdef OCULUS_RIFT
                introSequence->hud = &hud;
                introSequence->setupHUDGui();
#endif
                introSequence->setDrawToScreen(false);
                
                // TODO: Look into using Intro events for setting bDrawHud, so it works like everything else.
                hud.setHudEnabled(true);
                
                showIntro();
                break;
                
            case TRANSITION_CLUSTERMAP_IN:
				
				showClusterMap();
				break;

            case TRANSITION_VISUALSYSTEM_IN:
                
                if(transitionController.getPreviousState() == TRANSITION_INTERVIEW_OUT){
                    rgbdVisualSystem->transtionFinished();
                }
                
                playNextVisualSystem();
                break;
                
            case TRANSITION_VISUALSYSTEM_OUT:
                
                // no need to do anything special, the crossfade value will take care of this
                
                //hud.animateOff(CLOUDS_HUD_FULL);
                
                break;
                
            case TRANSITION_INTERVIEW_IN:
                
                hideVisualSystem();
                showRGBDVisualSystem();
                
                if(transitionController.getPreviousState() == TRANSITION_VISUALSYSTEM_OUT){
                    hud.playCued();
                }
                
                break;
                
            case TRANSITION_INTERLUDE_OUT:
				
                updateCompletedInterlude();
                break;
                
            case TRANSITION_INTERLUDE_IN:
                
				interludeStartTime = ofGetElapsedTimef();
				
                CloudsVisualSystem::getRGBDVideoPlayer().getPlayer().stop();
                
                if(transitionController.getPreviousState() == TRANSITION_INTERVIEW_OUT){
                    rgbdVisualSystem->transtionFinished();
                    rgbdVisualSystem->stopSystem();
                }
                else if(transitionController.getPreviousState() == TRANSITION_VISUALSYSTEM_OUT){
                    hideVisualSystem();
                }
				
				hud.setHomeEnabled(false);
                
                showInterlude();
                
				createInterludeSoundQueue();
                
                break;
                
    //////////////////
    ////////IDLE CASES
    ///////////////////
            case TRANSITION_IDLE:

                if(transitionController.getPreviousState() == TRANSITION_INTRO_IN){
                    sound.enterTunnel();
                }
				else if(transitionController.getPreviousState() == TRANSITION_INTRO_OUT){

					selectedQuestion = introSequence->getSelectedQuestion();
                    selectedQuestionClip = selectedQuestion->clip;
                    
                    showingVisualSystem = false;
					clearRenderTarget();

                    shouldPlayClusterMap = true;
					
                    introSequence->stopSystem();
					introSequence->exit();
                    
                    storyEngine.buildAct(run, selectedQuestionClip, selectedQuestion->topic, true);
                }
				else if(transitionController.getPreviousState() == TRANSITION_CLUSTERMAP_OUT){
				
					shouldLoadAct = true;
					shouldPlayAct = false;

					showingVisualSystem = false;
					clusterMap->stopSystem();
					clearRenderTarget();
					
                    if(showingClusterMapNavigation){
                        showingClusterMapNavigation = false;
                        //rgbdVisualSystem->removeQuestionFromQueue(clusterMap->getSelectedQuestion()->clip);
						vector<CloudsClip*>& screeningQueue = storyEngine.screeningQuestionClips;
						CloudsClip* interludeExitClip = clusterMap->getSelectedQuestion()->clip;
						if(ofContains(screeningQueue,interludeExitClip)){
							storyEngine.screeningQuestionClips.erase(screeningQueue.begin() + ofFind(screeningQueue, interludeExitClip));
						}
						
						//build the next act
                        storyEngine.buildAct(run,
											 clusterMap->getSelectedQuestion()->clip,
											 clusterMap->getSelectedQuestion()->topic, true);
                    }
                    
					break;
				}
				
                else if(transitionController.getPreviousState() == TRANSITION_INTERLUDE_OUT){
                    
                    cachedTransition = true;
                    cachedTransitionType = interludeSystem->getTransitionType();

                    cleanupInterlude();
                    
                    //build the next clip based on the history
                    #ifdef CLOUDS_SCREENING
                    if(run.questionsAsked > 2 && !showedClusterMapNavigation){
						createInterludeSoundQueue();
						shouldPlayClusterMap = true;
                        showingClusterMapNavigation = true;
                    }
                    else{
                        storyEngine.buildAct(run);
                    }
                    #else
					storyEngine.buildAct(run);
                    #endif
					
                    cout << "IDLE POST TRANSITION INTERLUDE OUT" << endl;
                }
				else if(transitionController.getPreviousState() == TRANSITION_INTERVIEW_OUT){

					if(bQuestionAsked){
                        
                        selectedQuestion = rgbdVisualSystem->getSelectedQuestion();
                        selectedQuestionClip = selectedQuestion->clip;
                        topic = selectedQuestion->topic;
                        
                        rgbdVisualSystem->transtionFinished();
                        rgbdVisualSystem->clearQuestions();
                        rgbdVisualSystem->stopSystem();
                        
                        CloudsVisualSystem::getRGBDVideoPlayer().stop();
                        CloudsVisualSystem::getRGBDVideoPlayer().maxVolume = 1.0;
                        
                        crossfadeValue = 0;
                        
                        shouldPlayClusterMap = run.questionsAsked % 3 == 2;
                        
                        storyEngine.buildAct(run, selectedQuestionClip, topic);

                        bQuestionAsked = false;
                    }
				}
				
                //we just finished fading out of the interview
                else if(transitionController.getPreviousState() == TRANSITION_INTERVIEW_IN){
                    rgbdVisualSystem->transtionFinished();
                }
				else if(transitionController.getPreviousState() == TRANSITION_VISUALSYSTEM_IN){
					if(currentVisualSystem == clusterMap){
						playNextVisualSystem();
						ofLogError("TRANSITIONED WITHOUT SYSTEM STARTING");
					}
                }                
                break;
				
            default:
                break;
        }

		if(transitionController.fadedOut()){
			clearRenderTarget();
		}
	}
    
    crossfadeValue = transitionController.getFadeValue();

    //if(bQuestionAsked || shouldLoadAct || loadingAct){
	if(bQuestionAsked){	
        CloudsVisualSystem::getRGBDVideoPlayer().maxVolume = crossfadeValue;
        //don't start fading out right away
        crossfadeValue = ofMap(crossfadeValue, .2, 0.0, 1.0, 0.0, true);
    }
	rgbdVisualSystem->visualSystemFadeValue = crossfadeValue;
    
	if(transitionController.transitioning){
		rgbdVisualSystem->updateTransition( transitionController.getInterviewTransitionPoint() );
	}   
}

//--------------------------------------------------------------------
void CloudsPlaybackController::clearRenderTarget(){
	CloudsVisualSystem::getStaticRenderTarget().begin();
	ofClear(0,0,0);
	CloudsVisualSystem::getStaticRenderTarget().end();
}

//--------------------------------------------------------------------
bool CloudsPlaybackController::updateInterludeInterface(){
	//hack
//	if(interludeSystem == NULL){
//		interludeSystem = currentVisualSystem;
//	}
	//hack
	
#ifdef OCULUS_RIFT

    interludeContinueSelected = interludeHoveringContinue;
	interludeResetSelected = interludeHoveringReset;

	if(currentVisualSystem->resetNode.finished){
		interludeResetSelected = true;
		return true;
	}
	if(currentVisualSystem->continueNode.finished){
		interludeContinueSelected = true;
		return true;
	}
	if(ofGetElapsedTimef() - interludeStartTime > 30){
		interludeResetSelected = true;
		return true;
	}
	return false;
#else
	
    if( currentVisualSystem->getSystemName() == "Balloons" ){
        hud.clearQuestion();
        return false;
    }
    
	interludeTimedOut = ofGetElapsedTimef() - interludeStartTime > interludeForceOnTimer;
	
	if(GetCloudsInputX() > interludeSystem->getCanvasWidth() - interludeExitBarWidth)
	{
		if(!interludeHoveringContinue){
			interludeHoveringContinue = true;
			interludeBarHoverStartTime = ofGetElapsedTimef();
			//TODO PLAY SOUND
			CloudsVisualSystem::getClick()->setPosition(0);
			CloudsVisualSystem::getClick()->play();
		}
	}
	else{
		interludeHoveringContinue = false;
	}
	
	if(GetCloudsInputX() < interludeExitBarWidth){
		if(!interludeHoveringReset){
			interludeHoveringReset = true;
			interludeBarHoverStartTime = ofGetElapsedTimef();
			CloudsVisualSystem::getClick()->setPosition(0);
			CloudsVisualSystem::getClick()->play();
		}
	}
	else{
		interludeHoveringReset = false;
	}
	
	if(interludeHoveringContinue || interludeHoveringReset){
		interludeBarHoverPercentComplete = ofMap(ofGetElapsedTimef(),
												 interludeBarHoverStartTime,interludeBarHoverStartTime+interludeBarHoverHoldTime,
												 0.0, 1.0, true);
//		cout << " interludeBarHoverPercentComplete " << interludeBarHoverPercentComplete << endl;
		if(interludeBarHoverPercentComplete == 1.0){
			
			///one of these will be true
			interludeContinueSelected = interludeHoveringContinue;
			interludeResetSelected = interludeHoveringReset;
			CloudsVisualSystem::getSelectMid()->setPosition(0);
			CloudsVisualSystem::getSelectMid()->play();
			return true;
		}
	}
	else {
		//slowly attenuate it back down
		interludeBarHoverPercentComplete *= 0.995;
		interludeBarHoverPercentComplete = MAX(0.0,interludeBarHoverPercentComplete-.001);
//		interludeBarHoverPercentComplete = 0;
	}
	
	if( interludeSystem->getSystemName() != "Balloons" && ofGetElapsedTimef() - interludeStartTime > 60){
		interludeResetSelected = true;
		return true;
	}
    
	return false;
#endif
	
}

//--------------------------------------------------------------------
void CloudsPlaybackController::updateCompletedInterlude(){

}

//--------------------------------------------------------------------
void CloudsPlaybackController::drawInterludeInterface(){
    

    if(currentVisualSystem->getSystemName() == "Balloons"){
        return;
    }
    
	ofRectangle hoverRect;
	bool hovering = false;
	string promptType;
	int tracking;
	if(interludeSystem != NULL){

		hoverRect = ofRectangle(interludeSystem->getCanvasWidth(), 0,
								-interludeExitBarWidth, interludeSystem->getCanvasHeight());
		hovering = interludeHoveringContinue;
		promptType = "CONTINUE";
		tracking = 6;
		drawInterludePanel(hoverRect, promptType, hovering, tracking);
        
		hoverRect = ofRectangle(0, 0, interludeExitBarWidth, interludeSystem->getCanvasHeight());
		hovering = interludeHoveringReset;
		promptType = "RESET";
		tracking = 11;
		drawInterludePanel(hoverRect, promptType, hovering, tracking);

	}
}

void CloudsPlaybackController::drawInterludePanel(ofRectangle hoverRect, string promptType, bool hovering, int tracking ){

	ofPushStyle();
	ofEnableAlphaBlending();
	
	ofFill();
	
	float forceExtendPercent = ofMap(ofGetElapsedTimef(),
									 interludeStartTime+interludeForceOnTimer,
									 interludeStartTime+interludeForceOnTimer+2.0,
									 0.0, 1.0, true);
	float percentExtended = MAX(interludeBarHoverPercentComplete, forceExtendPercent);
	float alpha;
	if(hovering){
		hoverRect.width *= ofxTween::map(percentExtended, 0.0, 0.2, 0.0, 1.0, true, ofxEasingQuad(), ofxTween::easeOut);
		alpha = ofxTween::map(percentExtended, 0.0, .3, 0.0, 1.0, true, ofxEasingQuad(), ofxTween::easeOut);
	}
	else{
		hoverRect.width *= ofxTween::map(percentExtended, 0.7, 0.9, 0.0, 1.0, true, ofxEasingQuad(), ofxTween::easeOut);
		alpha = ofxTween::map(percentExtended, 0.7, 0.9, 0.0, 1.0, true, ofxEasingQuad(), ofxTween::easeOut);
	}
	ofFloatColor shelfColor = ofFloatColor(1.0, alpha*powf(crossfadeValue,2.0f) * .2);
	if(interludeSystem->getBgColor().getBrightness() > 100){
		ofSetColor(0, shelfColor.a*255);
	}
	else{
		ofSetColor(255, shelfColor.a*255);
	}
	ofRect(hoverRect);
	ofVec2f cursorPosition = GetCloudsInputPosition();
	float halfWidth = hoverRect.getStandardized().getWidth()*.5;
	cursorPosition.x = ofClamp(cursorPosition.x, halfWidth, interludeSystem->getCanvasWidth() - halfWidth);
	cursorPosition.y = ofClamp(cursorPosition.y, interludeArcRadius*2.2, interludeSystem->getCanvasHeight() - interludeArcRadius*2.2);
	
	//force extend after interludeForceOnTimer times out
	if(percentExtended > 0.0){
		//draw basic circle
		ofNoFill();
		ofFloatColor arcColor = ofFloatColor(1.0, 100/255.0,100/255.0, crossfadeValue*powf(crossfadeValue,2.0f) );
		if(hovering){
			ofSetColor(arcColor, 100*crossfadeValue*alpha);
		}else{
			ofSetColor(1.0, 100*crossfadeValue*alpha) ;
		}
		ofCircle(cursorPosition, interludeArcRadius*crossfadeValue);
		interludeInterfaceFont.setTracking(tracking);
		float typeWidth  = interludeInterfaceFont.stringWidth(promptType);
		float typeHeight = interludeInterfaceFont.stringHeight(promptType);
		//draw type
		ofSetColor(255, 255*alpha*crossfadeValue);
		interludeInterfaceFont.drawString(promptType,
										  hoverRect.getCenter().x - typeWidth/2 - 10,
										  interludeSystem->getCanvasHeight()*.5 + typeHeight + 10);

		if(hovering){
			float arcPercent = ofxTween::map(interludeBarHoverPercentComplete,
											 0.0, 1.0, 0.0, 1.0,
											 true, ofxEasingQuad(), ofxTween::easeOut);
			
			ofPath arc;
			arc.setFilled(false);
			arc.setStrokeWidth(4);
			arc.setStrokeColor(arcColor);
			float expandedArcRadius = interludeArcRadius + powf(1.0-crossfadeValue,2.0f) * 40; //expand it beyond when it's finished
			arc.moveTo(cursorPosition.x, cursorPosition.y - expandedArcRadius, 0);
			arc.arc(cursorPosition, expandedArcRadius, expandedArcRadius, -90, 360*arcPercent-90, true);
			arc.draw();
		}
		
	}
	
	ofPopStyle();
}

#ifdef KINECT_INPUT
//--------------------------------------------------------------------
void CloudsPlaybackController::drawKinectFeedback(){
    
    ofPtr<CloudsInputKinectOSC> kinectInput = dynamic_pointer_cast<CloudsInputKinectOSC>(GetCloudsInput());
    kinectInput->draw();
}
#endif

//This is where everything in clouds is drawn
//--------------------------------------------------------------------
void CloudsPlaybackController::draw(ofEventArgs & args){
    
    ofBackground(0);
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_DEPTH_TEST);
	
    drawRenderTarget();
    
    drawInterludeInterface();
    
#ifdef KINECT_INPUT
    drawKinectFeedback();
#endif
    
	drawDebugOverlay();
	
	glPopAttrib();
}

void CloudsPlaybackController::drawRenderTarget(){
	if(currentVisualSystem != NULL){
		
        ofPushStyle();
        ofEnableAlphaBlending();
        
		//cout << "crosffade value is " << crossfadeValue << " showing intro? " << showingIntro << endl;
		if(loading){
			crossfadeValue = 1.0;
		}
		ofSetColor(255, crossfadeValue*255 );
		
		currentVisualSystem->selfPostDraw();
        
        ofEnableAlphaBlending();		
        CloudsVisualSystem::getRGBDVideoPlayer().drawSubtitles();
        
#ifndef OCULUS_RIFT
		hud.draw();
#endif
		ofPopStyle();
	}
    else{
#ifdef CLOUDS_APP
		ofBackground(0, 0, 0);
#else
        ofBackground(0, 0, 255);
#endif
    }
}



//--------------------------------------------------------------------
void CloudsPlaybackController::drawDebugOverlay(){
    
	if(currentAct == NULL){
		return;
	}
	
	if(ofGetKeyPressed('-')){
		currentAct->getTimeline().enableEvents();
		currentAct->drawDebug();
	}
	else{
		currentAct->getTimeline().disableEvents();
	}
	

	
}

#pragma story engine events
//--------------------------------------------------------------------
void CloudsPlaybackController::actCreated(CloudsActEventArgs& args){
	
    if(currentAct != NULL){
        clearAct();
    }
    
	rgbdVisualSystem->clearQuestions();
	
	numClipsPlayed = 0;
	currentAct = args.act;
	if(shouldPlayClusterMap){
		clusterMap->setAct(currentAct);
	}
	else {
		shouldLoadAct = true;
		shouldPlayAct = false;
	}
	
	cout << "***** ORDER OF OPERATIONS: ACT CREATED CONTROLLER " << args.act << endl;

}

//--------------------------------------------------------------------
void CloudsPlaybackController::actBegan(CloudsActEventArgs& args){
    cout << "***** ORDER OF OPERATIONS: ACT BEGAN CONTROLLER " << args.act << endl;
	
	resetInterludeVariables();
	
    if(!args.act->startsWithVisualSystem()){
        transitionController.transitionToFirstInterview(1.0);
    }
}

//--------------------------------------------------------------------
void CloudsPlaybackController::actEnded(CloudsActEventArgs& args){
	
	if(!returnToIntro){
		shouldClearAct = true;
		
		cout << "ACT ENDED TRIGGERED" << endl;
		if(!bQuestionAsked){
			cout << "Transitioning to interlude/clustermap" << endl;
			transitionController.transitionToInterlude(1.0,1.0);
		}
	}
}

//--------------------------------------------------------------------
void CloudsPlaybackController::clipBegan(CloudsClipEventArgs& args){
	playClip(args.chosenClip);
}

//--------------------------------------------------------------------
void CloudsPlaybackController::visualSystemBegan(CloudsVisualSystemEventArgs& args)
{
	if(showingVisualSystem){
		ofLogError("CloudsPlaybackController::visualSystemBegan") << "Triggered visual system while still showing one";
	}
	
    if(bQuestionAsked){
        //Don't run the system
        return;
    }
    
	//rgbdVisualSystem->clearQuestions();
	nextVisualSystemPreset = args.preset;
    
	//	cout << "CloudsPlaybackController::showVisualSystem SETTING NEXT SYSTEM TO " << nextVisualSystem.presetName << endl;
	if(nextVisualSystemPreset.system == NULL){
		ofLogError("CloudsPlaybackController::showVisualSystem") << "Incoming system is NULL";
	}
	
	if(currentVisualSystem == introSequence ||
	   currentVisualSystem == clusterMap ||
       exitedInterlude)
	{
		transitionController.transitionToFirstVisualSystem(1.0);
        exitedInterlude = false;
	}
	else if(currentVisualSystem == rgbdVisualSystem) {
		transitionController.transitionToVisualSystem(1.0, 1.0);
	}
	else {
		playNextVisualSystem();
	}
}

//--------------------------------------------------------------------
void CloudsPlaybackController::visualSystemEnded(CloudsVisualSystemEventArgs& args){
	if(showingVisualSystem){
		float fadeDuration = 1;
		transitionController.transitionToInterview(fadeDuration, 1.0);
	}
	else {
		ofLogError("CloudsPlaybackController::visualSystemEnded") << "Hiding visual system while none is showing";
	}
}

//--------------------------------------------------------------------
void CloudsPlaybackController::questionProposed(CloudsQuestionEventArgs& args){
	//don't ask a topic that we've already seen
	if(!ofContains(run.topicHistory, args.topic) &&
       !run.historyContainsClip(args.questionClip))
    {
		rgbdVisualSystem->addQuestion(args.questionClip,
                                      args.topic,
                                      args.question);
	}
}

void CloudsPlaybackController::questionSelected(CloudsQuestionEventArgs& args){
    
}

//--------------------------------------------------------------------
void CloudsPlaybackController::topicChanged(CloudsTopicEventArgs& args){
	currentTopic = args.topic;
}

//--------------------------------------------------------------------
void CloudsPlaybackController::preRollRequested(CloudsPreRollEventArgs& args){
	cout << "PREROLLING CLIP TO " << 1. - args.handleLength << endl;
	prerollClip(args.preRollClip, 1. - args.handleLength);
}

//--------------------------------------------------------------------
void CloudsPlaybackController::prerollClip(CloudsClip* clip, float toTime){

	if(clip == NULL){
		ofLogError("CloudsPlaybackController::prerollClip") << "clip IS NULL ";
		return;
	}

	if(!clip->hasMediaAsset){
		ofLogError("CloudsPlaybackController::prerollClip") << "clip " << clip->getLinkName() << " doesn't have combined video";
		return;
	}
	
	bool clipLoadSuccessfullyLoaded = false;
	if(clip->voiceOverAudio){
		clipLoadSuccessfullyLoaded = CloudsVisualSystem::getRGBDVideoPlayer().setupVO(clip->voiceOverAudioPath);
	}
	else{
		clipLoadSuccessfullyLoaded = CloudsVisualSystem::getRGBDVideoPlayer().setup(clip->combinedVideoPath,
																					clip->combinedCalibrationXMLPath,
																					GetCloudsDataPath() + "subtitles/"+ clip->getSubtitlesPath(),
																					1.0, clip->getSpeakerVolume());
	}
    
	if(!clipLoadSuccessfullyLoaded){
		ofLogError("CloudsPlaybackController::prerollClip") << "Error loading clip " << clip->getLinkName() << " file path " << clip->combinedVideoPath;
		return;
	}
    
	prerolledClipID = clip->getID();
}

//--------------------------------------------------------------------
void CloudsPlaybackController::playClip(CloudsClip* clip){
    
	numClipsPlayed++;
	
	cout << "**** CLIP BEGAN" << endl;

	if(clip->getID() != prerolledClipID){
		cout << "**** HAD TO MANUALLY PREROLL IN CLIP BEGAN" << endl;
		prerollClip(clip, 1);
	}
	
	rgbdVisualSystem->setupSpeaker( 
		clip->getSpeakerFirstName(),
		clip->getSpeakerLastName(), 
		clip->name);
	
	prerolledClipID = "";
	currentClip = clip;
	currentClipName = clip->getID();
    
	cout << "**** SWAPPING IN FROM CLIP BEGAN" << endl;
	rgbdVisualSystem->getRGBDVideoPlayer().swapAndPlay();
}

//--------------------------------------------------------------------
void CloudsPlaybackController::showClusterMap(){
    if(showingClusterMapNavigation){
        #ifdef CLOUDS_SCREENING
		rgbdVisualSystem->clearQuestionQueue();
		storyEngine.populateScreeningQuestionsPart2();
		clusterMap->setQuestions(storyEngine.screeningQuestionClips);
		showedClusterMapNavigation = true;
		clusterMap->loadPresetGUISFromName("NavigationInterlude_Screen");
        #else
        //SHOW QUESTIONS FROM CURRENT ACT
        showingClusterMapNavigation = false; //TEMP HACK UNTIL WE GET THIS WORKING ON NON SCREENING MODE
        #endif
    }
    
    if(!showingClusterMapNavigation){
        #ifdef OCULUS_RIFT
        if(CloudsVisualSystem::getOculusRift().isHD()){
            clusterMap->loadPresetGUISFromName("FollowTraverse_OculusHD");
        }
        else{
            clusterMap->loadPresetGUISFromName("FollowTraverse_OculusSD");
        }
        #else
        clusterMap->loadPresetGUISFromName("FollowTraverse_Screen");
        #endif
    }
    
    clusterMap->playSystem();
    clusterMap->clearTraversal();

    if(!showingClusterMapNavigation){
        clusterMap->autoTraversePoints = true;
        
        clusterMap->traverse();
        clusterMap->traverse();
    }
    else{
        clusterMap->autoTraversePoints = false;
    }
    
    currentVisualSystem = clusterMap;

    showingVisualSystem = true;
    showingClusterMap = true;
}

//--------------------------------------------------------------------
void CloudsPlaybackController::showInterlude(){
    
	resetInterludeVariables();
	
    vector<string> topics;
    CloudsVisualSystemPreset interludePreset;
	#ifdef CLOUDS_SCREENING
	if(rgbdVisualSystem->hasQuestionsRemaining() && showedClusterMapNavigation){
		forceCredits = true;
	}
	#endif
    if(storyEngine.getPresetIDForInterlude(run, interludePreset, forceCredits)){
        
        interludeSystem = CloudsVisualSystemManager::InstantiateSystem(interludePreset.systemName);
        
        interludeSystem->setDrawToScreen( false );
        interludeSystem->setup();
        interludeSystem->loadPresetGUISFromName( interludePreset.presetName );
        interludeSystem->playSystem();
        interludeSystem->isInterlude = true;
		
        currentVisualSystem = interludeSystem;
        
        showingInterlude = true;

    }
    else{
        ofLogError("CloudsPlaybackController::showInterlude") << "Defaulting to cluster map because we found no topics from the last act";
//        showClusterMap();
        returnToIntro = true;
    }
}

//--------------------------------------------------------------------
void CloudsPlaybackController::cleanupInterlude(){
    showingVisualSystem = false;
	clearRenderTarget();

    if(currentVisualSystem == clusterMap) {
        clusterMap->stopSystem();
    }
    else if(currentVisualSystem != NULL && currentVisualSystem == interludeSystem){
        interludeSystem->stopSystem();
        interludeSystem->exit();
		
		interludeSystem = NULL;
		
		hud.clearQuestion();
        exitedInterlude = true;
    }
    else {
        ofLogError("CloudsPlaybackController::cleanupInterlude") << " Ended interulde while not showing ClusterMap or Interlude System";
    }
}

//--------------------------------------------------------------------
void CloudsPlaybackController::hideVisualSystem() {
	if(showingVisualSystem){
		currentVisualSystem->stopSystem();
		showingVisualSystem = false;
	}
}

void CloudsPlaybackController::showRGBDVisualSystem(){

	rgbdVisualSystem->loadPresetGUISFromName(basePreset);
    if(numActsCreated < 3){
		rgbdVisualSystem->loadBackgroundGUISFromName(backgroundPresets[numActsCreated]);
		rgbdVisualSystem->loadPointcloudGUISFromName(pointcloudPresets[numActsCreated]);
    }
	else{
		rgbdVisualSystem->loadBackgroundGUISFromName(backgroundPresets[ ofRandom(backgroundPresets.size()) ]);
		rgbdVisualSystem->loadPointcloudGUISFromName(pointcloudPresets[ ofRandom(pointcloudPresets.size()) ]);
	}
    
    if(currentVisualSystem == NULL){
        rgbdVisualSystem->startTransitionIn( CloudsVisualSystemRGBD::FLY_THROUGH );
    }
    else{
        if(cachedTransition){
            rgbdVisualSystem->startTransitionIn( cachedTransitionType );
            cachedTransition = false;
        }
        else {
            rgbdVisualSystem->startTransitionIn( currentVisualSystem->getTransitionType() );
        }
    }
	
	rgbdVisualSystem->playSystem();
	
	hud.setHomeEnabled(false);
	
	currentVisualSystem = rgbdVisualSystem;
}

void CloudsPlaybackController::playNextVisualSystem()
{
	if(nextVisualSystemPreset.system != NULL){
		
		if(rgbdVisualSystem->isPlaying()){
			rgbdVisualSystem->stopSystem();
		}
		
		nextVisualSystemPreset.system->setDrawToScreen( false );
		nextVisualSystemPreset.system->loadPresetGUISFromName( nextVisualSystemPreset.presetName );
		nextVisualSystemPreset.system->playSystem();
		
		currentVisualSystemPreset = nextVisualSystemPreset;
		currentVisualSystem = nextVisualSystemPreset.system;
		
        cachedTransition = false;
        
		showingVisualSystem = true;
	}
	else{
		ofLogError("CloudsPlaybackController::playNextVisualSystem") << "nextVisualSystemPreset == NULL";
	}
}

#pragma mark - Visual System Event Callbacks

//--------------------------------------------------------------------
void CloudsPlaybackController::portalHoverBegan(CloudsPortalEventArgs &args){
    hud.questionHoverOn(args.question);
}

//--------------------------------------------------------------------
void CloudsPlaybackController::portalHoverEnded(CloudsPortalEventArgs &args){
	hud.questionHoverOff();
	if(args.question == "RESET"){
		returnToIntro = true;
		CloudsVisualSystem::getRGBDVideoPlayer().stop();
		currentAct->getTimeline().stop();		
	}
}
