/*
This file belongs to Ashes.
See LICENSE file in root folder.
*/
#include "Command/Commands/GlClearColourCommand.hpp"

#include "Core/GlContextStateStack.hpp"
#include "Image/GlImage.hpp"
#include "Miscellaneous/GlCallLogger.hpp"

#include "ashesgl_api.hpp"

namespace ashes::gl
{
	template< typename ValueT >
	bool operator==( ValueT const( & lhs )[4], std::array< ValueT, 4u > rhs )
	{
		return lhs[0] == rhs[0]
			&& lhs[1] == rhs[1]
			&& lhs[2] == rhs[2]
			&& lhs[3] == rhs[3];
	}

	template< typename ValueT >
	bool operator!=( ValueT const( &lhs )[4], std::array< ValueT, 4u > rhs )
	{
		return !( lhs == rhs );
	}

	void buildClearColourCommand( VkDevice device
		, ContextStateStack & stack
		, VkImage image
		, VkImageLayout imageLayout
		, VkClearColorValue value
		, ArrayView< VkImageSubresourceRange const > ranges
		, CmdList & list )
	{
		glLogCommand( list, "ClearColourCommand" );
		stack.applyDisableBlend( list );

		if ( get( getInstance( device ) )->hasClearTexImage() )
		{
			for ( auto range : ranges )
			{
				if ( range.levelCount == RemainingArrayLayers )
				{
					range.levelCount = ashes::getMaxMipCount( get( image )->getDimensions() );
				}

				if ( range.layerCount == RemainingArrayLayers )
				{
					range.layerCount = get( device )->getLimits().maxImageArrayLayers;
				}

				for ( uint32_t level = range.baseMipLevel;
					level < range.baseMipLevel + range.levelCount;
					++level )
				{
					if ( isUIntFormat( get( image )->getFormatVk() ) )
					{
						list.push_back( makeCmd< OpType::eClearTexColorUI >( get( image )->getInternal()
							, level
							, get( image )->getUnpackFormat()
							, value.uint32 ) );
					}
					else if ( isSIntFormat( get( image )->getFormatVk() ) )
					{
						list.push_back( makeCmd< OpType::eClearTexColorSI >( get( image )->getInternal()
							, level
							, get( image )->getUnpackFormat()
							, value.int32 ) );
					}
					else
					{
						list.push_back( makeCmd< OpType::eClearTexColorF >( get( image )->getInternal()
							, level
							, get( image )->getUnpackFormat()
							, value.float32 ) );
					}
				}
			}
		}
		else
		{
			std::cerr << "Unsupported command : ClearColourCommand" << std::endl;
		}
	}
}
