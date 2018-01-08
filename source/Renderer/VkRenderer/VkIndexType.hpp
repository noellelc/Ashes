/*
This file belongs to Renderer.
See LICENSE file in root folder.
*/
#pragma once

#include <Renderer/IndexType.hpp>

namespace vk_renderer
{
	/**
	*\brief
	*	Convertit un renderer::IndexType en VkIndexType.
	*\param[in] type
	*	Le renderer::IndexType.
	*\return
	*	Le VkIndexType.
	*/
	VkIndexType convert( renderer::IndexType const & type );
}
