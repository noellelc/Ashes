/*
This file belongs to Ashes.
See LICENSE file in root folder
*/
#include "GlContextState.hpp"

namespace ashes::gl
{
	ContextState::ContextState( ContextState const & rhs )
		: cbStateAttachments{ rhs.cbStateAttachments }
		, cbState{ rhs.cbState }
		, dsState{ rhs.dsState }
		, sampleMask{ rhs.sampleMask }
		, msState{ rhs.msState }
		, tsState{ rhs.tsState }
		, iaState{ rhs.iaState }
		, vpState{ rhs.vpState }
		, rsState{ rhs.rsState }
		, dynamicStates{ rhs.dynamicStates }
		, dyState{ rhs.dyState }
		, viewports{ rhs.viewports }
		, scissors{ rhs.scissors }
	{
		doInit();
	}

	ContextState::ContextState( ContextState && rhs )
		: cbStateAttachments{ std::move( rhs.cbStateAttachments ) }
		, cbState{ std::move( rhs.cbState ) }
		, dsState{ std::move( rhs.dsState ) }
		, sampleMask{ std::move( rhs.sampleMask ) }
		, msState{ std::move( rhs.msState ) }
		, tsState{ std::move( rhs.tsState ) }
		, iaState{ std::move( rhs.iaState ) }
		, vpState{ std::move( rhs.vpState ) }
		, rsState{ std::move( rhs.rsState ) }
		, dynamicStates{ std::move( rhs.dynamicStates ) }
		, dyState{ std::move( rhs.dyState ) }
		, viewports{ std::move( rhs.viewports ) }
		, scissors{ std::move( rhs.scissors ) }
	{
		doInit();
	}

	ContextState::ContextState( Optional< VkPipelineColorBlendStateCreateInfo > cbState
		, Optional< VkPipelineDepthStencilStateCreateInfo > dsState
		, Optional< VkPipelineMultisampleStateCreateInfo > msState
		, Optional< VkPipelineTessellationStateCreateInfo > tsState
		, VkPipelineInputAssemblyStateCreateInfo iaState
		, VkPipelineViewportStateCreateInfo vpState
		, VkPipelineRasterizationStateCreateInfo rsState
		, VkPipelineDynamicStateCreateInfo dyState )
		: ContextState
		{
			cbState ? &cbState.value() : nullptr,
			dsState ? &dsState.value() : nullptr,
			msState ? &msState.value() : nullptr,
			tsState ? &tsState.value() : nullptr,
			&iaState,
			&vpState,
			&rsState,
			&dyState,
		}
	{
	}

	ContextState::ContextState( VkPipelineColorBlendStateCreateInfo cbState
		, VkPipelineDepthStencilStateCreateInfo const * dsState
		, VkPipelineMultisampleStateCreateInfo const * msState
		, VkPipelineTessellationStateCreateInfo const * tsState
		, VkPipelineInputAssemblyStateCreateInfo const * iaState
		, VkPipelineViewportStateCreateInfo const * vpState
		, VkPipelineRasterizationStateCreateInfo const * rsState
		, VkPipelineDynamicStateCreateInfo const * dyState )
		: ContextState
		{
			&cbState,
			dsState,
			msState,
			tsState,
			iaState,
			vpState,
			rsState,
			dyState,
		}
	{
	}

	ContextState::ContextState( VkPipelineColorBlendStateCreateInfo const * cbState
		, VkPipelineDepthStencilStateCreateInfo const * dsState
		, VkPipelineMultisampleStateCreateInfo const * msState
		, VkPipelineTessellationStateCreateInfo const * tsState
		, VkPipelineInputAssemblyStateCreateInfo const * iaState
		, VkPipelineViewportStateCreateInfo const * vpState
		, VkPipelineRasterizationStateCreateInfo const * rsState
		, VkPipelineDynamicStateCreateInfo const * dyState )
		: cbStateAttachments{ ( cbState
			? makeVector( cbState->pAttachments
				, cbState->attachmentCount )
			: VkPipelineColorBlendAttachmentStateArray{} ) }
		, cbState{ ( cbState
			? *cbState
			: getDeactivatedColorBlendState() ) }
		, dsState{ ( dsState
			? *dsState
			: getDeactivatedDepthStencilState() ) }
		, msState{ ( msState
			? *msState
			: getDeactivatedMultisampleState() ) }
		, tsState{ ( tsState
			? *tsState
			: getDeactivatedTessellationState() ) }
		, iaState{ ( iaState
			? *iaState
			: getDeactivatedInputAssemblyState() ) }
		, vpState{ ( vpState
			? *vpState
			: getDeactivatedViewportState() ) }
		, rsState{ ( rsState
			? *rsState
			: getDeactivatedRasterizationState() ) }
		, dynamicStates{ ( dyState
			? makeVector( dyState->pDynamicStates, dyState->dynamicStateCount )
			: VkDynamicStateArray{} ) }
		, dyState{ ( dyState
			? *dyState
			: getDeactivatedDynamicState() ) }
		, viewports{ ( vpState && vpState->pViewports
			? makeVector( vpState->pViewports, vpState->viewportCount )
			: VkViewportArray{} ) }
		, scissors{ ( vpState && vpState->pScissors
			? makeVector( vpState->pScissors, vpState->scissorCount )
			: VkScissorArray{} ) }
	{
		doInit();
	}

	ContextState & ContextState::swap( ContextState && rhs )
	{
		std::swap( rhs.cbStateAttachments, cbStateAttachments );
		std::swap( rhs.cbState, cbState );
		std::swap( rhs.dsState, dsState );
		std::swap( rhs.sampleMask, sampleMask );
		std::swap( rhs.msState, msState );
		std::swap( rhs.tsState, tsState );
		std::swap( rhs.iaState, iaState );
		std::swap( rhs.viewports, viewports );
		std::swap( rhs.scissors, scissors );
		std::swap( rhs.vpState, vpState );
		std::swap( rhs.rsState, rsState );
		std::swap( rhs.dynamicStates, dynamicStates );
		std::swap( rhs.dyState, dyState );
		doInit();
		return *this;
	}

	void ContextState::doInit()
	{
		cbState.pAttachments = cbStateAttachments.data();
		msState.pSampleMask = bool( sampleMask )
			? &sampleMask.value()
			: nullptr;
		vpState.pScissors = scissors.data();
		vpState.pViewports = viewports.data();
		dyState.pDynamicStates = dynamicStates.data();
	}
}
