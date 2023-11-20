#if 0
#if !_SERVER

// template definitions for the base render interface go here

#include "engine/render/backend/d3d11/d3d11_interface.h"
#include "engine/render/backend/opengl/opengl_interface.h"
#include "engine/utility/k_assert.h"
#include "vertex_types.h"

/****** explicit type definitions *******/

template std::unique_ptr<class KVertexBufferBase> 
KRenderInterface::CreateVertexBufferDynamic<TestVertex>
(const TVector<TestVertex>& vertices, const TVector<u32>& indices);

template std::unique_ptr<class KVertexBufferBase>
KRenderInterface::CreateVertexBufferStatic<TestVertex>
(const TVector<TestVertex>& vertices, const TVector<u32>& indices);



/************* function definitions **************/

template <typename VertexType>
std::unique_ptr<class KVertexBufferBase>
KRenderInterface::CreateVertexBufferDynamic(const TVector<VertexType>& vertices, const TVector<u32>& indices)
{
#if _WIN32
	if (D3D11Interface)
	{
		return D3D11Interface->CreateVertexBufferDynamic<VertexType>(vertices, indices);
	}
	else
	{
		return OpenGLInterface->CreateVertexBufferDynamic<VertexType>(vertices, indices);
	}
#else
	return OpenGLInterface->CreateVertexBufferDynamic<VertexType>(vertices, indices);
#endif

	K_ASSERT(true, "no child render interface pointer was valid in the base class");

	return std::unique_ptr<class KVertexBufferBase>(nullptr);
}

template <typename VertexType>
std::unique_ptr<class KVertexBufferBase>
KRenderInterface::CreateVertexBufferStatic(const TVector<VertexType>& vertices, const TVector<u32>& indices)
{
#if _WIN32
	if (D3D11Interface)
	{
		return D3D11Interface->CreateVertexBufferStatic<VertexType>(vertices, indices);
	}
	else
	{
		return OpenGLInterface->CreateVertexBufferStatic<VertexType>(vertices, indices);
	}
#else
	return OpenGLInterface->CreateVertexBufferStatic<VertexType>(vertices, indices);
#endif

	K_ASSERT(true, "no child render interface pointer was valid in the base class");

	return std::unique_ptr<class KVertexBufferBase>(nullptr);
}

#endif
#endif // 0