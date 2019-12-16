/*
This file belongs to Ashes.
See LICENSE file in root folder.
*/
#include "ashespp/Image/Image.hpp"

#include "ashespp/Buffer/StagingBuffer.hpp"
#include "ashespp/Command/CommandBuffer.hpp"
#include "ashespp/Core/Device.hpp"
#include "ashespp/Image/ImageView.hpp"
#include "ashespp/Sync/Fence.hpp"

#include <ashes/common/Format.hpp>

namespace ashes
{
	namespace
	{
		VkImageMemoryBarrier makeTransition( VkImage image
			, VkImageLayout srcLayout
			, VkImageLayout dstLayout
			, VkImageSubresourceRange mipSubRange
			, uint32_t mipLevel )
		{
			mipSubRange.baseMipLevel = mipLevel;
			return VkImageMemoryBarrier
			{
				VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				nullptr,
				getAccessMask( srcLayout ),
				getAccessMask( dstLayout ),
				srcLayout,
				dstLayout,
				VK_QUEUE_FAMILY_IGNORED,
				VK_QUEUE_FAMILY_IGNORED,
				image,
				std::move( mipSubRange ),
			};
		}

		VkImageMemoryBarrier makeTransition( VkImage image
			, VkImageLayout prv
			, VkImageLayout cur
			, VkImageSubresourceRange mipSubRange )
		{
			auto mipLevel = mipSubRange.baseMipLevel;
			return makeTransition( image
				, prv
				, cur
				, std::move( mipSubRange )
				, mipLevel );
		}
	}

	VkAccessFlags getAccessMask( VkImageLayout layout )
	{
		VkAccessFlags result{ 0u };

		switch ( layout )
		{
		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
		case VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR:
			result |= VK_ACCESS_MEMORY_READ_BIT;
			break;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			result |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			result |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
			result |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
			break;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			result |= VK_ACCESS_SHADER_READ_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			result |= VK_ACCESS_TRANSFER_READ_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			result |= VK_ACCESS_TRANSFER_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
		case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
			result |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
			result |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;
#ifdef VK_NV_shading_rate_image
		case VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV:
			result |= VK_ACCESS_SHADING_RATE_IMAGE_READ_BIT_NV;
			break;
#endif
#ifdef VK_EXT_fragment_density_map
		case VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT:
			result |= VK_ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT_EXT;
			break;
#endif
		default:
			break;
		}

		return result;
	}

	VkPipelineStageFlags getStageMask( VkImageLayout layout )
	{
		VkPipelineStageFlags result{ 0u };

		switch ( layout )
		{
		case VK_IMAGE_LAYOUT_UNDEFINED:
			result |= VK_PIPELINE_STAGE_HOST_BIT;
			break;
		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
		case VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR:
			result |= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			break;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
		case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
		case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
			result |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			break;
#ifdef VK_EXT_fragment_density_map
		case VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT:
#endif
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			result |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			result |= VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
#ifdef VK_NV_shading_rate_image
		case VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV:
			result |= VK_PIPELINE_STAGE_SHADING_RATE_IMAGE_BIT_NV;
			break;
#endif
		default:
			break;
		}

		return result;
	}

	Image::Image()
	{
	}

	Image::Image( Image && rhs )
		: m_device{ rhs.m_device }
		, m_createInfo{ std::move( rhs.m_createInfo ) }
		, m_internal{ rhs.m_internal }
		, m_storage{ std::move( rhs.m_storage ) }
		, m_ownInternal{ rhs.m_ownInternal }
		, m_views{ std::move( rhs.m_views ) }
	{
		rhs.m_internal = VK_NULL_HANDLE;
		rhs.m_ownInternal = true;

		if ( m_ownInternal )
		{
			registerObject( *m_device, "Image", this );
		}
	}

	Image & Image::operator=( Image && rhs )
	{
		if ( &rhs != this )
		{
			m_createInfo = std::move( rhs.m_createInfo );
			m_internal = rhs.m_internal;
			m_storage = std::move( rhs.m_storage );
			m_ownInternal = rhs.m_ownInternal;
			m_views = std::move( rhs.m_views );
			rhs.m_internal = VK_NULL_HANDLE;
			rhs.m_ownInternal = true;

			if ( m_ownInternal )
			{
				registerObject( *m_device, "Image", this );
			}
		}

		return *this;
	}

	Image::Image( Device const & device
		, ImageCreateInfo createInfo )
		: m_device{ &device }
		, m_createInfo{ std::move( createInfo ) }
	{
		DEBUG_DUMP( m_createInfo );
		auto res = m_device->vkCreateImage( *m_device
			, &static_cast< VkImageCreateInfo const & >( m_createInfo )
			, nullptr
			, &m_internal );
		checkError( res, "Image creation" );
		registerObject( *m_device, "Image", this );
	}

	Image::Image( Device const & device
		, VkImage image )
		: m_device{ &device }
		, m_internal{ image }
		, m_createInfo
		{
			0u,
			VK_IMAGE_TYPE_2D,
			VK_FORMAT_UNDEFINED,	// TODO ?
			VkExtent3D{},			// TODO ?
			1u,
			1u,
			VK_SAMPLE_COUNT_1_BIT,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT
		}
		, m_ownInternal{ false }
	{
	}

	Image::~Image()
	{
		assert( ( ( m_internal != VK_NULL_HANDLE ) || m_views.empty() )
			&& "No more internal handle, but some image views remain." );

		if ( m_internal != VK_NULL_HANDLE )
		{
			for ( auto & view : m_views )
			{
				destroyView( view.first );
			}

			if ( m_ownInternal )
			{
				unregisterObject( *m_device, this );
				m_device->vkDestroyImage( *m_device
					, m_internal
					, nullptr );
			}
		}
	}

	void Image::bindMemory( DeviceMemoryPtr memory )
	{
		assert( !m_storage && "A resource can only be bound once to a device memory object." );
		m_storage = std::move( memory );
		auto res = m_device->vkBindImageMemory( *m_device
			, m_internal
			, *m_storage
			, 0 );
		checkError( res, "Image storage binding" );
	}

	Image::Mapped Image::lock( uint32_t offset
		, uint32_t size
		, VkMemoryMapFlags flags )const
	{
		assert( m_storage && "The resource is not bound to a device memory object." );
		Mapped mapped{};
		VkImageSubresource subResource{};
		subResource.aspectMask = getAspectMask( getFormat() );
		VkSubresourceLayout subResourceLayout;
		m_device->getImageSubresourceLayout( *this, subResource, subResourceLayout );

		mapped.data = m_storage->lock( offset
			, size
			, flags );

		if ( mapped.data )
		{
			mapped.arrayPitch = subResourceLayout.arrayPitch;
			mapped.depthPitch = subResourceLayout.depthPitch;
			mapped.rowPitch = subResourceLayout.rowPitch;
			mapped.size = subResourceLayout.size;
			mapped.data += subResourceLayout.offset;
		}

		return mapped;
	}

	void Image::invalidate( uint32_t offset
		, uint32_t size )const
	{
		assert( m_storage && "The resource is not bound to a device memory object." );
		return m_storage->invalidate( offset, size );
	}

	void Image::flush( uint32_t offset
		, uint32_t size )const
	{
		assert( m_storage && "The resource is not bound to a device memory object." );
		return m_storage->flush( offset, size );
	}

	void Image::unlock()const
	{
		assert( m_storage && "The resource is not bound to a device memory object." );
		return m_storage->unlock();
	}

	void Image::generateMipmaps( CommandPool const & commandPool
		, Queue const & queue
		, VkImageLayout dstImageLayout )const
	{
		if ( getMipmapLevels() <= 1u )
		{
			return;
		}

		auto commandBuffer = commandPool.createCommandBuffer();
		commandBuffer->begin( VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT );
		generateMipmaps( *commandBuffer, dstImageLayout );
		commandBuffer->end();
		auto fence = m_device->createFence();
		queue.submit( *commandBuffer, fence.get() );
		fence->wait( MaxTimeout );
	}

	void Image::generateMipmaps( CommandBuffer & commandBuffer
		, VkImageLayout dstImageLayout )const
	{
		if ( getMipmapLevels() <= 1u )
		{
			return;
		}

		auto const width = int32_t( getDimensions().width );
		auto const height = int32_t( getDimensions().height );
		auto const depth = int32_t( getDimensions().depth );
		auto const aspectMask = getAspectMask( getFormat() );
		auto const dstAccessMask = getAccessMask( dstImageLayout );
		auto const dstStageMask = getStageMask( dstImageLayout );
		auto const imageViewType = VkImageViewType( getType() );

		for ( uint32_t layer = 0u; layer < getLayerCount(); ++layer )
		{
			VkImageSubresourceRange mipSubRange
			{
				aspectMask,
				0u,
				1u,
				layer,
				1u
			};
			VkImageBlit imageBlit{};
			imageBlit.dstSubresource.aspectMask = aspectMask;
			imageBlit.dstSubresource.baseArrayLayer = mipSubRange.baseArrayLayer;
			imageBlit.dstSubresource.layerCount = 1;
			imageBlit.dstSubresource.mipLevel = mipSubRange.baseMipLevel;
			imageBlit.dstOffsets[0].x = 0;
			imageBlit.dstOffsets[0].y = 0;
			imageBlit.dstOffsets[0].z = 0;
			imageBlit.dstOffsets[1].x = getSubresourceDimension( width, mipSubRange.baseMipLevel );
			imageBlit.dstOffsets[1].y = getSubresourceDimension( height, mipSubRange.baseMipLevel );
			imageBlit.dstOffsets[1].z = depth;

			// Transition first mip level to transfer source layout
			commandBuffer.memoryBarrier( VK_PIPELINE_STAGE_TRANSFER_BIT
				, VK_PIPELINE_STAGE_TRANSFER_BIT
				, makeTransition( VK_IMAGE_LAYOUT_UNDEFINED
					, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
					, mipSubRange ) );

			// Copy down mips
			while ( ++mipSubRange.baseMipLevel < getMipmapLevels() )
			{
				// Blit from previous level
				// Blit source is previous blit destination
				imageBlit.srcSubresource = imageBlit.dstSubresource;
				imageBlit.srcOffsets[0] = imageBlit.dstOffsets[0];
				imageBlit.srcOffsets[1] = imageBlit.dstOffsets[1];

				// Update blit destination
				imageBlit.dstSubresource.mipLevel = mipSubRange.baseMipLevel;
				imageBlit.dstOffsets[1].x = getSubresourceDimension( width, mipSubRange.baseMipLevel );
				imageBlit.dstOffsets[1].y = getSubresourceDimension( height, mipSubRange.baseMipLevel );

				// Transition current mip level to transfer dest
				commandBuffer.memoryBarrier( VK_PIPELINE_STAGE_TRANSFER_BIT
					, VK_PIPELINE_STAGE_TRANSFER_BIT
					, makeTransition( VK_IMAGE_LAYOUT_UNDEFINED
						, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
						, mipSubRange ) );

				// Perform blit
				commandBuffer.blitImage( *this
					, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
					, *this
					, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
					, { imageBlit }
					, VkFilter::VK_FILTER_LINEAR );

				// Transition current mip level to transfer source for read in next iteration
				commandBuffer.memoryBarrier( VK_PIPELINE_STAGE_TRANSFER_BIT
					, VK_PIPELINE_STAGE_TRANSFER_BIT
					, makeTransition( VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
						, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
						, mipSubRange ) );

				// Transition previous mip level to wanted destination layout
				commandBuffer.memoryBarrier( VK_PIPELINE_STAGE_TRANSFER_BIT
					, dstStageMask
					, makeTransition( VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
						, dstImageLayout
						, mipSubRange
						, mipSubRange.baseMipLevel - 1u ) );
			}

			// Transition last mip level to wanted destination layout
			commandBuffer.memoryBarrier( VK_PIPELINE_STAGE_TRANSFER_BIT
				, dstStageMask
				, makeTransition( VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
					, dstImageLayout
					, mipSubRange
					, mipSubRange.baseMipLevel - 1u ) );
		}
	}

	VkMemoryRequirements Image::getMemoryRequirements()const
	{
		return m_device->getImageMemoryRequirements( m_internal );
	}

	ImageView Image::createView( VkImageViewCreateInfo createInfo )const
	{
		DEBUG_DUMP( createInfo );
		auto pCreateInfo = std::make_unique< VkImageViewCreateInfo >( std::move( createInfo ) );
		VkImageView vk;
		auto res = m_device->vkCreateImageView( *m_device
			, pCreateInfo.get()
			, nullptr
			, &vk );
		checkError( res, "ImageView creation" );
		auto create = *pCreateInfo;
		m_views.emplace( vk, std::move( pCreateInfo ) );
		registerObject( *m_device, "ImageView", vk );
		return ImageView
		{
			create,
			vk,
			this,
		};
	}

	ImageView Image::createView( VkImageViewType type
		, VkFormat format
		, uint32_t baseMipLevel
		, uint32_t levelCount
		, uint32_t baseArrayLayer
		, uint32_t layerCount
		, VkComponentMapping const & mapping )const
	{
		return createView(
		{
			VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			nullptr,
			0u,
			*this,
			type,
			format,
			mapping,
			{
				getAspectMask( format ),
				baseMipLevel,
				levelCount,
				baseArrayLayer,
				layerCount
			}
		} );
	}

	VkImageMemoryBarrier Image::makeTransition( VkImageLayout srcLayout
		, VkImageLayout dstLayout
		, VkImageSubresourceRange mipSubRange
		, uint32_t mipLevel )const
	{
		return ashes::makeTransition( *this
			, srcLayout
			, dstLayout
			, mipSubRange
			, mipLevel );
	}

	VkImageMemoryBarrier Image::makeTransition( VkImageLayout srcLayout
		, VkImageLayout dstLayout
		, VkImageSubresourceRange mipSubRange )const
	{
		return ashes::makeTransition( *this
			, srcLayout
			, dstLayout
			, mipSubRange );
	}

	void Image::destroyView( VkImageView view )const
	{
		unregisterObject( *m_device, view );
		m_device->vkDestroyImageView( *m_device
			, view
			, nullptr );
	}

	void Image::destroyView( ImageView view )const
	{
		auto it = m_views.find( view );
		assert( it != m_views.end() );
		destroyView( it->first );
		m_views.erase( it );
	}
}
