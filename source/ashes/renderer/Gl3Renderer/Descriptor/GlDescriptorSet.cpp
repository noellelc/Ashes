#include "Descriptor/GlDescriptorSet.hpp"

#include "Buffer/GlBuffer.hpp"
#include "Descriptor/GlDescriptorPool.hpp"
#include "Image/GlSampler.hpp"
#include "Image/GlImage.hpp"

#include "ashesgl3_api.hpp"

#include <ashes/common/VkTypeTraits.hpp>

#include <algorithm>

namespace ashes::gl3
{
	DescriptorSet::DescriptorSet( VkDescriptorPool pool
		, VkDescriptorSetLayout layout )
		: m_pool{ pool }
		, m_layout{ layout }
	{
		get( pool )->registerSet( get( this ) );

		for ( auto & binding : *get( layout ) )
		{
			LayoutBindingWrites bindingWrites
			{
				binding.binding,
				binding.descriptorCount,
				binding.descriptorType
			};
			m_writes.insert( { binding.binding, bindingWrites } );
		}

		for ( auto & write : m_writes )
		{
			switch ( write.second.descriptorType )
			{
			case VK_DESCRIPTOR_TYPE_SAMPLER:
				m_samplers.push_back( &write.second );
				break;
			case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
				m_combinedTextureSamplers.push_back( &write.second );
				break;
			case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
				m_sampledTextures.push_back( &write.second );
				break;
			case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
				m_storageTextures.push_back( &write.second );
				break;
			case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
				m_texelBuffers.push_back( &write.second );
				break;
			case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
				m_texelBuffers.push_back( &write.second );
				break;
			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
				m_uniformBuffers.push_back( &write.second );
				break;
			case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
				m_storageBuffers.push_back( &write.second );
				break;
			case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
				m_inputAttachments.push_back( &write.second );
				break;
			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
				m_dynamicUniformBuffers.push_back( &write.second );
				m_dynamicBuffers.push_back( &write.second );
				break;
			case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
				m_dynamicStorageBuffers.push_back( &write.second );
				m_dynamicBuffers.push_back( &write.second );
				break;
#if VK_EXT_inline_uniform_block
			case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT:
				m_inlineUniforms.push_back( &write.second );
				break;
#endif
#if VK_NV_ray_tracing
			case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:
				break;
#endif
			default:
				break;
			}
		}

		std::sort( m_dynamicBuffers.begin()
			, m_dynamicBuffers.end()
			, []( LayoutBindingWrites const * lhs
				, LayoutBindingWrites const * rhs )
			{
				return lhs->binding < rhs->binding;
			} );
	}

	DescriptorSet::~DescriptorSet()
	{
		for ( auto & inlineUbo : m_inlineUbos )
		{
			deallocate( inlineUbo->buffer, nullptr );
			deallocate( inlineUbo->memory, nullptr );
		}
	}

	void DescriptorSet::mergeWrites( LayoutBindingWrites & writes, VkWriteDescriptorSet const & write )
	{
		writes.writes.push_back( write );
		auto & myWrite = writes.writes.back();

		if ( myWrite.pImageInfo )
		{
			m_imagesInfos.emplace_back( std::vector< VkDescriptorImageInfo >{ myWrite.pImageInfo, myWrite.pImageInfo + myWrite.descriptorCount } );
			myWrite.pImageInfo = m_imagesInfos.back().data();
		}

#if VK_EXT_inline_uniform_block

		auto inlineUniform = tryGet< VkWriteDescriptorSetInlineUniformBlockEXT >( myWrite.pNext );

		if ( inlineUniform )
		{
			writes.descriptorCount /= inlineUniform->dataSize;
			myWrite.descriptorCount /= inlineUniform->dataSize;
			auto device = get( m_pool )->getDevice();
			auto inlineUbo = createInlineUbo( device
				, *inlineUniform
				, Instance::getMemoryProperties()
				, ashes::gl3::vkCreateBuffer
				, ashes::gl3::vkGetBufferMemoryRequirements
				, ashes::gl3::vkAllocateMemory
				, ashes::gl3::vkBindBufferMemory
				, ashes::gl3::vkMapMemory
				, ashes::gl3::vkFlushMappedMemoryRanges
				, ashes::gl3::vkUnmapMemory );
			myWrite.pBufferInfo = &inlineUbo->info;
			m_inlineUbos.emplace_back( std::move( inlineUbo ) );
		}

#endif

		if ( myWrite.pBufferInfo )
		{
			m_buffersInfos.emplace_back( std::vector< VkDescriptorBufferInfo >{ myWrite.pBufferInfo, myWrite.pBufferInfo + myWrite.descriptorCount } );
			myWrite.pBufferInfo = m_buffersInfos.back().data();
		}
	}

	void DescriptorSet::update( VkWriteDescriptorSet const & write )
	{
		auto it = m_writes.find( write.dstBinding );
		assert( it != m_writes.end() );
		assert( it->second.descriptorType == write.descriptorType );
		assert( it->second.descriptorCount >= write.dstArrayElement + write.descriptorCount );
		assert( write.dstSet == get( this ) );
		mergeWrites( it->second, write );
	}

	void DescriptorSet::update( VkCopyDescriptorSet const & copy )
	{
		//m_writes[copy.dstBinding].dstBinding = copy.srcBinding;
		//m_writes[copy.dstBinding].dstArrayElement = copy.srcArrayElement;
	}
}
