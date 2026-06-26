#pragma once

#include <d3d11.h>

class SwapChain
{
public:
	SwapChain();

	bool init(HWND hwnd, UINT width, UINT height);
	bool present(bool vsync);
	bool release();

	// expose RTV access for clearing
	ID3D11RenderTargetView*   getRenderTargetView()   { return m_rtv; }
	ID3D11DepthStencilView*   getDepthStencilView()   { return m_dsv; }

	~SwapChain();

private:
	IDXGISwapChain*           m_swap_chain;
	ID3D11RenderTargetView*   m_rtv;
	ID3D11DepthStencilView*   m_dsv                 = nullptr;
	ID3D11DepthStencilState*  m_depth_stencil_state = nullptr;
	ID3D11RasterizerState*    m_rasterizer_state    = nullptr;
};