#include "D3D11RendererPrerequisites.hpp"

namespace ashes::d3d11
{
	D3D11_PRIMITIVE_TOPOLOGY getPrimitiveTopology( VkPrimitiveTopology const & topology )
	{
		switch ( topology )
		{
		case VK_PRIMITIVE_TOPOLOGY_POINT_LIST:
			return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;

		case VK_PRIMITIVE_TOPOLOGY_LINE_LIST:
			return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;

		case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP:
			return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;

		case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
			return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
			return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

		case VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY:
			return D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;

		case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY:
			return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;

		case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY:
			return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;

		case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY:
			return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;

		case VK_PRIMITIVE_TOPOLOGY_PATCH_LIST:
			return D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST;

		default:
			assert( false );
			return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
		}
	}
}
