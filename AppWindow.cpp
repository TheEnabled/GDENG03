#include "AppWindow.h"
#include "VertexBuffer.h"
#include <chrono>
#include <vector>
#include "Particle.h"

// -----------------------------------------------------------------------
// Minimal render vertex that matches VertexShader.hlsl:
//   float3 position : POSITION
//   float3 color    : COLOR
// We copy from Particle into this struct before uploading to the GPU.
// -----------------------------------------------------------------------
struct ParticleVertex
{
    float x, y, z;   // position
    float r, g, b;   // color  (alpha is handled by the update fade, not sent to GPU here)
};

AppWindow::AppWindow()
    : m_swap_chain(nullptr)
    , m_vb(nullptr)
    , m_particleVB(nullptr)
    , m_vs(nullptr)
    , m_ps(nullptr)
    , m_particleSystem(nullptr)
{
}

AppWindow::~AppWindow()
{
}

void AppWindow::onCreate()
{
    GraphicsEngine::get()->init();

    m_swap_chain = GraphicsEngine::get()->createSwapChain();
    RECT rc = getClientWindowRect();
    m_swap_chain->init(this->m_hwnd, rc.right - rc.left, rc.bottom - rc.top);

    // ---- Compile shaders and keep byte-code for input layout creation ----
    void* shader_byte_code = nullptr;
    size_t size_shader = 0;

    GraphicsEngine::get()->compileVertexShaders(L"VertexShader.hlsl", "vsmain", &shader_byte_code, &size_shader);
    m_vs_blob      = shader_byte_code;
    m_vs_blob_size = size_shader;
    m_vs = GraphicsEngine::get()->createVertexShader(shader_byte_code, size_shader);
    // NOTE: do NOT call releaseCompiledShaders() yet — we need the bytecode below.

    GraphicsEngine::get()->compilePixelShaders(L"PixelShader.hlsl", "psmain", &shader_byte_code, &size_shader);
    m_ps = GraphicsEngine::get()->createPixelShader(shader_byte_code, size_shader);
    GraphicsEngine::get()->releaseCompiledShaders(); // safe to release pixel shader blob now

    // ---- Create the particle system ----
    m_particleSystem = new ParticleSystem(2000);

    // Fire an initial explosion so particles appear immediately
    ParticleEffects::fountain(*m_particleSystem, Vec3{0.0f, -0.5f, 0.0f}, 600, 1.0f);


    // ---- Create the dynamic vertex buffer that we'll stream into each frame ----
    // max_vertices = 2000, matching the ParticleSystem capacity
    m_particleVB = GraphicsEngine::get()->createVertexBuffer();
    m_particleVB->loadDynamic(
        sizeof(ParticleVertex),
        2000,
        m_vs_blob,        // vertex shader bytecode needed to create the input layout
        (UINT)m_vs_blob_size
    );
    // Now safe to release the vertex shader blob
    GraphicsEngine::get()->releaseCompiledShaders();

    // Start the frame timer
    m_last_time = std::chrono::steady_clock::now();
}

void AppWindow::onUpdate()
{
    // ---- Delta time ----
    auto now = std::chrono::steady_clock::now();
    float dt = std::chrono::duration<float>(now - m_last_time).count();
    m_last_time = now;

    // ---- Update particle simulation ----
    m_particleSystem->update(dt);

    // ---- Periodically emit a new fountain burst ----
    m_emit_accum += dt;
    if (m_emit_accum >= m_emit_interval)
    {
        m_emit_accum = 0.0f;
        ParticleEffects::fountain(*m_particleSystem, Vec3{0.0f, 0.0f, 0.0f}, 50, 1.0f);
    }

    // ---- Build CPU-side render vertex array from alive particles ----
    std::vector<ParticleVertex> verts;
    verts.reserve(m_particleSystem->particles().size());

    for (const Particle& p : m_particleSystem->particles())
    {
        if (!p.alive) continue;
        ParticleVertex v;
        v.x = p.position.x;
        v.y = p.position.y;
        v.z = p.position.z;
        v.r = p.color.r * p.color.a;   // pre-multiply alpha for simple fade
        v.g = p.color.g * p.color.a;
        v.b = p.color.b * p.color.a;
        verts.push_back(v);
    }

    // ---- Upload to GPU ----
    UINT alive_count = static_cast<UINT>(verts.size());
    if (alive_count > 0)
        m_particleVB->updateDynamic(verts.data(), alive_count);

    // ---- Render ----
    GraphicsEngine::get()->getImmediateDeviceContext()->clearRenderTargetColor(
        this->m_swap_chain, 0, 0.05f, 0.07f, 1);

    RECT rc = getClientWindowRect();
    GraphicsEngine::get()->getImmediateDeviceContext()->setViewportSize(
        rc.right - rc.left, rc.bottom - rc.top);

    GraphicsEngine::get()->getImmediateDeviceContext()->setVertexShader(m_vs);
    GraphicsEngine::get()->getImmediateDeviceContext()->setPixelShader(m_ps);

    if (alive_count > 0)
    {
        GraphicsEngine::get()->getImmediateDeviceContext()->setVertexBuffer(m_particleVB);
        GraphicsEngine::get()->getImmediateDeviceContext()->drawPointList(alive_count, 0);
    }

    m_swap_chain->present(false);
}

void AppWindow::onDestroy()
{
    Window::onDestroy();

    if (m_particleVB)  { m_particleVB->release();  m_particleVB  = nullptr; }
    if (m_vb)          { m_vb->release();           m_vb          = nullptr; }
    if (m_vs)          { m_vs->release();           m_vs          = nullptr; }
    if (m_ps)          { m_ps->release();           m_ps          = nullptr; }
    if (m_swap_chain)  { m_swap_chain->release();   m_swap_chain  = nullptr; }
    delete m_particleSystem; m_particleSystem = nullptr;
    GraphicsEngine::get()->release();
}