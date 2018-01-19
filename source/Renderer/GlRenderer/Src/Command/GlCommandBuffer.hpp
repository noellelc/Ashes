/*
This file belongs to Renderer.
See LICENSE file in root folder
*/
#pragma once

#include "Commands/GlCommandBase.hpp"

#include <Command/CommandBuffer.hpp>

namespace gl_renderer
{
	/**
	*\brief
	*	Emulation d'un command buffer, à la manière de Vulkan.
	*/
	class CommandBuffer
		: public renderer::CommandBuffer
	{
	public:
		/**
		*\brief
		*	Constructeur.
		*\param[in] device
		*	Le périphérique logique.
		*\param[in] pool
		*	Le pool de tampons de commandes.
		*\param[in] primary
		*	Dit si le tampon est un tampon de commandes primaire (\p true) ou secondaire (\p false).
		*/
		CommandBuffer( renderer::Device const & device
			, renderer::CommandPool const & pool
			, bool primary );
		/**
		*\copydoc	renderer::CommandBuffer:begin
		*/
		bool begin( renderer::CommandBufferUsageFlags flags )const override;
		/**
		*\copydoc	renderer::CommandBuffer:begin
		*/
		bool begin( renderer::CommandBufferUsageFlags flags
			, renderer::RenderPass const & renderPass
			, uint32_t subpass
			, renderer::FrameBuffer const & frameBuffer
			, bool occlusionQueryEnable
			, renderer::QueryControlFlags queryFlags
			, renderer::QueryPipelineStatisticFlags pipelineStatistics )const override;
		/**
		*\copydoc	renderer::CommandBuffer:end
		*/
		bool end()const override;
		/**
		*\copydoc	renderer::CommandBuffer:reset
		*/
		bool reset( renderer::CommandBufferResetFlags flags )const override;
		/**
		*\copydoc	renderer::CommandBuffer:beginRenderPass
		*/
		void beginRenderPass( renderer::RenderPass const & renderPass
			, renderer::FrameBuffer const & frameBuffer
			, renderer::ClearValueArray const & clearValues
			, renderer::SubpassContents contents )const override;
		/**
		*\copydoc	renderer::CommandBuffer:nextSubpass
		*/
		void nextSubpass( renderer::SubpassContents contents )const override;
		/**
		*\copydoc	renderer::CommandBuffer:endRenderPass
		*/
		void endRenderPass()const override;
		/**
		*\copydoc	renderer::CommandBuffer:executeCommands
		*/
		void executeCommands( renderer::CommandBufferCRefArray const & commands )const override;
		/**
		*\copydoc	renderer::CommandBuffer:clear
		*/
		void clear( renderer::Texture const & image
			, renderer::RgbaColour const & colour )const override;
		/**
		*\copydoc	renderer::CommandBuffer:memoryBarrier
		*/
		void memoryBarrier( renderer::PipelineStageFlags after
			, renderer::PipelineStageFlags before
			, renderer::BufferMemoryBarrier const & transitionBarrier )const override;
		/**
		*\copydoc	renderer::CommandBuffer:memoryBarrier
		*/
		void memoryBarrier( renderer::PipelineStageFlags after
			, renderer::PipelineStageFlags before
			, renderer::ImageMemoryBarrier const & transitionBarrier )const override;
		/**
		*\copydoc	renderer::CommandBuffer:bindPipeline
		*/
		void bindPipeline( renderer::Pipeline const & pipeline
			, renderer::PipelineBindPoint bindingPoint )const override;
		/**
		*\copydoc	renderer::CommandBuffer:bindGeometryBuffers
		*/
		void bindGeometryBuffers( renderer::GeometryBuffers const & geometryBuffers )const override;
		/**
		*\copydoc	renderer::CommandBuffer:bindDescriptorSet
		*/
		void bindDescriptorSet( renderer::DescriptorSet const & descriptorSet
			, renderer::PipelineLayout const & layout
			, renderer::PipelineBindPoint bindingPoint )const override;
		/**
		*\copydoc	renderer::CommandBuffer:setViewport
		*/
		void setViewport( renderer::Viewport const & viewport )const override;
		/**
		*\copydoc	renderer::CommandBuffer:setScissor
		*/
		void setScissor( renderer::Scissor const & scissor )const override;
		/**
		*\copydoc	renderer::CommandBuffer:draw
		*/
		void draw( uint32_t vtxCount
			, uint32_t instCount
			, uint32_t firstVertex
			, uint32_t firstInstance )const override;
		/**
		*\copydoc	renderer::CommandBuffer:drawIndexed
		*/
		void drawIndexed( uint32_t indexCount
			, uint32_t instCount
			, uint32_t firstIndex
			, uint32_t vertexOffset
			, uint32_t firstInstance )const override;
		/**
		*\copydoc	renderer::CommandBuffer:copyBuffer
		*/
		void copyBuffer( renderer::BufferBase const & src
			, renderer::BufferBase const & dst
			, uint32_t size
			, uint32_t offset )const override;
		/**
		*\copydoc	renderer::CommandBuffer:copyBuffer
		*/
		void copyBuffer( renderer::BufferBase const & src
			, renderer::VertexBufferBase const & dst
			, uint32_t size
			, uint32_t offset )const override;
		/**
		*\copydoc	renderer::CommandBuffer:copyBuffer
		*/
		void copyBuffer( renderer::VertexBufferBase const & src
			, renderer::BufferBase const & dst
			, uint32_t size
			, uint32_t offset )const override;
		/**
		*\copydoc	renderer::CommandBuffer:copyBuffer
		*/
		void copyBuffer( renderer::VertexBufferBase const & src
			, renderer::VertexBufferBase const & dst
			, uint32_t size
			, uint32_t offset )const override;
		/**
		*\copydoc	renderer::CommandBuffer:copyBuffer
		*/
		void copyBuffer( renderer::BufferBase const & src
			, renderer::UniformBufferBase const & dst
			, uint32_t size
			, uint32_t offset )const override;
		/**
		*\copydoc	renderer::CommandBuffer:copyBuffer
		*/
		void copyBuffer( renderer::UniformBufferBase const & src
			, renderer::BufferBase const & dst
			, uint32_t size
			, uint32_t offset )const override;
		/**
		*\copydoc	renderer::CommandBuffer:copyBuffer
		*/
		void copyBuffer( renderer::UniformBufferBase const & src
			, renderer::UniformBufferBase const & dst
			, uint32_t size
			, uint32_t offset )const override;
		/**
		*\copydoc	renderer::CommandBuffer:copyImage
		*/
		void copyImage( renderer::BufferBase const & src
			, renderer::Texture const & dst )const override;
		/**
		*\copydoc	renderer::CommandBuffer:copyImage
		*/
		void copyImage( renderer::StagingBuffer const & src
			, renderer::Texture const & dst )const override;
		/**
		*\copydoc	renderer::CommandBuffer:resetQueryPool
		*/
		void resetQueryPool( renderer::QueryPool const & pool
			, uint32_t firstQuery
			, uint32_t queryCount )const override;
		/**
		*\copydoc	renderer::CommandBuffer:beginQuery
		*/
		void beginQuery( renderer::QueryPool const & pool
			, uint32_t query
			, renderer::QueryControlFlags flags )const override;
		/**
		*\copydoc	renderer::CommandBuffer:endQuery
		*/
		void endQuery( renderer::QueryPool const & pool
			, uint32_t query )const override;
		/**
		*\copydoc	renderer::CommandBuffer:writeTimestamp
		*/
		void writeTimestamp( renderer::PipelineStageFlag pipelineStage
			, renderer::QueryPool const & pool
			, uint32_t query )const override;
		/**
		*\return
		*	Le tableau de commandes.
		*/
		inline CommandArray const & getCommands()const
		{
			return m_commands;
		}

	private:
		mutable renderer::CommandBufferUsageFlags m_beginFlags{ 0u };
		mutable CommandArray m_commands;
		mutable renderer::Pipeline const * m_currentPipeline{ nullptr };
		mutable renderer::IndexType m_indexType{ renderer::IndexType::eUInt32 };
	};
}