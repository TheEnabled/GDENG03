#pragma once
#include <d3d11.h>

class DeviceContext;

class IndexBuffer
{
public:
	IndexBuffer();
	// Upload a static list of indices to the GPU (created once, never changed)
	// list_indices : pointer to UINT array
	// size_list    : number of indices in the array
	bool load(void* list_indices, UINT size_list);
	UINT getSizeIndexList() const;
	bool release();
	~IndexBuffer();

private:
	UINT m_size_list;   // number of indices stored in the buffer

private:
	ID3D11Buffer* m_buffer;

private:
	friend class DeviceContext;
	friend class GraphicsEngine;
};