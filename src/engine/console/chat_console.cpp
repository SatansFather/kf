#if !_SERVER

#include "engine/game_instance.h"
#include "engine/input/listener_console.h"
#include "engine/input/input_processor.h"
#include "engine/game_instance.h"
#include "chat_console.h"
#include "../audio/sound_asset.h"
#include "../audio/audio.h"

KChatConsole* GetChatConsole()
{
	static KChatConsole console;
	return &console;
}

#if !_COMPILER

KString CCOM_Say(const KString& val)
{
	GetChatConsole()->ProcessSubmission(val);
	return "";
}

KString CCOM_TeamSay(const KString& val)
{
	// TODO team lol
	GetChatConsole()->ProcessSubmission(val);
	return "";
}

KChatConsole::KChatConsole()
{
	Listener->SetIsChat(true);
}

void KChatConsole::OnMessageAdded()
{
	KAudio::PlaySound(KSoundID::Chat_Beep);
}

void KChatConsole::ProcessSubmission(const KString& str)
{
	if (!str.IsEmpty())
	{
		KGameInstance::Get().SendChatMessage(str);	
		SubmittedCommands.push_back(str);
	}

	CloseConsole();
}

#if !_PACK
void CHATLOG(const KString& message, GFlt r /*= 1*/, GFlt g /*= 1*/, GFlt b /*= 1*/)
{
#if !_SERVER && !_COMPILER
	GetChatConsole()->AddMessage(message, r, g, b, false);
#else
	_SYSLOG(message);
#endif
}
#endif

#endif

#endif