/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __QUESTLOG_H_
#define __QUESTLOG_H_

#include <list>
#include <map>
#include <string>

#include "mthread.h"


struct Quest {
	std::string name;
	bool completed;
	
	Quest() {
		name = "";
		completed = false;
	}
};

struct Mission {
	std::string name;
	std::string description;
	
	Mission() {
		name = "";
		description = "";
	}
};


typedef std::pair<unsigned short, Quest> QuestPair;
typedef std::map<unsigned short, Quest> QuestsMap;
typedef std::list<Mission> MissionsList;

class QuestLog {
private:
	QuestsMap quests;
	MissionsList missions;
	
	MUTEX lockQuestLog;
	
public:
	QuestLog();
	~QuestLog();
	
	void AddQuest(unsigned short id, Quest quest);
	QuestPair GetQuestByNumber(int number);
	QuestsMap GetQuests();
	void ClearQuests();
	
	void AddMission(Mission mission);
	Mission GetMissionByNumber(int number);
	MissionsList GetMissions();
	void ClearMissions();
};


#endif //__QUESTLOG_H_
