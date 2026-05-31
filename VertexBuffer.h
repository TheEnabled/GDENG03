#pragma once
#include <d3d11.h>

class DeviceContext; // FIX: was 'deviceContext' (lowercase) — wrong class name,
// so the friend declaration never matched DeviceContext

class VertexBuffer
{
public:
	VertexBuffer();
	// Static geometry buffer (GPU-only, created once)
	bool load(void* list_vertices, UINT size_vertex, UINT size_list, void* shader_byte_code, UINT size_byte_shader);
	// Dynamic buffer (CPU-writable each frame) — used by particle system
	bool loadDynamic(UINT size_vertex, UINT max_vertices, void* shader_byte_code, UINT size_byte_shader);
	// Upload new data into a dynamic buffer each frame
	bool updateDynamic(void* data, UINT vertex_count);
	UINT getSizeVertexList() const;
	bool release();
	~VertexBuffer();

private:
	UINT m_size_vertex;
	UINT m_size_list;

private:
	ID3D11Buffer* m_buffer;
	ID3D11InputLayout* m_layout;

private:
	friend class DeviceContext; // FIX: was 'deviceContext' — friendship never applied,
	// causing all private member access errors in DeviceContext.cpp
};