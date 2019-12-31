#include "Math.Core/Mesh.h"


////////////////////////////////////////////////////

struct Mesh::Impl
{

};


Mesh::Mesh()
	: mp_impl(std::make_unique<Impl>())
{
}
