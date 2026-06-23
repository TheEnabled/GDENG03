#include "AppWindow.h"
#include "Vector3D.h"
#include "Matrix4x4.h"
#include "InputSystem.h"
#include <Windows.h>
#include "Quad.h"
#include <chrono>

AppWindow::AppWindow()
    : m_swap_chain(nullptr)
    , m_circleVB(nullptr)
    , m_circleIB(nullptr)
    , m_vs(nullptr)
    , m_ps(nullptr)
    , m_cb(nullptr)
    , m_old_delta(0)
    , m_new_delta(0)
    , m_delta_time(0)
    , m_delta_pos(0)
{
}

struct vec3
{
	float x, y, z;
};

struct vertex
{
    Vector3D position;
    Vector3D position1;
    Vector3D color;
};

__declspec(align(16))
struct constant
{
	Matrix4x4 m_world;
    Matrix4x4 m_view;
    Matrix4x4 m_proj;
    Vector3D m_color;
    unsigned int m_time;
};

void AppWindow::updateCircles(float dt)
{
    RECT rc = getClientWindowRect();
    float width = (rc.right - rc.left) / 100.0f;
    float height = (rc.bottom - rc.top) / 100.0f;
    float halfWidth = width / 2.0f;
    float halfHeight = height / 2.0f;

    for (CircleData& c : m_circles)
    {
        c.position.m_x += c.velocity.m_x * dt;
        c.position.m_y += c.velocity.m_y * dt;

        // Bounce horizontally
        if (c.position.m_x + c.radius > halfWidth)
        {
            c.position.m_x = halfWidth - c.radius;
            c.velocity.m_x = -c.velocity.m_x;
        }
        else if (c.position.m_x - c.radius < -halfWidth)
        {
            c.position.m_x = -halfWidth + c.radius;
            c.velocity.m_x = -c.velocity.m_x;
        }

        // Bounce vertically
        if (c.position.m_y + c.radius > halfHeight)
        {
            c.position.m_y = halfHeight - c.radius;
            c.velocity.m_y = -c.velocity.m_y;
        }
        else if (c.position.m_y - c.radius < -halfHeight)
        {
            c.position.m_y = -halfHeight + c.radius;
            c.velocity.m_y = -c.velocity.m_y;
        }
    }
}

AppWindow::~AppWindow()
{
}

void AppWindow::onCreate()
{
	InputSystem::get()->addListener(this);

    GraphicsEngine::get()->init();

    m_swap_chain = GraphicsEngine::get()->createSwapChain();
    RECT rc = getClientWindowRect();
    m_swap_chain->init(this->m_hwnd, rc.right - rc.left, rc.bottom - rc.top);

    
    void*  shader_byte_code = nullptr;
    size_t size_shader      = 0;

    if (!GraphicsEngine::get()->compileVertexShaders(L"VertexShader.hlsl", "vsmain",
                                                    &shader_byte_code, &size_shader))
    {
        MessageBoxA(m_hwnd, "Failed to compile VertexShader.hlsl", "Shader Error", MB_OK | MB_ICONERROR);
        return;
    }
    m_vs = GraphicsEngine::get()->createVertexShader(shader_byte_code, size_shader);
    if (!m_vs)
    {
        MessageBoxA(m_hwnd, "Failed to create vertex shader", "Shader Error", MB_OK | MB_ICONERROR);
        return;
    }

    const int numSegments = 32;
    std::vector<vertex> circleVertices;
    std::vector<unsigned int> circleIndices;

    // Center vertex
    circleVertices.push_back({ Vector3D(0, 0, 0), Vector3D(0, 0, 0), Vector3D(1, 1, 1) });

    // Rim vertices
    for (int i = 0; i < numSegments; ++i)
    {
        float theta = (2.0f * 3.1415926f * i) / numSegments;
        float x = cos(theta);
        float y = sin(theta);
        circleVertices.push_back({ Vector3D(x, y, 0), Vector3D(x, y, 0), Vector3D(1, 1, 1) });
    }

    // Indices for triangles (CLOCKWISE WINDING ORDER)
    for (int i = 1; i <= numSegments; ++i)
    {
        circleIndices.push_back(0); // Center
        if (i == numSegments)
            circleIndices.push_back(1); // Wrap around
        else
            circleIndices.push_back(i + 1);
        circleIndices.push_back(i);
    }

    m_circleVB = GraphicsEngine::get()->createVertexBuffer();
    m_circleVB->load(circleVertices.data(), sizeof(vertex), (UINT)circleVertices.size(), shader_byte_code, (UINT)size_shader);

    m_circleIB = GraphicsEngine::get()->createIndexBuffer();
    m_circleIB->load(circleIndices.data(), (UINT)circleIndices.size());

    GraphicsEngine::get()->releaseCompiledShaders();

    if (!GraphicsEngine::get()->compilePixelShaders(L"PixelShader.hlsl", "psmain",
                                               &shader_byte_code, &size_shader))
    {
        MessageBoxA(m_hwnd, "Failed to compile PixelShader.hlsl", "Shader Error", MB_OK | MB_ICONERROR);
        return;
    }
    m_ps = GraphicsEngine::get()->createPixelShader(shader_byte_code, size_shader);
    if (!m_ps)
    {
        MessageBoxA(m_hwnd, "Failed to create pixel shader", "Shader Error", MB_OK | MB_ICONERROR);
        return;
    }
    GraphicsEngine::get()->releaseCompiledShaders();

    constant cc = {};
	cc.m_time = 0;

    m_cb = GraphicsEngine::get()->createConstantBuffer();
    m_cb->load(&cc, sizeof(constant));
}

void AppWindow::onUpdate()
{
    static auto last_frame_time = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    float dt = std::chrono::duration<float>(now - last_frame_time).count();
    
    // Manual 60 FPS cap (16.66ms per frame)
    if (dt < (1.0f / 60.0f))
    {
        Sleep(1);
        return; 
    }
    last_frame_time = now;

    // Clear to a solid black background
    GraphicsEngine::get()->getImmediateDeviceContext()
        ->clearRenderTargetColor(this->m_swap_chain, 0, 0, 0, 1);

    // Resize viewport to match the current window size
    RECT rc = getClientWindowRect();
    GraphicsEngine::get()->getImmediateDeviceContext()
        ->setViewportSize(rc.right - rc.left, rc.bottom - rc.top);

	updateCircles(dt);

    // Bind shaders
    GraphicsEngine::get()->getImmediateDeviceContext()->setVertexShader(m_vs);
    GraphicsEngine::get()->getImmediateDeviceContext()->setPixelShader(m_ps);

    // Draw circles
    if (m_circleVB->getSizeVertexList() > 0 && !m_circles.empty())
    {
        GraphicsEngine::get()->getImmediateDeviceContext()->setVertexBuffer(m_circleVB);
        GraphicsEngine::get()->getImmediateDeviceContext()->setIndexBuffer(m_circleIB);

        for (const CircleData& c : m_circles)
        {
            constant cc = {};
            cc.m_time = (unsigned int)::GetTickCount64();
            
            Matrix4x4 m_scale, m_trans;
            m_scale.setIdentity();
            m_scale.m_mat[0][0] = c.radius;
            m_scale.m_mat[1][1] = c.radius;
            m_scale.m_mat[2][2] = c.radius;
            
            m_trans.setTranslation(c.position);
            
            cc.m_world.setIdentity();
            cc.m_world *= m_scale;
            cc.m_world *= m_trans;

            cc.m_view.setIdentity();
            
            float width = (rc.right - rc.left) / 100.0f;
            float height = (rc.bottom - rc.top) / 100.0f;
            cc.m_proj.setOrthoLH(width, height, -4.0f, 4.0f);

            cc.m_color = c.color;

            m_cb->update(GraphicsEngine::get()->getImmediateDeviceContext(), &cc);

            GraphicsEngine::get()->getImmediateDeviceContext()->setConstantBuffer(m_vs, m_cb);
            GraphicsEngine::get()->getImmediateDeviceContext()->setConstantBuffer(m_ps, m_cb);

            GraphicsEngine::get()->getImmediateDeviceContext()
                ->drawIndexedTriangleList(m_circleIB->getSizeIndexList(), 0, 0);
        }
    }

    m_swap_chain->present(false);

	m_old_delta = m_new_delta;
	m_new_delta = (float)::GetTickCount64();

    m_delta_time = (m_old_delta) ? ((m_new_delta - m_old_delta) / 1000.0f) : 0.0f;
}

void AppWindow::onDestroy()
{
    Window::onDestroy();

    if (m_circleVB)    { m_circleVB->release();     m_circleVB    = nullptr; }
    if (m_circleIB)    { m_circleIB->release();     m_circleIB    = nullptr; }
    if (m_cb)        { m_cb->release();        m_cb        = nullptr; }
    if (m_vs)        { m_vs->release();         m_vs        = nullptr; }
    if (m_ps)        { m_ps->release();         m_ps        = nullptr; }
    if (m_swap_chain){ m_swap_chain->release(); m_swap_chain = nullptr; }

    GraphicsEngine::get()->release();
}

void AppWindow::onKeyDown(int key)
{
}

void AppWindow::onKeyUp(int key)
{
    if (key == VK_ESCAPE)
    {
        OutputDebugStringA("DEBUG: ESCAPE key pressed.\n");
        m_is_run = false;
    }
    else if (key == VK_SPACE)
    {
        OutputDebugStringA("DEBUG: SPACEBAR key pressed. Spawning new circle.\n");
        CircleData c;
        c.position = Vector3D(0, 0, 0); 
        
        // Random direction angle and random speed between 2.0 and 5.0
        float angle = ((float)rand() / RAND_MAX) * 2.0f * 3.14159f;
        float speed = 2.0f + ((float)rand() / RAND_MAX) * 3.0f;
        c.velocity = Vector3D(cos(angle) * speed, sin(angle) * speed, 0);
        
        c.radius = 0.3f; 
        
        // Random color
        c.color = Vector3D(
            0.2f + 0.8f * ((float)rand() / RAND_MAX),
            0.2f + 0.8f * ((float)rand() / RAND_MAX),
            0.2f + 0.8f * ((float)rand() / RAND_MAX)
        );

        m_circles.push_back(c);
    }
    else if (key == VK_BACK)
    {
        OutputDebugStringA("DEBUG: BACKSPACE key pressed. Removing last circle.\n");
        if (!m_circles.empty())
        {
            m_circles.pop_back();
        }
    }
    else if (key == VK_DELETE)
    {
        OutputDebugStringA("DEBUG: DELETE key pressed. Clearing all circles.\n");
        m_circles.clear();
    }
}
