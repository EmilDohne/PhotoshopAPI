#include "Macros.h"

#include "Mesh.h"

PSAPI_NAMESPACE_BEGIN

namespace Geometry
{

	template struct Face<double, 4>;
	template struct OctreeNode<double, 128>;
	template class Octree<double, 128>;
	template struct QuadMesh<double>;

}

PSAPI_NAMESPACE_END