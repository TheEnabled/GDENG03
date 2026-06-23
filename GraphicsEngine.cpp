#include "GraphicsEngine.h"
#include "SwapChain.h"
#include "DeviceContext.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ConstantBuffer.h"
#include "VertexShader.h"
#include "PixelShader.h"

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

GraphicsEngine::GraphicsEngine()
	: m_d3d_device(nullptr)
	, m_feature_level(D3D_FEATURE_LEVEL_11_0)
	, m_imm_device_context(nullptr)
	, m_dxgi_device(nullptr)
	, m_dxgi_adapter(nullptr)
	, m_dxgi_factory(nullptr)
	, m_blob(nullptr)
	, m_psblob(nullptr)
	// FIX: removed m_vsblob and raw m_vs initializers — no longer needed
	, m_ps(nullptr)
{
}

bool GraphicsEngine::init()
{
	D3D_DRIVER_TYPE driver_types[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT num_driver_types = ARRAYSIZE(driver_types);

	D3D_FEATURE_LEVEL feature_levels[] = { D3D_FEATURE_LEVEL_11_0 };
	UINT num_feature_levels = ARRAYSIZE(feature_levels);

	HRESULT res = 0;
	ID3D11DeviceContext* imm_context = nullptr;

	for (UINT driver_type_index = 0; driver_type_index < num_driver_types;)
	{
		res = D3D11CreateDevice(NULL, driver_types[driver_type_index], NULL, NULL,
			feature_levels, num_feature_levels, D3D11_SDK_VERSION,
			&m_d3d_device, &m_feature_level, &imm_context);

		if (SUCCEEDED(res))
			break;
		++driver_type_index;
	}

	if (FAILED(res))
		return false;

	m_imm_device_context = new DeviceContext(imm_context);

	if (FAILED(m_d3d_device->QueryInterface(__uuidof(IDXGIDevice), (void**)&m_dxgi_device)))  return false;
	if (FAILED(m_dxgi_device->GetParent(__uuidof(IDXGIAdapter), (void**)&m_dxgi_adapter))) return false;
	if (FAILED(m_dxgi_adapter->GetParent(__uuidof(IDXGIFactory), (void**)&m_dxgi_factory))) return false;

	// FIX: removed inline shader compilation — vertex shader is now compiled
	// from a .hlsl file via compileVertexShaders(), and pixel shader via createShaders()

	return true;
}

SwapChain* GraphicsEngine::createSwapChain()
{
	return new SwapChain();
}

DeviceContext* GraphicsEngine::getImmediateDeviceContext()
{
	return this->m_imm_device_context;
}

VertexBuffer* GraphicsEngine::createVertexBuffer()
{
	return new VertexBuffer();
}

IndexBuffer* GraphicsEngine::createIndexBuffer()
{
	return new IndexBuffer();
}

ConstantBuffer* GraphicsEngine::createConstantBuffer()
{
	return new ConstantBuffer();
}

VertexShader* GraphicsEngine::createVertexShader(const void* shader_byte_code, size_t byte_code_size)
{
	VertexShader* vs = new VertexShader();

	if (!vs->init(shader_byte_code, byte_code_size))
	{
		vs->release();
		return nullptr;
	}

	return vs;
}

PixelShader* GraphicsEngine::createPixelShader(const void* shader_byte_code, size_t byte_code_size)
{
	PixelShader* ps = new PixelShader();

	if (!ps->init(shader_byte_code, byte_code_size))
	{
		ps->release();
		return nullptr;
	}

	return ps;
}

bool GraphicsEngine::compileVertexShaders(const wchar_t* file_name, const char* entry_point_name, void** shader_byte_code, size_t* byte_code_size)
{
	ID3DBlob* error_blob = nullptr;
	if (FAILED(D3DCompileFromFile(file_name, nullptr, nullptr, entry_point_name,
		"vs_5_0", 0, 0, &m_blob, &error_blob)))
	{
		if (error_blob) error_blob->Release();
		return false;
	}

	*shader_byte_code = m_blob->GetBufferPointer();
	*byte_code_size = m_blob->GetBufferSize();

	return true;
}

bool GraphicsEngine::compilePixelShaders(const wchar_t* file_name, const char* entry_point_name,
	void** shader_byte_code, size_t* byte_code_size)
{
	ID3DBlob* error_blob = nullptr;
	if (FAILED(D3DCompileFromFile(file_name, nullptr, nullptr, entry_point_name,
		"ps_5_0", 0, 0, &m_blob, &error_blob)))
	{
		if (error_blob) error_blob->Release();
		return false;
	}

	*shader_byte_code = m_blob->GetBufferPointer();
	*byte_code_size = m_blob->GetBufferSize();

	return true;
}

void GraphicsEngine::releaseCompiledShaders()
{
	if (m_blob) { m_blob->Release(); m_blob = nullptr; }
}

bool GraphicsEngine::release()
{
	if (m_ps) { m_ps->Release();     m_ps = nullptr; }
	if (m_psblob) { m_psblob->Release(); m_psblob = nullptr; }

	m_dxgi_factory->Release();
	m_dxgi_adapter->Release();
	m_dxgi_device->Release();
	m_imm_device_context->release();
	m_d3d_device->Release();

	return true;
}

GraphicsEngine::~GraphicsEngine()
{
}

GraphicsEngine* GraphicsEngine::get()
{
	static GraphicsEngine engine;
	return &engine;
}