//
//  keywordDichotomy.h
//  CloudsStoryEngine
//
//  Created by Surya Mattu on 22/07/13.
//
//

#ifndef __CloudsStoryEngine__keywordDichotomy__
#define __CloudsStoryEngine__keywordDichotomy__

#include "ofMain.h"
class CloudsDichotomy{
public:
    CloudsDichotomy();
	
	static vector<CloudsDichotomy> getDichotomies();
    string left;
    string right;
    int balance; //pos/neg
};
#endif /* defined(__CloudsStoryEngine__keywordDichotomy__) */
