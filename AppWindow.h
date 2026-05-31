#pragma once
#include "Window.h"
#include "GraphicsEngine.h"
#include "SwapChain.h"
#include "DeviceContext.h"
#include "VertexBuffer.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "ParticleSystem.h"
#include "ParticleEffects.h"
#include <chrono>

class AppWindow : public Window
{
public:
	AppWindow();
	~AppWindow();

    virtual void onCreate()  override;
	virtual void onUpdate()  override;
	virtual void onDestroy() override;

private:
	SwapChain*     m_swap_chain;
	VertexBuffer*  m_vb;           // static geometry VB (kept for future use)
	VertexBuffer*  m_particleVB;   // dynamic VB streamed each frame with live particles
	VertexShader*  m_vs;
	PixelShader*   m_ps;
	ParticleSystem* m_particleSystem;
	void*   m_vs_blob      = nullptr;
	size_t  m_vs_blob_size = 0;
	float   m_emit_accum     = 0.0f;
	float   m_emit_interval  = 2.0f;
	std::chrono::steady_clock::time_point m_last_time;
};