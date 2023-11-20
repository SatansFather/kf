#if !_COMPILER && !_SERVER
#include "kfglobal.h"
#include <functional>
#include "engine_console.h"
#include "../game_instance.h"
#include "../input/key_names.h"
#include "../input/input_processor.h"
#include "../net/data/address.h"


#if _DEV
extern KString CCOM_Tele(const KString& val);
extern KString CCOM_RMode(const KString& val);
extern KString CCOM_ServerQuit(const KString& val);
extern KString CCOM_GoForward(const KString& val);
#endif

extern KString CCOM_ServerMap(const KString& val);

extern KString CCOM_Monsters(const KString& val);

extern KString CCOM_Position(const KString& val);
extern KString CCOM_Direction(const KString& val);

extern KString CCOM_Ghost(const KString& val);
extern KString CCOM_Fly(const KString& val);
extern KString CCOM_Walk(const KString& val);
extern KString CCOM_Ammo(const KString& val);
extern KString CCOM_God(const KString& val);

extern KString CCOM_Exit(const KString& val);
extern KString CCOM_Time(const KString& val);

extern KString CCOM_ConsolePause(const KString& val);
extern KString CCOM_BobScale(const KString& val);
extern KString CCOM_ShakeScale(const KString& val);
extern KString CCOM_ConcussionScale(const KString& val);
extern KString CCOM_RollScale(const KString& val);
extern KString CCOM_WeaponFlash(const KString& val);

extern KString CCOM_Map(const KString& val);
extern KString CCOM_Maps(const KString& val);
extern KString CCOM_Replays(const KString& val);
extern KString CCOM_Replay(const KString& val);

extern KString CCOM_Fov(const KString& val);
extern KString CCOM_FovAdd(const KString& val);
extern KString CCOM_FovSub(const KString& val);
extern KString CCOM_HFov(const KString& val);
extern KString CCOM_Res(const KString& val);
extern KString CCOM_MaxFPS(const KString& val);
extern KString CCOM_RenderScale(const KString& val);
extern KString CCOM_RenderScaleFilter(const KString& val);
extern KString CCOM_RenderScaleMetric(const KString& val);
extern KString CCOM_Brightness(const KString& val);
extern KString CCOM_Contrast(const KString& val);
extern KString CCOM_Saturation(const KString& val);
extern KString CCOM_Flip(const KString& val);

extern KString CCOM_Interpolate(const KString& val);

extern KString CCOM_Screenshot(const KString& val);

extern KString CCOM_MouseSense(const KString& val);
extern KString CCOM_MouseSenseY(const KString& val);
extern KString CCOM_InvertX(const KString& val);
extern KString CCOM_InvertY(const KString& val);
extern KString CCOM_M_Measurement(const KString& val);
extern KString CCOM_M_Unit(const KString& val);
extern KString CCOM_M_UnitsPerSpin(const KString& val);
extern KString CCOM_M_DPI(const KString& val);
extern KString CCOM_M_Degrees(const KString& val);

extern KString CCOM_Connect(const KString& val);
extern KString CCOM_Host(const KString& val);
extern KString CCOM_Name(const KString& val);
extern KString CCOM_NetFramePad(const KString& val);
extern KString CCOM_NetSnapshotBuffer(const KString& val);
extern KString CCOM_AddServer(const KString& val);
extern KString CCOM_Servers(const KString& val);
extern KString CCOM_Spectate(const KString& val);
extern KString CCOM_Join(const KString& val);

extern KString CCOM_MasterVolume(const KString& val);

extern KString CCOM_Clear(const KString& val);

extern KString CCOM_ListWadContents(const KString& val);

extern KString CCOM_Say(const KString& val);
extern KString CCOM_TeamSay(const KString& val);

extern KString CCOM_SpecPoint(const KString& val);
extern KString CCOM_SpecPlay(const KString& val);

extern KString CCOM_DrawStats(const KString& val);


KString CCOM_Bind(const KString& val);

TVector<char> savestate;

KString CCOM_Save(const KString& val)
{
	//savestate = KPool::DataPoolBase::SerializeAllPools();
	return "";
}

KString CCOM_Load(const KString& val)
{
	//KPool::DataPoolBase::DeserializeAllPools(savestate);
	return "";
}

struct KConsoleCommand
{
	KString Invoke;
	KString Description;
	std::function<KString(const KString&)> Function;
};

enum class ECommandThread
{
	None,
	Game,
	Render,
	Input
};

struct ConsoleCommand
{
	std::function<KString(const KString&)> Function;
	ECommandThread Thread;
};

static TMap<KString, ConsoleCommand> Functions =
{
	{ "bind", { CCOM_Bind, ECommandThread::Input } },
	{ "listbind", { CCOM_Bind, ECommandThread::Input } },
	{ "addbind", { CCOM_Bind, ECommandThread::Input } },
	{ "resetbind", { CCOM_Bind, ECommandThread::Input } },

#if _DEV
	{ "teleport", { CCOM_Tele, ECommandThread::Game } },
	{ "r_mode", { CCOM_RMode, ECommandThread::Render } },
	{ "serverquit", { CCOM_ServerQuit, ECommandThread::Game } },
	{ "goforward", { CCOM_GoForward, ECommandThread::Game } },
#endif

	{ "monsters", { CCOM_Monsters, ECommandThread::Game } },

	{ "clear", { CCOM_Clear, ECommandThread::None} },

	{ "position",   { CCOM_Position, ECommandThread::Game } },
	{ "direction",  { CCOM_Direction, ECommandThread::Game } },
	{ "rotation",   { CCOM_Direction, ECommandThread::Game } },

	{ "map", { CCOM_Map, ECommandThread::Game } },
	{ "maps", { CCOM_Maps, ECommandThread::Game } },

	{ "replays", { CCOM_Replays, ECommandThread::Game } },
	{ "replay", { CCOM_Replay, ECommandThread::Game } },

	{ "exit", { CCOM_Exit, ECommandThread::Game } },
	{ "quit", { CCOM_Exit, ECommandThread::Game } },
	{ "listwad", { CCOM_ListWadContents, ECommandThread::Game } },


	{ "time", { CCOM_Time, ECommandThread::Game } },

	{ "consolepause", { CCOM_ConsolePause, ECommandThread::Game } },
	{ "bobscale", { CCOM_BobScale, ECommandThread::Game } },
	{ "rollscale", { CCOM_RollScale, ECommandThread::Game } },
	{ "shakescale", { CCOM_ShakeScale, ECommandThread::Game } },
	{ "concussionscale", { CCOM_ConcussionScale, ECommandThread::Game } },
	{ "weaponflash", { CCOM_WeaponFlash, ECommandThread::Game } },

	// control
	{ "sense", { CCOM_MouseSense, ECommandThread::None } },
	{ "sensitivity", { CCOM_MouseSense, ECommandThread::None } },
	{ "sensex", { CCOM_MouseSense, ECommandThread::None } },
	{ "sensitivityx", { CCOM_MouseSense, ECommandThread::None } },
	{ "sensey", { CCOM_MouseSenseY, ECommandThread::None } },
	{ "sensitivityy", { CCOM_MouseSenseY, ECommandThread::None } },
	{ "invertx", { CCOM_InvertX, ECommandThread::None } },
	{ "inverty", { CCOM_InvertY, ECommandThread::None } },
	{ "m_measurement", { CCOM_M_Measurement, ECommandThread::None } },
	{ "m_unit", { CCOM_M_Unit, ECommandThread::None } },
	{ "m_unitsperspin", { CCOM_M_UnitsPerSpin, ECommandThread::None } },
	{ "m_dpi", { CCOM_M_DPI, ECommandThread::None } },
	{ "m_degrees", { CCOM_M_Degrees, ECommandThread::None } },

	// render
	{ "fov", { CCOM_Fov, ECommandThread::Render } },
	{ "fov+", { CCOM_FovAdd, ECommandThread::Render } },
	{ "fov-", { CCOM_FovSub, ECommandThread::Render } },
	{ "hfov", { CCOM_HFov, ECommandThread::Render } },
	{ "res", { CCOM_Res, ECommandThread::Render } },
	{ "maxfps", { CCOM_MaxFPS, ECommandThread::Render } },
	{ "brightness", { CCOM_Brightness, ECommandThread::Render } },
	{ "contrast", { CCOM_Contrast, ECommandThread::Render } },
	{ "saturation", { CCOM_Saturation, ECommandThread::Render } },
	{ "r_scale", { CCOM_RenderScale, ECommandThread::Render } },
	{ "r_scalefilter", { CCOM_RenderScaleFilter, ECommandThread::Render } },
	{ "r_scalemetric", { CCOM_RenderScaleMetric, ECommandThread::Render } },
	{ "r_interpolate", { CCOM_Interpolate, ECommandThread::Render } },
	{ "screenshot", { CCOM_Screenshot, ECommandThread::Render } },
	{ "drawstats", { CCOM_DrawStats, ECommandThread::Render } },
	{ "r_flip", { CCOM_Flip, ECommandThread::Render } },

	// audio
	{ "volume", { CCOM_MasterVolume, ECommandThread::Game } },
	{ "masterv", { CCOM_MasterVolume, ECommandThread::Game } },

	// net
	{ "servermap", { CCOM_ServerMap, ECommandThread::Game } },
	{ "connect", { CCOM_Connect, ECommandThread::Game } },
	{ "host", { CCOM_Host, ECommandThread::Game } },
	{ "name", { CCOM_Name, ECommandThread::Game } },
	{ "n_framepad", { CCOM_NetFramePad, ECommandThread::Game } },
	{ "framepad", { CCOM_NetFramePad, ECommandThread::Game } },
	{ "n_snapshotbuffer", { CCOM_NetSnapshotBuffer, ECommandThread::Game } },
	{ "n_snapbuff", { CCOM_NetSnapshotBuffer, ECommandThread::Game } },
	{ "snapshotbuffer", { CCOM_NetSnapshotBuffer, ECommandThread::Game } },
	{ "snapbuff", { CCOM_NetSnapshotBuffer, ECommandThread::Game } },
	{ "addserver", { CCOM_AddServer, ECommandThread::Game } },
	{ "servers", { CCOM_Servers, ECommandThread::Game } },
	{ "spectate", { CCOM_Spectate, ECommandThread::Game } },
	{ "join", { CCOM_Join, ECommandThread::Game } },

	// cheats
	{ "ghost",   { CCOM_Ghost, ECommandThread::Game } },
	{ "fly",  { CCOM_Fly, ECommandThread::Game } },
	{ "walk",  { CCOM_Walk, ECommandThread::Game } },
	{ "god",  { CCOM_God, ECommandThread::Game } },
	{ "maxammo",  { CCOM_Ammo, ECommandThread::Game } },

	{ "say",  { CCOM_Say, ECommandThread::Game } },
	{ "teamsay",  { CCOM_TeamSay, ECommandThread::Game } },
	{ "sayteam",  { CCOM_TeamSay, ECommandThread::Game } },

	{ "specpoint",  { CCOM_SpecPoint, ECommandThread::Game } },
	{ "specplay",  { CCOM_SpecPlay, ECommandThread::Game } },

	{ "save",  { CCOM_Save, ECommandThread::Game } },
	{ "load",  { CCOM_Load, ECommandThread::Game } },

};

void KEngineConsole::ProcessSubmission(const KString& command)
{
	KString com = command.Trim();
	if (bPrintBack) AddMessage(">" + com.Trim(), .6, .6, .6);

	// find the first space to separate the command and value
	u32 space = com.Find(' ');
	KString cmd = com.GetSubstring(0, space);

	// dont let the value go lower case
	cmd.ToLowerInPlace();

	if (cmd != "connect")
		bLastWasServers = false;

	KString val;
	if (space != -1)
	{
		val = com.GetSubstring(space, MAX_U32);
		val.TrimInPlace();
	}

	// store entered commands so they can be cycled with up/down arrows
	if (!com.IsEmpty() && bPrintBack) // bPrintBack is false when not typed by user
	{
		// remove older matching entry
		VectorRemove(SubmittedCommands, com);
		SubmittedCommands.push_back(com);
	}

	// execute
	if (Functions.count(cmd))
	{
		if (Functions[cmd].Thread == ECommandThread::Game)
		{
			KGameInstance::Get().ConsoleCommandMutex.lock();
			KGameInstance::Get().PendingConsoleCommands.push_back(
			{
				Functions[cmd].Function,
				val
			});
			KGameInstance::Get().ConsoleCommandMutex.unlock();
		}
		else if (Functions[cmd].Thread == ECommandThread::Render)
		{
			std::lock_guard<std::mutex> lock(GetRenderInterface()->CommandMutex);
			KPendingConsoleCommand pending;
			pending.Function = Functions[cmd].Function;
			pending.Value = val;
			GetRenderInterface()->PendingConsoleCommands.push_back(pending);
		}
		else
		{		
			KString out = Functions[cmd].Function(val);
			if (!out.IsEmpty()) AddMessage(out);
		}
	}
}

TVector<TMap<KString, TVector<KString>>> AutocompleteMaps;

void KEngineConsole::InitAutocompleteMaps()
{
	for (const auto& kv : Functions)
	{
		const KString& funcStr = kv.first;
		KString buffer;

		if (funcStr.Size() > AutocompleteMaps.size()) 
			AutocompleteMaps.resize(funcStr.Size());

		for (u32 i = 0; i < funcStr.Size(); i++)
		{
			buffer += funcStr[i];
			AutocompleteMaps[i][buffer].push_back(funcStr);
		}
	}

	// TODO
	// server change/reset map
	//	players shouldnt be added to net state replicated objects map because they persist through map changes
	//	net destroys for player should flag first netID bit and the rest should be player index
	//	player net ID should be completely irrelevant
	//	client should destroy players before creating new snapshot items in case the player was replaced
}

void KEngineConsole::OnTextUpdated()
{
	// TextMutex is locked here

	AutoSuggestions = nullptr;
	u32 size = CurrentString.Size();
	if (size > 0 && size <= AutocompleteMaps.size())
	{
		size -= 1;
		if (AutocompleteMaps[size].contains(CurrentString))
			AutoSuggestions = &(AutocompleteMaps[size][CurrentString]);
	}
	LastAutoCompleteUpdateTime = KTime::Now();
}

KString CCOM_Bind(const KString& val)
{
	if (val.Size() < 3)
	{
		// size must be at least 3 to fit a key and a command
		return "No command was passed";
	}

	const auto splitFirstSpace = [](const KString& in, KString& left, KString& right) -> void
	{
		for (u32 i = 0; i < in.Size() - 1; i++)
		{
			if (in[i] == ' ')
			{
				left = in.GetSubstring(0, i).Trim();
				right = in.GetSubstring(i, in.Size()).Trim();
				break;
			}	
		}
	};

	KString key;
	KString command;
	splitFirstSpace(val, key, command);
	key.ToLowerInPlace();

	if (!KKeyNames::KeyNameIsValid(key))
		return "Invalid key " + KString("\'") + key + KString("\'");

	if (command.IsEmpty())
		return "No command was passed";

	// split command value to make sure the command is valid

	KString com;
	KString value;

	if (command.Contains(" "))
		splitFirstSpace(command, com, value);
	else
		com = command;
		
	if (!Functions.contains(com))
		return "Invalid command: \'" + com + "\'";
		
	// TODO check if command needs a value?
	
	if (!value.IsEmpty())
		value = KString(" ") + value;

	// write config
	LockConfig();
	KUserConfig* cfg = GetUserConfig();
	cfg->Keybinds.CustomBinds[key] = com + value;
	cfg->Save(cfg->GetActiveConfig());
	KInputProcessor::UpdateCustomBindsFromConfig(cfg);
	UnlockConfig();

	// success
	return "\'" + key + "\' was bound to \'" + com + value + "\'";
}

KString CCOM_AddServer(const KString& val)
{
	TVector<KString> split;
	val.SplitByChar(split, ' ', 1);
	if (split.size() == 2)
	{
		split[0].TrimInPlace();
		split[1].TrimInPlace();

		KNetAddress addr;
		if (!addr.SetFromString(split[0]))
			return "Improperly formed IP";
			
		if (split[1].IsEmpty())
			return "No server nickname given";

		if (split[1] == "local")
			return "\'local\' nickname reserved 127.0.0.1";

		LockConfig();
		KUserConfig* cfg = GetUserConfig();
		cfg->Net.FavoriteServers[split[1]] = split[0];
		cfg->Save(cfg->GetActiveConfig());
		UnlockConfig();

		return "Server IP " + split[0] + " saved as \'" + split[1] + "\'";
	}

	return "Improper format - use \'addserver ip name\'";
}

#endif