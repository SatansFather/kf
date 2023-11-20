#if !_SERVER

#include "text_console.h"
#include "../game_instance.h"
#include "../input/input_processor.h"
#include "../input/listener_console.h"
#include "engine_console.h"

KTextConsole::KTextConsole()
{
	PendingLayoutRemoval.reserve(100);
	Listener = std::make_unique<KConsoleInputListener>();
	Listener->Console = this;
}

void KTextConsole::OpenConsole()
{
	if (!bIsShowing)
	{
		bIsShowing = true;
		KInputProcessor::PushListenerStack(Listener.get());
		std::lock_guard<std::mutex> lock(TextMutex);
		LastCursorFlashTime = KTime::Now();
	}
}

void KTextConsole::CloseConsole()
{
	if (bIsShowing)
	{
		bIsShowing = false;
		KInputProcessor::RemoveListenerFromStack(Listener.get());
	}
}

void KTextConsole::AddMessage(const KString& text, f32 r /*= 1*/, f32 g /*= 1*/, f32 b /*= 1*/, bool playSound /*= true*/)
{
	std::lock_guard<std::mutex> lock(MessageMutex);

	UPtr<KTextMessage> msg = std::make_unique<KTextMessage>();
	msg->TimeCreated = KTime::Now();
	MessageCount++;
	msg->Message = text;
	msg->r = r; msg->g = g; msg->b = b;
	if (MessageHead)
	{
		MessageHead->Prev = msg.get();
		msg->Next = std::move(MessageHead);
	}
	MessageHead = std::move(msg);

	if (!OldestMessage) OldestMessage = MessageHead.get();

	if (MessageCount > MaxMessages)
		RemoveOldestMessage(false);

	if (playSound)
		OnMessageAdded();
}

bool KTextConsole::RemoveOldestMessage(bool lock)
{
	if (lock)
	{
		std::lock_guard<std::mutex> l(MessageMutex);
		return RemoveOldestMessage(false);
	}

	if (OldestMessage)
	{
		if (KTextMessage* msg = OldestMessage->Prev)
		{
			ResetMessageUPtr(msg->Next);
			OldestMessage = msg;
		}
		else
		{
			ResetMessageUPtr(MessageHead);
			OldestMessage = nullptr;
		}
		K_ASSERT(MessageCount > 0, "removed a console message while MessageCount was 0");
		MessageCount--;
		return true;
	}
	return false;
}

void KTextConsole::ClearConsole()
{
	std::lock_guard<std::mutex> l(MessageMutex);
	while (RemoveOldestMessage(false)) {}
}

void KTextConsole::SetShowing(bool showing)
{
	showing ? OpenConsole() : CloseConsole();
}

void KTextConsole::UpdateSubmissionString(const KString& text)
{
	std::lock_guard<std::mutex> lock(TextMutex);
	CurrentString = text;
	OnTextUpdated();
	bPendingTextUpdate = true;
}

void KTextConsole::UpdateCursor(i32 pos)
{
	std::lock_guard<std::mutex> lock(TextMutex);
	CursorPosition = pos;
	LastCursorFlashTime = KTime::Now();
	bPendingTextUpdate = true;
}

void KTextConsole::AddScroll(i32 scroll)
{
	SetScroll(ScrollCount + scroll);
}

void KTextConsole::SetScroll(i32 scroll)
{
	const i32 maxScroll = MessageCount > 0 ? MessageCount - 1 : 0;
	ScrollCount = std::clamp(scroll, 0, maxScroll);
}

void KTextConsole::ResetCommandScroll()
{
	SubmittedCommandScrollIndex = -1;
}

bool KTextConsole::IsInputFocus() const
{
	return KInputProcessor::GetActiveListener() == Listener.get();
}

void KTextConsole::ResetMessageUPtr(UPtr<KTextMessage>& msg)
{
	PendingLayoutRemoval.push_back(msg->LayoutHandle);
	msg.reset();
}

#endif