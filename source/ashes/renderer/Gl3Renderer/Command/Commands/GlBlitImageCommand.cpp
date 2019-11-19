/*
This file belongs to GlInstance.
See LICENSE file in root folder.
*/
#include "Command/Commands/GlBlitImageCommand.hpp"

#include "Command/Commands/GlCopyImageCommand.hpp"
#include "Core/GlDevice.hpp"
#include "Image/GlImage.hpp"
#include "Image/GlImageView.hpp"
#include "Miscellaneous/GlCallLogger.hpp"
#include "RenderPass/GlFrameBuffer.hpp"

#include "ashesgl3_api.hpp"

namespace ashes::gl3
{
	namespace
	{
		GlImageAspectFlags getMask( VkFormat format )
		{
			GlImageAspectFlags result = 0u;

			if ( isDepthFormat( format ) )
			{
				result |= GL_DEPTH_BUFFER_BIT;
			}

			if ( isStencilFormat( format ) )
			{
				result |= GL_STENCIL_BUFFER_BIT;
			}

			if ( !isDepthFormat( format )
				&& !isStencilFormat( format ) )
			{
				result |= GL_COLOR_BUFFER_BIT;
			}

			return result;
		}
	}

	FboAttachment makeFboAttachment( VkDevice device
		, VkImageSubresourceLayers & subresource
		, VkImage image
		, uint32_t layer )
	{
		FboAttachment result
		{
			getAttachmentPoint( get( image )->getFormat() ),
			get( image )->getInternal(),
			getAttachmentType( get( image )->getFormat() ),
			( get( image )->getSamples() > VK_SAMPLE_COUNT_1_BIT
				? GL_TEXTURE_2D_MULTISAMPLE
				: ( checkFlag( get( image )->getCreateFlags(), VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT )
					? GL_TEXTURE_CUBE_POSITIVE_X
					: GL_TEXTURE_2D ) ),
			subresource.mipLevel,
			0u,
		};

		if ( get( image )->getArrayLayers() > 1u )
		{
			result.mipLevel = 0u;
		}

		return result;
	}

	struct LayerCopy
	{
		LayerCopy( VkDevice device
			, VkImageBlit region
			, VkImage srcImage
			, VkImage dstImage
			, uint32_t layer )
			: region{ region }
			, src{ makeFboAttachment( device, region.srcSubresource, srcImage, layer ) }
			, dst{ makeFboAttachment( device, region.dstSubresource, dstImage, layer ) }
		{
		}

		VkImageBlit region;
		FboAttachment src;
		FboAttachment dst;
	};

	void apply( ContextLock const & context
		, CmdBlitFramebuffer const & cmd )
	{
		glLogCall( context
			, glBlitFramebuffer
			, cmd.srcL
			, cmd.srcT
			, cmd.srcR
			, cmd.srcB
			, cmd.dstL
			, cmd.dstT
			, cmd.dstR
			, cmd.dstB
			, cmd.mask
			, cmd.filter );
	}

	void buildBlitImageCommand( ContextStateStack & stack
		, VkDevice device
		, VkImage srcImage
		, VkImage dstImage
		, VkImageBlit region
		, VkFilter filter
		, CmdList & list )
	{
		assert( get( srcImage )->getArrayLayers() == get( dstImage )->getArrayLayers() );

		for ( uint32_t layer = 0u; layer < get( srcImage )->getArrayLayers(); ++layer )
		{
			LayerCopy layerCopy
			{
				device,
				region,
				srcImage,
				dstImage,
				layer
			};

			// Setup source FBO
			list.push_back( makeCmd< OpType::eInitFramebuffer >( &get( get( device )->getBlitSrcFbo() )->getInternal() ) );
			list.push_back( makeCmd< OpType::eBindFramebuffer >( GL_FRAMEBUFFER
				, get( device )->getBlitSrcFbo() ) );
			list.push_back( makeCmd< OpType::eFramebufferTexture2D >( GL_FRAMEBUFFER
				, layerCopy.src.point
				, layerCopy.src.target
				, layerCopy.src.object
				, layerCopy.region.srcSubresource.mipLevel ) );
			list.push_back( makeCmd< OpType::eBindFramebuffer >( GL_FRAMEBUFFER
				, nullptr ) );

			// Setup dst FBO
			list.push_back( makeCmd< OpType::eInitFramebuffer >( &get( get( device )->getBlitDstFbo() )->getInternal() ) );
			list.push_back( makeCmd< OpType::eBindFramebuffer >( GL_FRAMEBUFFER
				, get( device )->getBlitDstFbo() ) );
			list.push_back( makeCmd< OpType::eFramebufferTexture2D >( GL_FRAMEBUFFER
				, layerCopy.dst.point
				, layerCopy.dst.target
				, layerCopy.dst.object
				, layerCopy.region.dstSubresource.mipLevel ) );
			list.push_back( makeCmd< OpType::eBindFramebuffer >( GL_FRAMEBUFFER
				, nullptr ) );

			// Perform the blit
			list.push_back( makeCmd< OpType::eBindFramebuffer >( GL_READ_FRAMEBUFFER
				, get( device )->getBlitSrcFbo() ) );
			list.push_back( makeCmd< OpType::eReadBuffer >( uint32_t( layerCopy.src.point ) ) );
			list.push_back( makeCmd< OpType::eBindFramebuffer >( GL_DRAW_FRAMEBUFFER
				, get( device )->getBlitDstFbo() ) );
			list.push_back( makeCmd< OpType::eDrawBuffers >( uint32_t( layerCopy.dst.point ) ) );
			list.push_back( makeCmd< OpType::eBlitFramebuffer >( layerCopy.region.srcOffsets[0].x
				, layerCopy.region.srcOffsets[0].y
				, layerCopy.region.srcOffsets[1].x
				, layerCopy.region.srcOffsets[1].y
				, layerCopy.region.dstOffsets[0].x
				, layerCopy.region.dstOffsets[0].y
				, layerCopy.region.dstOffsets[1].x
				, layerCopy.region.dstOffsets[1].y
				, getMask( get( srcImage )->getFormat() )
				, convert( filter ) ) );

			// Unbind
			list.push_back( makeCmd< OpType::eBindFramebuffer >( GL_READ_FRAMEBUFFER
				, nullptr ) );
			list.push_back( makeCmd< OpType::eBindFramebuffer >( GL_DRAW_FRAMEBUFFER
				, nullptr ) );

			if ( stack.hasCurrentFramebuffer() )
			{
				stack.setCurrentFramebuffer( VK_NULL_HANDLE );
			}
		}

		if ( get( get( dstImage )->getMemory() )->getBuffer() != GL_INVALID_INDEX )
		{
			auto dstTarget = convert( get( dstImage )->getType()
				, get( dstImage )->getArrayLayers()
				, get( dstImage )->getCreateFlags() );
			list.push_back( makeCmd< OpType::eBindBuffer >( GL_BUFFER_TARGET_PIXEL_PACK, get( get( dstImage )->getMemory() )->getBuffer() ) );
			list.push_back( makeCmd< OpType::eBindTexture >( dstTarget, get( dstImage )->getInternal() ) );
			auto internal = getInternalFormat( get( dstImage )->getFormat() );
			list.push_back( makeCmd< OpType::eGetTexImage >( dstTarget, getFormat( internal ), getType( internal ) ) );
			list.push_back( makeCmd< OpType::eBindTexture >( dstTarget, 0u ) );
			list.push_back( makeCmd< OpType::eBindBuffer >( GL_BUFFER_TARGET_PIXEL_PACK, 0u ) );
			list.push_back( makeCmd< OpType::eDownloadMemory >( get( dstImage )->getMemory() ) );
		}
	}
}