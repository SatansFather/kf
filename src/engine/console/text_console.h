#pragma once

#if !_SERVER

#include "kfglobal.h"
#include <mutex>

struct KTextMessage
{
	KString Message;
	u32 LayoutHandle = 0;
	f32 r, g, b;
	KTextMessage* Prev = nullptr;
	KTimePoint TimeCreated;
	UPtr<KTextMessage> Next;
};

class KTextConsole
{
	friend class KHudTextConsole;
	friend class KConsoleInputListener;
	friend class KGameInstance;

protected:
	// starting at most recent
	UPtr<KTextMessage> MessageHead;
	UPtr<class KConsoleInputListener> Listener;

	KTextMessage* OldestMessage = nullptr;
	u32 MessageCount = 0;
	u32 MaxMessages = 500;
	std::atomic<bool> bIsShowing = { false };
	TVector<u32> PendingLayoutRemoval;
	i32 ScrollCount = 0; // scroll up message count
	std::mutex MessageMutex;

	std::mutex TextMutex;
	KString CurrentString;
	u32 SubmissionLayoutHandle = 0;
	u32 CursorPosHandle = 0;
	i32 CursorPosition = 0;
	KTimePoint LastCursorFlashTime;
	bool bPendingTextUpdate = { false };

	TVector<KString> SubmittedCommands;
	i32 SubmittedCommandScrollIndex = -1;

public:

	KTextConsole();

	void OpenConsole();
	void CloseConsole();

	void AddMessage(const KString& text, f32 r = 1, f32 g = 1, f32 b = 1, bool playSound = true);
	bool RemoveOldestMessage(bool lock = true);
	void ClearConsole();
	bool IsShowing() const { return bIsShowing; }
	void SetShowing(bool showing);

	void UpdateSubmissionString(const KString& text);
	void UpdateCursor(i32 pos);

	void AddScroll(i32 scroll);
	void SetScroll(i32 scroll);

	void ResetCommandScroll();

	bool IsInputFocus() const;

	KConsoleInputListener* GetListener() { return Listener.get(); }

	virtual void OnMessageAdded() {}

#if !_COMPILER
	virtual void ProcessSubmission(const KString& str) = 0;
#endif

protected:

	virtual void OnTextUpdated() {}

private:

	void ResetMessageUPtr(UPtr<KTextMessage>& msg);
};

#endif