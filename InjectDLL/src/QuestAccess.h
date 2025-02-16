#pragma once

namespace MemoryAccess
{
	class QuestAccess 
	{
		std::vector<std::string> ServerSettings;
        bool IsQuestSync;
        bool eventStatus = false;
        bool questSyncUsingEvent = false;

	public:
		Memory::Quests_class* QuestSyncer = new Memory::Quests_class();

		void Startup(bool isQuestSync, std::vector<std::string> questServerSettings)
		{
			this->ServerSettings = questServerSettings;
            this->IsQuestSync = isQuestSync;
		}

        DTO::QuestDTO* UpdateQuests()
        {
            DTO::QuestDTO* result = new DTO::QuestDTO();

            if (IsQuestSync)
            {
                QuestSyncer->readQuests();
                result->Completed = QuestSyncer->getChangedQuests();
            }

            return result;
        }

		void SetServerData(std::vector<std::string> questData, bool paused, bool questSyncReady)
		{
            if (IsQuestSync)
            {
                for (int i = 0; i < questData.size(); i++)
                    if (std::find(QuestSyncer->questsToChange.begin(), QuestSyncer->questsToChange.end(), questData[i]) == QuestSyncer->questsToChange.end())
                        QuestSyncer->questsToChange.push_back(questData[i]);

                if (questSyncReady) questSyncUsingEvent = QuestSyncer->updateQuests(eventStatus);

                if (!paused)
                {
                    QuestSyncer->resyncQuests();
                }

                eventStatus = questSyncUsingEvent;
            }
            else
            {
                QuestSyncer->QuestMutex.lock();

                QuestSyncer->serverQuests.clear();
                QuestSyncer->koroksToAdd = 0;
                QuestSyncer->boolsToChange.clear();
                QuestSyncer->itemsToAdd.clear();
                QuestSyncer->intsToChange.clear();

                for (auto const& pair : QuestSyncer->numberOfQuests)
                {
                    std::string QType = pair.first;
                    int QNumber = pair.second;

                    for (int i = 0; i < QNumber; i++)
                    {
                        QuestSyncer->QuestList[QType + std::to_string(i)].Value = 0;
                        QuestSyncer->QuestList[QType + std::to_string(i)].beingChanged = false;
                    }
                }

                QuestSyncer->changedQuests.clear();
                QuestSyncer->QuestMutex.unlock();
            }
		}
	};
}