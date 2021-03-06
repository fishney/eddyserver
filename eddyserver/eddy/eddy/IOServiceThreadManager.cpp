﻿#include "IOServiceThreadManager.h"
#include <limits>
#include <cassert>
#include <iostream>
#include "IOServiceThread.h"
#include "TCPSessionHandle.h"

namespace eddy
{
	const size_t kMainThreadIndex = 0;

	IOServiceThreadManager::IOServiceThreadManager(size_t thread_num)
	{
		assert(thread_num > kMainThreadIndex);
		if (thread_num == kMainThreadIndex)
		{
			std::abort();
		}

		threads_.resize(thread_num);
		thread_load_.resize(threads_.size());
		for (size_t i = 0; i < threads_.size(); ++i)
		{
			threads_[i] = std::make_shared<IOServiceThread>(i, *this);
		}
	}

	IOServiceThreadManager::~IOServiceThreadManager()
	{
		Stop();
	}

	void IOServiceThreadManager::Run()
	{
		if (threads_.empty())
		{
			return;
		}

		for (size_t i = 0; i < threads_.size(); ++i)
		{
			if (i != kMainThreadIndex)
			{
				threads_[i]->RunThread();
			}
		}
		threads_[kMainThreadIndex]->Run();
	}

	void IOServiceThreadManager::Stop()
	{
		if (threads_.empty())
		{
			return;
		}

		for (size_t i = 0; i < threads_.size(); ++i)
		{
			if (i != kMainThreadIndex)
			{
				threads_[i]->Stop();
			}
		}

		for (size_t i = 0; i < threads_.size(); ++i)
		{
			if (i != kMainThreadIndex)
			{
				threads_[i]->Join();
			}
		}

		threads_[kMainThreadIndex]->Stop();
	}

	ThreadPointer& IOServiceThreadManager::Thread()
	{
		if (threads_.size() == 1)
		{
			return threads_[kMainThreadIndex];
		}

		size_t min_element_index = kMainThreadIndex;
		size_t min_element_value = thread_load_[kMainThreadIndex];
		for (size_t i = 0; i < thread_load_.size(); ++i)
		{
			if (kMainThreadIndex != i && thread_load_[i] < min_element_value)
			{
				min_element_index = i;
			}
		}
		return threads_[min_element_index];
	}

	ThreadPointer IOServiceThreadManager::Thread(IOThreadID id)
	{
		for (size_t i = 0; i < threads_.size(); ++i)
		{
			if (threads_[i]->ID() == id)
			{
				return threads_[i];
			}
		}
		return nullptr;
	}

	ThreadPointer& IOServiceThreadManager::MainThread()
	{
		return threads_[kMainThreadIndex];
	}

	void IOServiceThreadManager::OnSessionConnect(SessionPointer &session_ptr, SessionHandlerPointer &handler_ptr)
	{
		TCPSessionID id = IDGenerator::kInvalidID;
		if (id_generator_.Get(id))
		{
			handler_ptr->Init(id, session_ptr->Thread()->ID(), this);
			session_handler_map_.insert(std::make_pair(id, handler_ptr));
			session_ptr->Thread()->Post(std::bind(&TCPSession::Init, session_ptr, id));
			handler_ptr->OnConnect();

			IOThreadID td_id = handler_ptr->ThreadID();
			if (td_id < thread_load_.size())
			{
				++thread_load_[td_id];
			}
		}
		else
		{
			std::cerr << "generator session id fail!" << std::endl;
		}
	}

	void IOServiceThreadManager::OnSessionClose(TCPSessionID id)
	{
		assert(IDGenerator::kInvalidID != id);
		SessionHandlerMap::iterator itr = session_handler_map_.find(id);
		if (itr != session_handler_map_.end())
		{
			SessionHandlerPointer handler_ptr = itr->second;
			IOThreadID td_id = handler_ptr->ThreadID();
			if (handler_ptr != nullptr)
			{
				handler_ptr->OnClose();
				handler_ptr->Dispose();
			}
			session_handler_map_.erase(itr);

			if (td_id < thread_load_.size() && thread_load_[td_id] > 0)
			{
				--thread_load_[td_id];
			}
		}

		if (IDGenerator::kInvalidID != id)
		{
			id_generator_.Put(id);
		}
	}

	SessionHandlerPointer IOServiceThreadManager::SessionHandler(TCPSessionID id) const
	{
		SessionHandlerMap::const_iterator itr = session_handler_map_.find(id);
		if (itr != session_handler_map_.end())
		{
			return itr->second;
		}
		return nullptr;
	}

	void IOServiceThreadManager::SetSessionTimeout(uint64_t seconds)
	{
		for (auto &item : threads_)
		{
			item->Post(std::bind(&IOServiceThread::SetSessionTimeout, item, seconds));
		}
	}
}