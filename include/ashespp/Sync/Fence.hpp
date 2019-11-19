/*
This file belongs to Ashes.
See LICENSE file in root folder.
*/
#ifndef ___AshesPP_Fence_HPP___
#define ___AshesPP_Fence_HPP___
#pragma once

#include "ashespp/AshesPPPrerequisites.hpp"

namespace ashes
{
	/**
	*\brief
	*	Possible returns while waiting for a fence.
	*/
	enum class WaitResult
	{
		eSuccess,
		eTimeOut,
		eError
	};
	/**
	*\brief
	*	Allows synchronisations of operations on a queue.
	*/
	class Fence
	{
	public:
		/**
		*\brief
		*	Constructor
		*\param[in] device
		*	The logical device.
		*\param[in] flags
		*	The creation flags.
		*/ 
		Fence( Device const & device
			, VkFenceCreateFlags flags );
		/**
		*\brief
		*	Destructor.
		*/
		~Fence();
		/**
		*\brief
		*	Waits for the fence to be signaled.
		*\param[in] timeout
		*	The time to wait for.
		*\return
		*	\p WaitResult::eSuccess or \p WaitResult::eTimeOut on success.
		*/ 
		WaitResult wait( uint64_t timeout )const;
		/**
		*\brief
		*	Unsignals the fence.
		*/ 
		void reset()const;
		/**
		*\brief
		*	VkFence implicit cast operator.
		*/
		inline operator VkFence const &()const
		{
			return m_internal;
		}

	private:
		Device const & m_device;
		VkFence m_internal{ VK_NULL_HANDLE };
	};
}

#endif