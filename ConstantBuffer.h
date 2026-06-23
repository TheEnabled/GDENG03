#pragma once
#include <d3d11.h>

class DeviceContext; // FIX: was 'deviceContext' (lowercase) — wrong class name,
// so the friend declaration never matched DeviceContext

class ConstantBuffer
{
public:
	ConstantBuffer();

	bool load(void* buffer, UINT size_buffer);
	void update(DeviceContext* context, void* buffer);
	bool release();
	~ConstantBuffer();

private:
	ID3D11Buffer* m_buffer;
private:
	friend class DeviceContext; // FIX: was 'deviceContext' — friendship never applied,
	// causing all private member access errors in DeviceContext.cpp
};