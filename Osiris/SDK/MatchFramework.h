#pragma once

#include "AppSystem3.h"
#include "MatchSession.h"
#include "KeyValues.h"

class matchExtensions;
class matchEventsSubscription;
class matchTitle;
class matchNetworkMsgController;
class matchSystem;

class MatchFramework : AppSystem3 {
public:
	virtual void runFrame() = 0; //9
	virtual matchExtensions* getMatchExtensions() = 0; //10
	virtual matchExtensions* getEventsSubscription() = 0; //11
	virtual matchTitle* getMatchTitle() = 0; //12
	virtual MatchSession* getMatchSession() = 0; //13
	virtual matchNetworkMsgController* getMsgController() = 0; //14
	virtual matchSystem* getMatchSystem() = 0; //15
	virtual void createSession(KeyValues* settings) = 0; //16
	virtual void matchMessions(KeyValues* settings) = 0; //17
	virtual void acceptInvite(int controller) = 0; //18
	virtual void closeSession() = 0; //19
};