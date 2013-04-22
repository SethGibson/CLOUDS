
#include "CloudsPlaybackController.h"

CloudsPlaybackController::CloudsPlaybackController(){
	eventsRegistered = false;
	playingCombinedVideo = false;
	currentClip = NULL;
}

CloudsPlaybackController::~CloudsPlaybackController(){
	if(eventsRegistered){
		eventsRegistered = false;
		ofRemoveListener(storyEngine->getEvents().storyBegan, this, &CloudsPlaybackController::storyBegan);
		ofRemoveListener(storyEngine->getEvents().clipChanged, this, &CloudsPlaybackController::clipChanged);
	}
}

void CloudsPlaybackController::setup(CloudsStoryEngine& storyEngine){
	if(!eventsRegistered){
		this->storyEngine = &storyEngine;
		ofAddListener(storyEngine.getEvents().storyBegan, this, &CloudsPlaybackController::storyBegan);
		ofAddListener(storyEngine.getEvents().clipChanged, this, &CloudsPlaybackController::clipChanged);
		eventsRegistered = true;
	}
}

void CloudsPlaybackController::storyBegan(CloudsStoryEventArgs& args){
	playClip(*args.chosenClip);
}

void CloudsPlaybackController::clipChanged(CloudsStoryEventArgs& args){
	playClip(*args.chosenClip);	
}

void CloudsPlaybackController::playClip(CloudsClip& clip){
	if(clip.hasCombinedVideo){
		if(player.loadMovie(clip.combinedVideoFilePath)){
			playingCombinedVideo = true;
			renderer.setup(clip.combinedVideoCalibrationXml);
			renderer.setTexture(player);
		}
	}
	else{
		playingCombinedVideo = false;
		player.loadMovie(clip.sourceVideoFilePath);
	}
	
	player.setFrame(clip.startFrame);
	player.play();
}

void CloudsPlaybackController::update(){
		
}

void CloudsPlaybackController::draw(){
	if(player.isPlaying()){
		if(playingCombinedVideo){
			renderer.drawPointCloud();
		}
		else {
			player.draw(0,0,960,540);
		}
	}
}

