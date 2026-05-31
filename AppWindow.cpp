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

    // Seed the pool with one initial batch so the screen isn't empty at startup.
    // onUpdate() will continuously top it up each frame.
    ParticleEffects::rain(*m_particleSystem, -0.9f, 0.9f, 100, 1.5f);

    // ---- Create the dynamic vertex buffer that we'll stream into each frame ----
    // max_vertices = 2000, matching the ParticleSystem capacity
    m_particleVB = GraphicsEngine::get()->createVertexBuffer();
    m_particleVB->loadDynamic(
        sizeof(ParticleVertex),
        12000,             // 2000 particles * 6 verts each (2 triangles per quad)
        m_vs_blob,
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

    // ---- Continuously drip new rain drops every frame ----
    // Emitting a small count per frame means as old drops die their slots
    // are immediately reused, keeping the rain density visually steady.
    ParticleEffects::rain(*m_particleSystem, -0.9f, 0.9f, 8, 1.5f);

    //ParticleEffects::fountain(*m_particleSystem, Vec3{ 0.0f, 0.0f, 0.0f }, 50, 1.0f);

    // ---- Build CPU-side render vertex array from alive particles ----
    // Each particle expands into a quad (2 triangles = 6 vertices).
    // Adjust half_size to control how big each particle appears on screen.
    const float half_size = 0.008f; // tweak this value to resize particles

    std::vector<ParticleVertex> verts;
    verts.reserve(m_particleSystem->particles().size() * 6);

    for (const Particle& p : m_particleSystem->particles())
    {
        if (!p.alive) continue;

        float cx = p.position.x;
        float cy = p.position.y;
        float cz = p.position.z;
        float r  = p.color.r * p.color.a;
        float g  = p.color.g * p.color.a;
        float b  = p.color.b * p.color.a;

        // 4 corners of the quad
        ParticleVertex tl = { cx - half_size, cy + half_size, cz, r, g, b }; // top-left
        ParticleVertex tr = { cx + half_size, cy + half_size, cz, r, g, b }; // top-right
        ParticleVertex bl = { cx - half_size, cy - half_size, cz, r, g, b }; // bottom-left
        ParticleVertex br = { cx + half_size, cy - half_size, cz, r, g, b }; // bottom-right

        // Triangle 1: top-left, top-right, bottom-left
        verts.push_back(tl);
        verts.push_back(tr);
        verts.push_back(bl);

        // Triangle 2: top-right, bottom-right, bottom-left
        verts.push_back(tr);
        verts.push_back(br);
        verts.push_back(bl);
    }

    // ---- Upload to GPU ----
    UINT alive_count = static_cast<UINT>(verts.size());
    if (alive_count > 0)
        m_particleVB->updateDynamic(verts.data(), alive_count);

    // ---- Render ----
    GraphicsEngine::get()->getImmediateDeviceContext()->clearRenderTargetColor(this->m_swap_chain, 0, 0.0f, 0.0f, 1);

    RECT rc = getClientWindowRect();
    GraphicsEngine::get()->getImmediateDeviceContext()->setViewportSize(
        rc.right - rc.left, rc.bottom - rc.top);

    GraphicsEngine::get()->getImmediateDeviceContext()->setVertexShader(m_vs);
    GraphicsEngine::get()->getImmediateDeviceContext()->setPixelShader(m_ps);

    if (alive_count > 0)
    {
        GraphicsEngine::get()->getImmediateDeviceContext()->setVertexBuffer(m_particleVB);
        GraphicsEngine::get()->getImmediateDeviceContext()->drawTriangleList(alive_count * 6, 0);
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