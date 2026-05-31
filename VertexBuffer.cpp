#include "VertexBuffer.h"
#include "GraphicsEngine.h"


VertexBuffer::VertexBuffer() :m_layout(0), m_buffer(0), m_size_list(0), m_size_vertex(0)
{
}

bool VertexBuffer::load(void* list_vertices, UINT size_vertex, UINT size_list, void* shader_byte_code, UINT size_byte_shader)
{

    if (m_buffer) { m_buffer->Release(); m_buffer = nullptr; }
	if (m_layout) { m_layout->Release(); m_layout = nullptr; }

    D3D11_BUFFER_DESC buff_desc = {};
	buff_desc.Usage = D3D11_USAGE_DEFAULT;
	buff_desc.ByteWidth = size_vertex * size_list;
	buff_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buff_desc.CPUAccessFlags = 0;
	buff_desc.MiscFlags = 0;
	buff_desc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA init_data = {};
	init_data.pSysMem = list_vertices;
	init_data.SysMemPitch = 0;
	init_data.SysMemSlicePitch = 0;

	m_size_vertex = size_vertex;
	m_size_list = size_list;


	if (FAILED(GraphicsEngine::get()->m_d3d_device->CreateBuffer(&buff_desc, &init_data, &m_buffer)))
	{
		return false;
	}

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		// FIX: changed from R32G32B32A32_FLOAT (4 floats) to R32G32B32_FLOAT (3 floats)
		// to match the vec3 struct { float x, y, z } used in AppWindow.cpp
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
		,{"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}

	};

	UINT size_layout = ARRAYSIZE(layout);

	if (FAILED(GraphicsEngine::get()->m_d3d_device->CreateInputLayout(layout, size_layout, shader_byte_code, size_byte_shader, &m_layout)))
	{
		return false;
	}

	return true;
}

bool VertexBuffer::loadDynamic(UINT size_vertex, UINT max_vertices, void* shader_byte_code, UINT size_byte_shader)
{
	if (m_buffer) { m_buffer->Release(); m_buffer = nullptr; }
	if (m_layout) { m_layout->Release(); m_layout = nullptr; }

	m_size_vertex = size_vertex;
	m_size_list   = max_vertices;

	// D3D11_USAGE_DYNAMIC lets us Map/Unmap every frame from the CPU
	D3D11_BUFFER_DESC desc = {};
	desc.Usage          = D3D11_USAGE_DYNAMIC;
	desc.ByteWidth      = size_vertex * max_vertices;
	desc.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	if (FAILED(GraphicsEngine::get()->m_d3d_device->CreateBuffer(&desc, nullptr, &m_buffer)))
		return false;

	// Particle render vertex: float3 position + float3 color (matches VertexShader.hlsl)
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	if (FAILED(GraphicsEngine::get()->m_d3d_device->CreateInputLayout(
		layout, ARRAYSIZE(layout),
		shader_byte_code, size_byte_shader,
		&m_layout)))
		return false;

	return true;
}

bool VertexBuffer::updateDynamic(void* data, UINT vertex_count)
{
	if (!m_buffer || vertex_count == 0) return false;

	ID3D11DeviceContext* ctx = nullptr;
	GraphicsEngine::get()->m_d3d_device->GetImmediateContext(&ctx);

	D3D11_MAPPED_SUBRESOURCE mapped = {};
	if (FAILED(ctx->Map(m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
	{
		ctx->Release();
		return false;
	}

	UINT copy_count = (vertex_count < m_size_list) ? vertex_count : m_size_list;
	memcpy(mapped.pData, data, static_cast<size_t>(m_size_vertex) * copy_count);

	ctx->Unmap(m_buffer, 0);
	ctx->Release();

	m_size_list = copy_count; // track live vert count for the draw call
	return true;
}

UINT VertexBuffer::getSizeVertexList() const
{
	return this->m_size_list;
}

bool VertexBuffer::release()
{
    if (m_layout) { m_layout->Release(); m_layout = nullptr; }
	if (m_buffer) { m_buffer->Release(); m_buffer = nullptr; }
	delete this;

	return true;
}

VertexBuffer::~VertexBuffer()
{
}