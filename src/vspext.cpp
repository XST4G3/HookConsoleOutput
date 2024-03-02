#include "vspext.h"

#include <chrono>
#include <thread>

HookConsoleOutput g_HookConsoleOutput;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(HookConsoleOutput, IServerPluginCallbacks, INTERFACEVERSION_ISERVERPLUGINCALLBACKS, g_HookConsoleOutput);

#define FILE_WORDS "/addons/HookConsoleOutput/HookConsoleOutput.ini"

IVEngineServer* engine;
std::vector<std::string> words;
std::vector<SpewType_t> flags;

const char* szSpewType[] = {
	"Message",
	"Warning",
	"Assert",
	"Error",
	"Log"
};

SpewRetval_t HookOutput(SpewType_t spewType, const tchar* pMsg)
{
	std::string message{ pMsg };

	bool foundWord = std::find_if(words.begin(), words.end(), [&message](std::string str)
	{
		return message.find(str) != std::string::npos;
	}) != words.end();

	bool foundFlags = std::find_if(flags.begin(), flags.end(), [&spewType](SpewType_t type)
	{
		return spewType == type;
	}) != flags.end();

	if (!foundWord && !foundFlags)
		printf("[%s] %s", szSpewType[spewType], pMsg);

	return SPEW_CONTINUE;
}

bool HookConsoleOutput::Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory)
{
	ConnectTier1Libraries(&interfaceFactory, 1);
	ConnectTier2Libraries(&interfaceFactory, 1);

	engine = (IVEngineServer*)interfaceFactory(INTERFACEVERSION_VENGINESERVER, NULL);
	if (!engine)
	{
		Warning("Unable to load IVEngineServer, ignoring\n");

		return false;
	}

	std::thread { [] {
		while (true)
		{
			if (GetSpewOutputFunc() != HookOutput)
			{
				SpewOutputFunc(HookOutput);
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	} }.detach();

	ConVar_Register(0);

	return LoadFile(FILE_WORDS);
}

bool HookConsoleOutput::LoadFile(const char* path)
{
	char buffer[256];
	engine->GetGameDir(buffer, sizeof(buffer));

	words.clear();
	flags.clear();

	strcat(buffer, path);
	FILE* fp = fopen(buffer, "r");

	if (fp != NULL)
	{
		while (fgets(buffer, sizeof(buffer), fp) != NULL)
		{
			std::string line{ buffer };
			trim(line);

			if (line.starts_with("flags=\""))
			{
				line.erase(0, 7);
				line.erase(line.length() - 1);

				for (const auto& flag : explode(";", line))
				{
					if (flag == "SPEW_MESSAGE")
						flags.push_back(SPEW_MESSAGE);
					else if (flag == "SPEW_WARNING")
						flags.push_back(SPEW_WARNING);
					else if (flag == "SPEW_ASSERT")
						flags.push_back(SPEW_ASSERT);
					else if (flag == "SPEW_ERROR")
						flags.push_back(SPEW_ERROR);
					else if (flag == "SPEW_LOG")
						flags.push_back(SPEW_LOG);
				}
			}
			else if (buffer[0] != '\0' && buffer[0] != '\n' && buffer[0] != '/')
				words.push_back(line);
		}

		fclose(fp);
	}
	else
	{
		perror("Failed to open");

		return false;
	}

	return true;
}

CON_COMMAND(hco_reload, "Reload block words")
{
	if (!g_HookConsoleOutput.LoadFile(FILE_WORDS))
	{
		printf("[HCO] Error load file\n");

		return;
	}

	printf("[HCO] Success reload!\n");
}

void HookConsoleOutput::Unload(void)
{
	ConVar_Unregister();

	DisconnectTier1Libraries();
	DisconnectTier2Libraries();
}

const char* HookConsoleOutput::GetPluginDescription()
{
	return "HookConsoleOutput by xstage [1.0.0]";
}