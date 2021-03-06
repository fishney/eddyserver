﻿#ifndef __ID_GENERATOR_H__
#define __ID_GENERATOR_H__

#include <vector>
#include <cstdint>

namespace eddy
{
	class IDGenerator final
	{
	public:
		static const uint32_t kInvalidID = 0;
		static const uint32_t kDefaultThreshold = 8192;

	public:
		explicit IDGenerator(uint32_t threshold = kDefaultThreshold)
			: next_(kInvalidID), threshold_(threshold)
		{
		}

		~IDGenerator() = default;

		bool Get(uint32_t &id)
		{
			if (pools_.size() > threshold_)
			{
				id = pools_.back();
				pools_.pop_back();
				return true;
			}
			else if (next_ <= threshold_)
			{
				id = ++next_;
				return true;
			}
			return false;
		}

		void Put(uint32_t id)
		{
			pools_.push_back(id);
		}

	private:
		uint32_t				next_;
		const size_t			threshold_;
		std::vector<uint32_t>	pools_;
	};
}

#endif