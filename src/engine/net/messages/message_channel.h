#pragma once

enum class EPacketChannel
{
	ServerWelcome,
	Password,
	GameState,
	UserChatMessage,
	SystemMessage,
	ConsoleMessage,
	MiscChannel,
	
	NumChannels
};

enum class EMessageType
{
	// server to client
	S_GameState,
S_RELIABLE,
	S_Ping,
	S_Welcome,
	S_MapSwitch,
	S_NeedPassword,
	S_UserChat,
	S_Message,
	S_MessageWarning,
	S_MessageError,
	S_ConsoleMessage,
	S_ConsoleWarning,
	S_ConsoleError,

BEGIN_CLIENT,

	// client to server
	C_Input,
C_RELIABLE,
	C_Pong,
	C_ConnectionAck,
	C_LoadedMap,
	C_ChatMessage,
	C_NameUpdate,
	C_ServerMap,

//#if _DEV
	C_ServerQuit
//#endif
};

static bool TypeIsReliable(EMessageType type)
{
	return (type > EMessageType::S_RELIABLE && type < EMessageType::BEGIN_CLIENT)
		|| type > EMessageType::C_RELIABLE;
}

static bool TypePacksFrame(EMessageType type)
{
	return type == EMessageType::S_GameState/* || type == EMessageType::C_Input*/;
}