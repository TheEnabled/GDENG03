#include "IndexBuffer.h"
#include "GraphicsEngine.h"


IndexBuffer::IndexBuffer() : m_buffer(0), m_size_list(0)
{
}

bool IndexBuffer::load(void* list_indices, UINT size_list)
{
	// Release any previously allocated buffer before creating a new one
	if (m_buffer) { m_buffer->Release(); m_buffer = nullptr; }

	D3D11_BUFFER_DESC buff_desc = {};
	buff_desc.Usage          = D3D11_USAGE_DEFAULT;
	buff_desc.ByteWidth      = sizeof(UINT) * size_list; // each index is a 32-bit UINT
	buff_desc.BindFlags      = D3D11_BIND_INDEX_BUFFER;  // index buffer, not vertex buffer
	buff_desc.CPUAccessFlags = 0;
	buff_desc.MiscFlags      = 0;
	buff_desc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA init_data = {};
	init_data.pSysMem       = list_indices;
	init_data.SysMemPitch   = 0;
	init_data.SysMemSlicePitch = 0;

	m_size_list = size_list;

	if (FAILED(GraphicsEngine::get()->m_d3d_device->CreateBuffer(&buff_desc, &init_data, &m_buffer)))
	{
		return false;
	}

	return true;
}

UINT IndexBuffer::getSizeIndexList() const
{
	return m_size_list;
}

bool IndexBuffer::release()
{
	if (m_buffer) { m_buffer->Release(); m_buffer = nullptr; }
	delete this;
	return true;
}

IndexBuffer::~IndexBuffer()
{
}