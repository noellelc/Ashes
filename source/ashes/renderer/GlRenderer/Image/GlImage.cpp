#include "Image/GlImage.hpp"

#include "Command/GlCommandBuffer.hpp"
#include "Core/GlDevice.hpp"
#include "Image/GlImageView.hpp"
#include "Miscellaneous/GlCallLogger.hpp"
#include "Miscellaneous/GlDeviceMemory.hpp"
#include "Miscellaneous/GlDeviceMemoryBinding.hpp"

#include "ashesgl_api.hpp"

#ifdef max
#	undef max
#	undef min
#endif

namespace ashes::gl
{
	namespace gl3
	{
		static GlTextureType convert( VkImageType type
			, uint32_t layerCount
			, VkImageCreateFlags flags
			, VkSampleCountFlagBits samples )
		{
			GlTextureType result;

			switch ( type )
			{
			case VK_IMAGE_TYPE_1D:
				if ( layerCount > 1 )
				{
					result = GL_TEXTURE_1D_ARRAY;
				}
				else
				{
					result = GL_TEXTURE_1D;
				}
				break;
			case VK_IMAGE_TYPE_2D:
				if ( layerCount > 1 )
				{
					if ( samples > VK_SAMPLE_COUNT_1_BIT )
					{
						result = GL_TEXTURE_2D_MULTISAMPLE_ARRAY;
					}
					else if ( checkFlag( flags, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT ) )
					{
						assert( ( layerCount % 6 ) == 0 );

						if ( layerCount > 6 )
						{
							result = GL_TEXTURE_CUBE_ARRAY;
						}
						else
						{
							result = GL_TEXTURE_CUBE;
						}
					}
					else
					{
						result = GL_TEXTURE_2D_ARRAY;
					}
				}
				else
				{
					if ( samples != VK_SAMPLE_COUNT_1_BIT )
					{
						result = GL_TEXTURE_2D_MULTISAMPLE;
					}
					else
					{
						result = GL_TEXTURE_2D;
					}
				}
				break;
			case VK_IMAGE_TYPE_3D:
				result = GL_TEXTURE_3D;
				break;
			default:
				assert( false );
				result = GL_TEXTURE_2D;
				break;
			}

			return result;
		}
	}

	namespace gl4
	{
		static GlTextureType convert( VkImageType type
			, uint32_t layerCount
			, VkImageCreateFlags flags
			, VkSampleCountFlagBits samples )
		{
			GlTextureType result;

			switch ( type )
			{
			case VK_IMAGE_TYPE_1D:
				if ( layerCount > 1 )
				{
					result = GL_TEXTURE_1D_ARRAY;
				}
				else
				{
					result = GL_TEXTURE_1D;
				}
				break;
			case VK_IMAGE_TYPE_2D:
				if ( layerCount > 1 )
				{
					if ( samples > VK_SAMPLE_COUNT_1_BIT )
					{
						result = GL_TEXTURE_2D_MULTISAMPLE_ARRAY;
					}
					else
					{
						result = GL_TEXTURE_2D_ARRAY;
					}
				}
				else
				{
					if ( samples != VK_SAMPLE_COUNT_1_BIT )
					{
						result = GL_TEXTURE_2D_MULTISAMPLE;
					}
					else
					{
						result = GL_TEXTURE_2D;
					}
				}
				break;
			case VK_IMAGE_TYPE_3D:
				result = GL_TEXTURE_3D;
				break;
			default:
				assert( false );
				result = GL_TEXTURE_2D;
				break;
			}

			return result;
		}
	}

	static GlTextureType convert( VkDevice device
		, VkImageType type
		, uint32_t layerCount
		, VkImageCreateFlags flags
		, VkSampleCountFlagBits samples )
	{
		if ( hasTextureViews( device ) )
		{
			return gl4::convert( type
				, layerCount
				, flags
				, samples );
		}

		return gl3::convert( type
			, layerCount
			, flags
			, samples );
	}

	Image::Image( VkAllocationCallbacks const * allocInfo
		, VkDevice device
		, VkFormat format
		, VkExtent2D const & dimensions
		, bool swapchainImage )
		: Image{ allocInfo
			, device
			, VkImageCreateInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO
				, nullptr
				, 0u
				, VK_IMAGE_TYPE_2D
				, format
				, VkExtent3D{ dimensions.width, dimensions.height, 1u }
				, 1u
				, 1u
				, VK_SAMPLE_COUNT_1_BIT
				, VK_IMAGE_TILING_OPTIMAL
				, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT
				, {} // sharingMode
				, {} // queueFamilyIndexCount
				, {} // pQueueFamilyIndices
				, {} } // initialLayout
			, swapchainImage }
	{
	}

	Image::Image( VkAllocationCallbacks const * allocInfo
		, VkDevice device
		, VkImageCreateInfo createInfo
		, bool swapchainImage )
		: m_allocInfo{ allocInfo }
		, m_device{ device }
		, m_createInfo{ std::move( createInfo ) }
		, m_target{ convert( device, getType(), getArrayLayers(), getCreateFlags(), getSamples() ) }
		, m_swapchainImage{ swapchainImage }
	{
		auto context = get( m_device )->getContext();
		m_pixelFormat = PixelFormat{ context
			, m_target
			, getFormatVk() };
		glLogCreateCall( context
			, glGenTextures
			, 1
			, &m_internal );
		glLogCall( context
			, glBindTexture
			, m_target
			, m_internal );
		m_pixelFormat.applySwizzle( context, m_target );
		glLogCall( context
			, glBindTexture
			, m_target
			, 0 );
		doInitialiseMemoryRequirements();
		registerObject( m_device, *this );
	}

	Image::~Image()
	{
		unregisterObject( m_device, *this );

		while ( !m_views.empty() )
		{
			destroyView( m_views.begin()->second );
		}

		if ( m_binding )
		{
			get( m_binding->getParent() )->unbindImage( get( this ) );
		}

		auto context = get( m_device )->getContext();
		glLogCall( context
			, glDeleteTextures
			, 1
			, &m_internal );
	}

	VkResult Image::createView( VkImageView & imageView
		, VkImageViewCreateInfo createInfo )
	{
		VkResult result = VK_SUCCESS;
		createInfo = ImageView::adjustCreateInfo( getDevice(), std::move( createInfo ) );
		auto lock = std::unique_lock< std::mutex >{ m_mtx };
		auto pair = m_views.emplace( createInfo, VkImageView{} );

		if ( pair.second )
		{
			result = allocate( pair.first->second
				, m_allocInfo
				, getDevice()
				, std::move( createInfo ) );
		}

		imageView = pair.first->second;
		return result;
	}

	VkImageView Image::createView( VkImageViewCreateInfo createInfo )
	{
		VkImageView imageView{};
		createView( imageView, std::move( createInfo ) );
		return imageView;
	}

	void Image::destroyView( VkImageView view )
	{
		auto lock = std::unique_lock< std::mutex >{ m_mtx };
		auto it = m_views.find( get( view )->getCreateInfo() );
		assert( it != m_views.end() );
		deallocate( it->second, m_allocInfo );
		m_views.erase( it );
	}

	VkMemoryRequirements Image::getMemoryRequirements()const
	{
		return m_memoryRequirements;
	}

	std::vector< VkSparseImageMemoryRequirements > Image::getSparseImageMemoryRequirements()const
	{
		return {};
	}

	void Image::doInitialiseMemoryRequirements()
	{
		auto physicalDevice = get( get( m_device )->getPhysicalDevice() );
		auto extent = ashes::getMinimalExtent3D( getFormatVk() );
		m_memoryRequirements.alignment = getSize( extent, getFormatVk() );
		m_memoryRequirements.size = getTotalSize( getDimensions(), getFormatVk(), getArrayLayers(), getMipLevels(), uint32_t( m_memoryRequirements.alignment ) );
		m_memoryRequirements.memoryTypeBits = physicalDevice->getMemoryTypeBits( VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
			, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT );
		m_memoryRequirements.size = ashes::getAlignedSize( m_memoryRequirements.size, m_memoryRequirements.alignment );
	}
}
