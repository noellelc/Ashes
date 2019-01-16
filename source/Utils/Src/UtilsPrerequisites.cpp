/*
This file belongs to Ashes.
See LICENSE file in root folder
*/
#include "UtilsPrerequisites.hpp"

#include "Vec2.hpp"
#include "Vec3.hpp"
#include "Vec4.hpp"

#include <Miscellaneous/Extent2D.hpp>
#include <Miscellaneous/Extent3D.hpp>
#include <Miscellaneous/Offset2D.hpp>
#include <Miscellaneous/Offset3D.hpp>
#include <RenderPass/ClearValue.hpp>

#include <cassert>

namespace utils
{
	ashes::Extent2D makeExtent2D( UIVec2 const & value )
	{
		return { value[0], value[1] };
	}

	ashes::Extent3D makeExtent3D( UIVec2 const & value )
	{
		return { value[0], value[1], 1u };
	}

	ashes::Extent3D makeExtent3D( UIVec3 const & value )
	{
		return { value[0], value[1], value[2] };
	}

	ashes::Offset2D makeOffset2D( IVec2 const & value )
	{
		return { value[0], value[1] };
	}

	ashes::Offset3D makeOffset3D( IVec3 const & value )
	{
		return { value[0], value[1], value[2] };
	}

	ashes::ClearColorValue makeClearColorValue( RgbColour const & value )
	{
		ashes::ClearColorValue result;
		result.float32[0] = value.r;
		result.float32[1] = value.g;
		result.float32[2] = value.b;
		result.float32[3] = 1.0f;
		return result;
	}

	ashes::ClearColorValue makeClearColorValue( RgbaColour const & value )
	{
		ashes::ClearColorValue result;
		result.float32[0] = value.r;
		result.float32[1] = value.g;
		result.float32[2] = value.b;
		result.float32[3] = value.a;
		return result;
	}
}
