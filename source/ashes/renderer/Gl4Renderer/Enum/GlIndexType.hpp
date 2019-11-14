/*
This file belongs to Ashes.
See LICENSE file in root folder.
*/
#pragma once

namespace ashes::gl4
{
	enum GlIndexType
		: GLenum
	{
		GL_INDEX_TYPE_UINT16 = 0x1403,
		GL_INDEX_TYPE_UINT32 = 0x1405,
	};
	std::string getName( GlIndexType value );
	/**
	*\brief
	*	Convertit un VkIndexType en GlIndexType.
	*\param[in] type
	*	Le VkIndexType.
	*\return
	*	Le GlIndexType.
	*/
	GlIndexType convert( VkIndexType const & type );
}
