﻿#ifndef __TYPES_H__
#define __TYPES_H__

#include <chrono>
#include <memory>
#include <cstdint>
#include <functional>

namespace eddy
{
	typedef uint32_t								IOThreadID;
	typedef uint32_t								TCPSessionID;
	typedef std::chrono::steady_clock::time_point	TimePoint;

	class											NetMessage;
	class											TCPSession;
	class											IOServiceThread;
	class											TCPSessionHandle;
	class											MessageFilterInterface;

	typedef std::shared_ptr<NetMessage>				NetMessagePointer;
	typedef std::shared_ptr<TCPSession>				SessionPointer;
	typedef std::shared_ptr<IOServiceThread>		ThreadPointer;
	typedef std::shared_ptr<TCPSessionHandle>		SessionHandlerPointer;
	typedef std::shared_ptr<MessageFilterInterface>	MessageFilterPointer;

	typedef std::function<SessionHandlerPointer()>	SessionHandlerCreator;
	typedef std::function<MessageFilterPointer()>	MessageFilterCreator;
}

#endif