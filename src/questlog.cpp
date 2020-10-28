/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include "questlog.h"

#include "allocator.h"


// ---- QuestLog ---- //

QuestLog::QuestLog() { }

QuestLog::~QuestLog() { }

void QuestLog::AddQuest(unsigned short id, Quest quest) {
	LOCKCLASS lockClass(lockQuestLog);

	quests[id] = quest;
}

QuestPair QuestLog::GetQuestByNumber(int number) {
	LOCKCLASS lockClass(lockQuestLog);

	int i = 0;

	QuestsMap::iterator it = quests.begin();
	for (it, i; it != quests.end(); it++, i++) {
		if (i == number)
			return QuestPair(it->first, it->second);
	}

	return QuestPair(0, Quest());
}

QuestsMap QuestLog::GetQuests() {
	LOCKCLASS lockClass(lockQuestLog);

	return quests;
}

void QuestLog::ClearQuests() {
	LOCKCLASS lockClass(lockQuestLog);

	quests.clear();
}

void QuestLog::AddMission(Mission mission) {
	LOCKCLASS lockClass(lockQuestLog);

	missions.push_back(mission);
}

MissionsList QuestLog::GetMissions() {
	LOCKCLASS lockClass(lockQuestLog);

	return missions;
}

Mission QuestLog::GetMissionByNumber(int number) {
	LOCKCLASS lockClass(lockQuestLog);

	int i = 0;

	MissionsList::iterator it = missions.begin();
	for (it, i; it != missions.end(); it++, i++) {
		if (i == number)
			return *it;
	}

	return Mission();
}

void QuestLog::ClearMissions() {
	LOCKCLASS lockClass(lockQuestLog);

	missions.clear();
}
