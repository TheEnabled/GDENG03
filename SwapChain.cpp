#include "SwapChain.h"
#include "GraphicsEngine.h"

SwapChain::SwapChain()
	: m_swap_chain(nullptr)
	, m_rtv(nullptr)
	, m_dsv(nullptr)
	, m_depth_stencil_state(nullptr)
	, m_rasterizer_state(nullptr)
{
}

bool SwapChain::init(HWND hwnd, UINT width, UINT height)
{
	ID3D11Device* device = GraphicsEngine::get()->m_d3d_device;

	DXGI_SWAP_CHAIN_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.BufferCount = 1;
	desc.BufferDesc.Width = width;
	desc.BufferDesc.Height = height;
	desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.BufferDesc.RefreshRate.Numerator = 60;
	desc.BufferDesc.RefreshRate.Denominator = 1;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.OutputWindow = hwnd;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Windowed = TRUE;

	HRESULT hr = GraphicsEngine::get()->m_dxgi_factory->CreateSwapChain(device, &desc, &m_swap_chain);

	// ADD THIS — if CreateSwapChain fails, m_swap_chain stays nullptr
	// and calling GetBuffer on it causes the access violation
	if (FAILED(hr))
		return false;

	ID3D11Texture2D* buffer = NULL;
	hr = m_swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&buffer);

	if (FAILED(hr))
		return false;

	hr = device->CreateRenderTargetView(buffer, NULL, &m_rtv);
	buffer->Release();

	if (FAILED(hr))
		return false;

	// ---- Depth-stencil buffer ----
	// Create a texture with the same width/height as the back buffer
	D3D11_TEXTURE2D_DESC ds_desc = {};
	ds_desc.Width              = width;
	ds_desc.Height             = height;
	ds_desc.MipLevels          = 1;
	ds_desc.ArraySize          = 1;
	ds_desc.Format             = DXGI_FORMAT_D32_FLOAT;   // 32-bit depth
	ds_desc.SampleDesc.Count   = 1;
	ds_desc.SampleDesc.Quality = 0;
	ds_desc.Usage              = D3D11_USAGE_DEFAULT;
	ds_desc.BindFlags          = D3D11_BIND_DEPTH_STENCIL;

	ID3D11Texture2D* ds_texture = nullptr;
	hr = device->CreateTexture2D(&ds_desc, nullptr, &ds_texture);
	if (FAILED(hr))
		return false;

	hr = device->CreateDepthStencilView(ds_texture, nullptr, &m_dsv);
	ds_texture->Release();
	if (FAILED(hr))
		return false;

	// ---- Explicit depth stencil STATE ----
	// Without this, D3D11 may silently skip depth testing even with a DSV bound.
	D3D11_DEPTH_STENCIL_DESC ds_state_desc = {};
	ds_state_desc.DepthEnable                  = TRUE;
	ds_state_desc.DepthWriteMask               = D3D11_DEPTH_WRITE_MASK_ALL;
	ds_state_desc.DepthFunc                    = D3D11_COMPARISON_LESS;
	ds_state_desc.StencilEnable                = FALSE;

	hr = device->CreateDepthStencilState(&ds_state_desc, &m_depth_stencil_state);
	if (FAILED(hr))
		return false;

	// ---- Rasterizer state: cull NONE — rely on depth test for HRR ----
	// This removes any dependence on triangle winding order.
	D3D11_RASTERIZER_DESC rs_desc = {};
	rs_desc.FillMode              = D3D11_FILL_SOLID;
	rs_desc.CullMode              = D3D11_CULL_NONE;   // draw both sides
	rs_desc.FrontCounterClockwise = FALSE;
	rs_desc.DepthClipEnable       = TRUE;

	hr = device->CreateRasterizerState(&rs_desc, &m_rasterizer_state);
	if (FAILED(hr))
		return false;

	// Apply both states immediately so they are active from the first frame
	ID3D11DeviceContext* imm_ctx = nullptr;
	device->GetImmediateContext(&imm_ctx);
	imm_ctx->OMSetDepthStencilState(m_depth_stencil_state, 0);
	imm_ctx->RSSetState(m_rasterizer_state);
	imm_ctx->Release();

	return true;
}

bool SwapChain::present(bool vsync)
{
	m_swap_chain->Present(vsync, NULL);
	return true;
}

bool SwapChain::release()
{
	if (m_rasterizer_state)    { m_rasterizer_state->Release();    m_rasterizer_state    = nullptr; }
	if (m_depth_stencil_state) { m_depth_stencil_state->Release(); m_depth_stencil_state = nullptr; }
	if (m_dsv)                 { m_dsv->Release();                 m_dsv                 = nullptr; }
	if (m_rtv)                 { m_rtv->Release();                 m_rtv                 = nullptr; }
	if (m_swap_chain)          { m_swap_chain->Release();          m_swap_chain          = nullptr; }
	delete this;
	return true;
}

SwapChain::~SwapChain()
{
}