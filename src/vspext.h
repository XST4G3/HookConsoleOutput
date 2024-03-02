#pragma once

#include <stdio.h>
#include <algorithm> 
#include <cctype>
#include <locale>
#include <eiface.h>
#include <vector>
#include <string>

#include "convar.h"
#include "tier1/tier1.h"
#include "tier2/tier2.h"
#include "engine/iserverplugin.h"

class HookConsoleOutput : public IServerPluginCallbacks
{
public:
	HookConsoleOutput() { m_iClientCommandIndex = 0; }
	~HookConsoleOutput() {}
	bool LoadFile(const char* path);

	// IServerPluginCallbacks methods
	bool Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory);
	const char* GetPluginDescription();
	void Unload(void);
	void Pause(void) {};
	void UnPause(void) {};
	void LevelInit(char const* pMapName) {};
	void ServerActivate(edict_t* pEdictList, int edictCount, int clientMax) {};
	void GameFrame(bool simulating) {};
	void LevelShutdown(void) {};
	void ClientActive(edict_t* pEntity) {};
	void ClientDisconnect(edict_t* pEntity) {};
	void ClientPutInServer(edict_t* pEntity, char const* playername) {};
	void SetCommandClient(int index) {};
	void ClientSettingsChanged(edict_t* pEdict) {};
	PLUGIN_RESULT ClientConnect(bool* bAllowConnect, edict_t* pEntity, const char* pszName, const char* pszAddress, char* reject, int maxrejectlen) { return PLUGIN_CONTINUE; };
	PLUGIN_RESULT ClientCommand(edict_t* pEntity, const CCommand& args) { return PLUGIN_CONTINUE; };
	PLUGIN_RESULT NetworkIDValidated(const char* pszUserName, const char* pszNetworkID) { return PLUGIN_CONTINUE; };
	void OnQueryCvarValueFinished(QueryCvarCookie_t iCookie, edict_t* pPlayerEntity, EQueryCvarValueStatus eStatus, const char* pCvarName, const char* pCvarValue) {};
	int GetCommandIndex() { return m_iClientCommandIndex; }
private:
	int m_iClientCommandIndex;
};

inline void ltrim(std::string& s)
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
		return !std::isspace(ch);
	}));
}

inline void rtrim(std::string& s)
{
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
		return !std::isspace(ch);
	}).base(), s.end());
}

inline void trim(std::string& s)
{
	rtrim(s);
	ltrim(s);
}

std::vector<std::string> explode(std::string separator, std::string input)
{
	std::vector<std::string> vec;
	for (size_t i{ 0 }; i < input.length(); ++i) {
		int pos = input.find(separator, i);
		if (pos < 0) { vec.push_back(input.substr(i)); break; }
		int count = pos - i;
		vec.push_back(input.substr(i, count));
		i = pos + separator.length() - 1;
	}
	return vec;
}