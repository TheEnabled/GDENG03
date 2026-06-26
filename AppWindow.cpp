#include "AppWindow.h"
#include "Vector3D.h"
#include "Matrix4x4.h"
#include "InputSystem.h"
#define NOMINMAX
#include <Windows.h>
#include <chrono>
#include <cstdlib>   // rand, srand

// ============================================================
//  RENDERING SYSTEM TEST CASES
//  Uncomment the active test case block, comment out the rest.
//
//  [DONE]   TEST CASE 1: Render a single cube with a rainbow pixel shader
//  [DONE]   TEST CASE 2: Render a single cube with a white shader + animated XYZ rotation
//  [DONE]   TEST CASE 3: Rainbow shader + XY movement + scale lerp 1.0 -> 0.25
//  [DONE]   TEST CASE 4: 50 random cubes each rotating along X/Y/Z
//  [DONE]   TEST CASE 5: Cube at center warps into a horizontal plane
//  [DONE]   TEST CASE 6: Reference scene — 3 cubes + ground plane (white shader)
//  [ACTIVE] TEST CASE 7: Stack of Cards (15 instances)
// ============================================================

AppWindow::AppWindow()
{
}

AppWindow::~AppWindow()
{
}

// -----------------------------------------------------------------------
//  GPU data structures (must match VertexShader.hlsl)
// -----------------------------------------------------------------------
struct vertex
{
    Vector3D position;
    Vector3D position1;
    Vector3D color;
};

__declspec(align(16))
struct constant
{
    Matrix4x4    m_world;
    Matrix4x4    m_view;
    Matrix4x4    m_proj;
    Vector3D     m_color;
    unsigned int m_time;
};

// -----------------------------------------------------------------------
//  onCreate — build cube, compile shaders, create constant buffer
// -----------------------------------------------------------------------
void AppWindow::onCreate()
{
    InputSystem::get()->addListener(this);

    GraphicsEngine::get()->init();

    m_swap_chain = GraphicsEngine::get()->createSwapChain();
    RECT rc = getClientWindowRect();
    m_swap_chain->init(this->m_hwnd, rc.right - rc.left, rc.bottom - rc.top);

    // ---- Vertex shader ----
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

    // ---- Cube geometry ----
    //
    //   7 --- 6
    //  /|    /|
    // 4 --- 5 |
    // | 3 --| 2
    // |/    |/
    // 0 --- 1
    //
    vertex cube_verts[] =
    {
        // position                  position1 (unused)    color
        { Vector3D(-0.5f,-0.5f,-0.5f), Vector3D(0,0,0), Vector3D(1.0f, 0.2f, 0.2f) }, // 0 front-bottom-left
        { Vector3D( 0.5f,-0.5f,-0.5f), Vector3D(0,0,0), Vector3D(0.2f, 1.0f, 0.2f) }, // 1 front-bottom-right
        { Vector3D( 0.5f,-0.5f, 0.5f), Vector3D(0,0,0), Vector3D(0.2f, 0.2f, 1.0f) }, // 2 back-bottom-right
        { Vector3D(-0.5f,-0.5f, 0.5f), Vector3D(0,0,0), Vector3D(1.0f, 1.0f, 0.2f) }, // 3 back-bottom-left
        { Vector3D(-0.5f, 0.5f,-0.5f), Vector3D(0,0,0), Vector3D(0.2f, 1.0f, 1.0f) }, // 4 front-top-left
        { Vector3D( 0.5f, 0.5f,-0.5f), Vector3D(0,0,0), Vector3D(1.0f, 0.2f, 1.0f) }, // 5 front-top-right
        { Vector3D( 0.5f, 0.5f, 0.5f), Vector3D(0,0,0), Vector3D(1.0f, 1.0f, 1.0f) }, // 6 back-top-right
        { Vector3D(-0.5f, 0.5f, 0.5f), Vector3D(0,0,0), Vector3D(0.5f, 0.5f, 0.5f) }, // 7 back-top-left
    };

    // 12 triangles × 3 indices = 36 (CW winding)
    unsigned int cube_indices[] =
    {
        // Front  (-Z)
        0, 5, 4,    0, 1, 5,
        // Back   (+Z)
        2, 7, 6,    2, 3, 7,
        // Left   (-X)
        3, 4, 7,    3, 0, 4,
        // Right  (+X)
        1, 6, 5,    1, 2, 6,
        // Bottom (-Y)
        3, 1, 0,    3, 2, 1,
        // Top    (+Y)
        4, 6, 7,    4, 5, 6,
    };

    m_cubeVB = GraphicsEngine::get()->createVertexBuffer();
    m_cubeVB->load(cube_verts, sizeof(vertex), ARRAYSIZE(cube_verts),
                   shader_byte_code, (UINT)size_shader);

    m_cubeIB = GraphicsEngine::get()->createIndexBuffer();
    m_cubeIB->load(cube_indices, ARRAYSIZE(cube_indices));

    GraphicsEngine::get()->releaseCompiledShaders();

    // ---- Pixel shader ----
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

    // ---- Constant buffer ----
    constant cc = {};
    cc.m_time = 0;
    m_cb = GraphicsEngine::get()->createConstantBuffer();
    m_cb->load(&cc, sizeof(constant));

    // ------------------------------------------------------------------
    //  TEST CASE 4: Initialise 50 random cube instances (fixed seed)
    // ------------------------------------------------------------------
    srand(42);
    for (int i = 0; i < CUBE_COUNT; i++)
    {
        m_cubes[i].position    = Vector3D(((rand() % 2001) - 1000) / 100.0f,   // -10..+10
                                          ((rand() % 1001) -  500) / 100.0f,   //  -5..+5
                                          ((rand() % 1801) +  200) / 100.0f);  //  +2..+20
        m_cubes[i].rot_speed_x = ((rand() % 401) - 200) / 100.0f;  // -2..+2 rad/s
        m_cubes[i].rot_speed_y = ((rand() % 401) - 200) / 100.0f;
        m_cubes[i].rot_speed_z = ((rand() % 401) - 200) / 100.0f;
        m_cubes[i].rot_x       = 0.0f;
        m_cubes[i].rot_y       = 0.0f;
        m_cubes[i].rot_z       = 0.0f;
        m_cubes[i].scale       = 0.3f + (rand() % 51) / 100.0f;    // 0.30..0.80
    }
}

// -----------------------------------------------------------------------
//  onUpdate — scale from held buttons, build transforms, draw cube
// -----------------------------------------------------------------------
void AppWindow::onUpdate()
{
    static auto last_frame_time = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    float dt = std::chrono::duration<float>(now - last_frame_time).count();

    if (dt < (1.0f / 60.0f))
    {
        Sleep(1);
        return;
    }
    last_frame_time = now;

    // Scale cube with held mouse buttons
    if (m_lmb_down)
    {
        m_cube_scale += 0.5f * dt;
        if (m_cube_scale > 5.0f) m_cube_scale = 5.0f;
    }
    if (m_rmb_down)
    {
        m_cube_scale -= 0.5f * dt;
        if (m_cube_scale < 0.1f) m_cube_scale = 0.1f;
    }

    // Clear
    GraphicsEngine::get()->getImmediateDeviceContext()
        ->clearRenderTargetColor(this->m_swap_chain, 0.1f, 0.1f, 0.15f, 1.0f);

    // Viewport
    RECT rc = getClientWindowRect();
    float width  = (float)(rc.right  - rc.left);
    float height = (float)(rc.bottom - rc.top);
    GraphicsEngine::get()->getImmediateDeviceContext()->setViewportSize((UINT)width, (UINT)height);

    // ---- View: Camera transform based on mouse rotation ----
    Matrix4x4 world_cam;
    world_cam.setIdentity();

    Matrix4x4 temp;
    temp.setIdentity();
    temp.setRotationX(m_rot_x);
    world_cam *= temp;

    temp.setIdentity();
    temp.setRotationY(m_rot_y);
    world_cam *= temp;

    // The first 3 rows of the rotation matrix represent the local X, Y, and Z axes
    Vector3D right(world_cam.m_mat[0][0], world_cam.m_mat[0][1], world_cam.m_mat[0][2]);
    Vector3D up   (world_cam.m_mat[1][0], world_cam.m_mat[1][1], world_cam.m_mat[1][2]);
    Vector3D forward(world_cam.m_mat[2][0], world_cam.m_mat[2][1], world_cam.m_mat[2][2]);

    float speed = 2.0f * dt;
    if (m_forward)  { m_cam_pos.m_x += forward.m_x * speed; m_cam_pos.m_y += forward.m_y * speed; m_cam_pos.m_z += forward.m_z * speed; }
    if (m_backward) { m_cam_pos.m_x -= forward.m_x * speed; m_cam_pos.m_y -= forward.m_y * speed; m_cam_pos.m_z -= forward.m_z * speed; }
    if (m_right)    { m_cam_pos.m_x += right.m_x * speed;   m_cam_pos.m_y += right.m_y * speed;   m_cam_pos.m_z += right.m_z * speed; }
    if (m_left)     { m_cam_pos.m_x -= right.m_x * speed;   m_cam_pos.m_y -= right.m_y * speed;   m_cam_pos.m_z -= right.m_z * speed; }
    if (m_up)       { m_cam_pos.m_x += up.m_x * speed;      m_cam_pos.m_y += up.m_y * speed;      m_cam_pos.m_z += up.m_z * speed; }
    if (m_down)     { m_cam_pos.m_x -= up.m_x * speed;      m_cam_pos.m_y -= up.m_y * speed;      m_cam_pos.m_z -= up.m_z * speed; }

    world_cam.setTranslation(m_cam_pos);

    Matrix4x4 m_view;
    m_view.setInverse(world_cam);

    // ---- Projection: perspective 60 deg FOV ----
    Matrix4x4 m_proj;
    float fov    = 3.14159265f / 3.0f;
    float aspect = (height > 0.0f) ? (width / height) : 1.0f;
    m_proj.setPerspectiveFovLH(fov, aspect, 0.1f, 100.0f);

    // ---- Bind shaders + geometry buffers (shared by all cases) ----
    auto* ctx = GraphicsEngine::get()->getImmediateDeviceContext();
    ctx->setVertexShader(m_vs);
    ctx->setPixelShader(m_ps);
    ctx->setVertexBuffer(m_cubeVB);
    ctx->setIndexBuffer(m_cubeIB);
    ctx->setConstantBuffer(m_vs, m_cb);
    ctx->setConstantBuffer(m_ps, m_cb);

    // ====================================================================
    //  CASE BLOCKS — each block is self-contained: world matrix + draw.
    //  Uncomment the active case, comment out the rest.
    // ====================================================================

    // ------------------------------------------------------------------
    //  TEST CASE 1: Scale only, no rotation | rainbow shader
    // ------------------------------------------------------------------
    /*
    {
        Matrix4x4 sc, m_world;
        sc.setIdentity();
        sc.m_mat[0][0] = m_cube_scale;
        sc.m_mat[1][1] = m_cube_scale;
        sc.m_mat[2][2] = m_cube_scale;
        m_world.setIdentity();
        m_world *= sc;

        constant cc = {};
        cc.m_time  = (unsigned int)::GetTickCount64();
        cc.m_world = m_world; cc.m_view = m_view; cc.m_proj = m_proj;
        cc.m_color = Vector3D(1, 1, 1);
        m_cb->update(ctx, &cc);
        ctx->drawIndexedTriangleList(m_cubeIB->getSizeIndexList(), 0, 0);
    }
    */

    // ------------------------------------------------------------------
    //  TEST CASE 2: Animated XYZ rotation | white shader
    // ------------------------------------------------------------------
    /*
    {
        m_anim_rot_x += 1.0f * dt;
        m_anim_rot_y += 1.5f * dt;
        m_anim_rot_z += 0.5f * dt;

        Matrix4x4 sc, rx, ry, rz, m_world;
        sc.setIdentity();
        sc.m_mat[0][0] = m_cube_scale;
        sc.m_mat[1][1] = m_cube_scale;
        sc.m_mat[2][2] = m_cube_scale;
        rx.setRotationX(m_anim_rot_x);
        ry.setRotationY(m_anim_rot_y);
        rz.setRotationZ(m_anim_rot_z);
        m_world.setIdentity();
        m_world *= sc; m_world *= rx; m_world *= ry; m_world *= rz;

        constant cc = {};
        cc.m_time  = (unsigned int)::GetTickCount64();
        cc.m_world = m_world; cc.m_view = m_view; cc.m_proj = m_proj;
        cc.m_color = Vector3D(1, 1, 1);
        m_cb->update(ctx, &cc);
        ctx->drawIndexedTriangleList(m_cubeIB->getSizeIndexList(), 0, 0);
    }
    */

    // ------------------------------------------------------------------
    //  TEST CASE 3: Rainbow shader + XY movement + scale lerp 1.0->0.25
    // ------------------------------------------------------------------
    /*
    {
        m_anim_time += dt;
        float t_scale    = sinf(m_anim_time * 0.5f) * 0.5f + 0.5f;
        float anim_scale = 1.0f + (0.25f - 1.0f) * t_scale;
        float anim_x     = sinf(m_anim_time * 1.0f)  * 1.5f;
        float anim_y     = sinf(m_anim_time * 0.75f) * 1.0f;

        Matrix4x4 sc, tr, m_world;
        sc.setIdentity();
        sc.m_mat[0][0] = anim_scale;
        sc.m_mat[1][1] = anim_scale;
        sc.m_mat[2][2] = anim_scale;
        tr.setIdentity();
        tr.setTranslation(Vector3D(anim_x, anim_y, 0.0f));
        m_world.setIdentity();
        m_world *= sc;
        m_world *= tr;

        constant cc = {};
        cc.m_time  = (unsigned int)::GetTickCount64();
        cc.m_world = m_world; cc.m_view = m_view; cc.m_proj = m_proj;
        cc.m_color = Vector3D(1, 1, 1);
        m_cb->update(ctx, &cc);
        ctx->drawIndexedTriangleList(m_cubeIB->getSizeIndexList(), 0, 0);
    }
    */

    // ------------------------------------------------------------------
    //  TEST CASE 4: 50 random cubes each rotating along X/Y/Z | rainbow
    // ------------------------------------------------------------------
    /*
    {
        for (int i = 0; i < CUBE_COUNT; i++)
        {
            m_cubes[i].rot_x += m_cubes[i].rot_speed_x * dt;
            m_cubes[i].rot_y += m_cubes[i].rot_speed_y * dt;
            m_cubes[i].rot_z += m_cubes[i].rot_speed_z * dt;

            Matrix4x4 sc, rx, ry, rz, tr, world;
            sc.setIdentity();
            sc.m_mat[0][0] = m_cubes[i].scale;
            sc.m_mat[1][1] = m_cubes[i].scale;
            sc.m_mat[2][2] = m_cubes[i].scale;
            rx.setRotationX(m_cubes[i].rot_x);
            ry.setRotationY(m_cubes[i].rot_y);
            rz.setRotationZ(m_cubes[i].rot_z);
            tr.setIdentity();
            tr.setTranslation(m_cubes[i].position);
            world.setIdentity();
            world *= sc; world *= rx; world *= ry; world *= rz; world *= tr;

            constant cc = {};
            cc.m_time  = (unsigned int)::GetTickCount64();
            cc.m_world = world; cc.m_view = m_view; cc.m_proj = m_proj;
            cc.m_color = Vector3D(1, 1, 1);
            m_cb->update(ctx, &cc);
            ctx->drawIndexedTriangleList(m_cubeIB->getSizeIndexList(), 0, 0);
        }
    }
    */

    // ------------------------------------------------------------------
    //  TEST CASE 5: Cube warps into a horizontal plane | rainbow shader
    // ------------------------------------------------------------------
    /*
    {
        m_anim_time += dt;
        float t    = sinf(m_anim_time * 0.4f) * 0.5f + 0.5f;
        float warp = t * t * (3.0f - 2.0f * t);
        float sx   = 1.0f + (3.0f - 1.0f)  * warp;
        float sy   = 1.0f + (0.05f - 1.0f) * warp;
        float sz   = 1.0f + (3.0f - 1.0f)  * warp;
        Matrix4x4 sc, m_world;
        sc.setIdentity();
        sc.m_mat[0][0] = sx; sc.m_mat[1][1] = sy; sc.m_mat[2][2] = sz;
        m_world.setIdentity();
        m_world *= sc;
        constant cc = {};
        cc.m_time  = (unsigned int)::GetTickCount64();
        cc.m_world = m_world; cc.m_view = m_view; cc.m_proj = m_proj;
        cc.m_color = Vector3D(1, 1, 1);
        m_cb->update(ctx, &cc);
        ctx->drawIndexedTriangleList(m_cubeIB->getSizeIndexList(), 0, 0);
    }
    */

    // ------------------------------------------------------------------
    //  TEST CASE 6: Reference scene — ground plane + 3 cubes | white shader
    // ------------------------------------------------------------------
    /*
    {
        // ---- Fixed camera override REMOVED ----
        // You can now move the camera freely using your WASD and mouse!
        // Start position is your current camera position.

        // Helper lambda: build world matrix from scale + translation, then draw
        auto draw_obj = [&](float sx, float sy, float sz, float px, float py, float pz)
        {
            Matrix4x4 sc, tr, world;
            sc.setIdentity();
            sc.m_mat[0][0] = sx;  sc.m_mat[1][1] = sy;  sc.m_mat[2][2] = sz;
            tr.setIdentity();
            tr.setTranslation(Vector3D(px, py, pz));
            world.setIdentity();
            world *= sc;
            world *= tr;
            constant cc = {};
            cc.m_time  = (unsigned int)::GetTickCount64();
            cc.m_world = world;  cc.m_view = m_view;  cc.m_proj = m_proj;
            cc.m_color = Vector3D(1, 1, 1);
            m_cb->update(ctx, &cc);
            ctx->drawIndexedTriangleList(m_cubeIB->getSizeIndexList(), 0, 0);
        };

        draw_obj(15.0f, 0.1f, 15.0f,   0.0f, -0.05f, 3.0f); // ground
        draw_obj(0.8f, 0.8f, 0.8f,    0.0f,  0.9f,  0.0f);  // cube 1
        draw_obj(0.8f, 0.8f, 0.8f,   -1.5f,  2.0f,  0.0f);  // cube 2
        draw_obj(0.8f, 0.8f, 0.8f,   -1.5f,  3.0f, -2.0f);  // cube 3
    }
    */

    // ------------------------------------------------------------------
    //  TEST CASE 7: Stack of Cards (15 cards) | white shader
    //  To activate: uncomment this block, comment out the active case
    // ------------------------------------------------------------------
    {
        // Helper lambda to draw a flattened cube as a card
        auto draw_card = [&](float px, float py, float pz, float rz, float sx, float sy, float sz)
        {
            Matrix4x4 sc, rmat, tr, world;
            sc.setIdentity();
            sc.m_mat[0][0] = sx;  sc.m_mat[1][1] = sy;  sc.m_mat[2][2] = sz;
            rmat.setRotationZ(rz);
            tr.setIdentity();
            tr.setTranslation(Vector3D(px, py, pz));
            
            world.setIdentity();
            world *= sc;
            world *= rmat;
            world *= tr;
            
            constant cc = {};
            cc.m_time  = (unsigned int)::GetTickCount64();
            cc.m_world = world;  cc.m_view = m_view;  cc.m_proj = m_proj;
            cc.m_color = Vector3D(1, 1, 1);
            m_cb->update(ctx, &cc);
            ctx->drawIndexedTriangleList(m_cubeIB->getSizeIndexList(), 0, 0);
        };

        // Dimensions of a single card
        float w = 2.0f, h = 3.0f, d = 0.05f;
        float angle = 0.45f; // ~25 degrees leaning
        
        // Offset from center of card to its peak (when leaning)
        float dx = (h / 2.0f) * sinf(angle); // 1.5 * 0.435 = ~0.65
        float dy = (h / 2.0f) * cosf(angle); // 1.5 * 0.900 = ~1.35

        // Spacing between adjacent triangles (base to base)
        float cx = 2.7f; 
        
        // TIER 1 (Bottom, 3 triangles = 6 cards)
        float y1 = dy; 
        draw_card(-cx - dx, y1, 0.0f, -angle, d, h, w);
        draw_card(-cx + dx, y1, 0.0f,  angle, d, h, w);
        
        draw_card(    - dx, y1, 0.0f, -angle, d, h, w);
        draw_card(      dx, y1, 0.0f,  angle, d, h, w);
        
        draw_card( cx - dx, y1, 0.0f, -angle, d, h, w);
        draw_card( cx + dx, y1, 0.0f,  angle, d, h, w);

        // HORIZONTAL CARDS on TIER 1 (2 cards)
        float h1 = y1 + dy + (d / 2.0f); // resting flush on the peaks
        draw_card(-cx / 2.0f, h1, 0.0f, 0.0f, 3.0f, d, 2.0f);
        draw_card( cx / 2.0f, h1, 0.0f, 0.0f, 3.0f, d, 2.0f);

        // TIER 2 (Middle, 2 triangles = 4 cards)
        float y2 = h1 + (d / 2.0f) + dy;
        draw_card(-cx / 2.0f - dx, y2, 0.0f, -angle, d, h, w);
        draw_card(-cx / 2.0f + dx, y2, 0.0f,  angle, d, h, w);

        draw_card( cx / 2.0f - dx, y2, 0.0f, -angle, d, h, w);
        draw_card( cx / 2.0f + dx, y2, 0.0f,  angle, d, h, w);

        // HORIZONTAL CARDS on TIER 2 (1 card)
        float h2 = y2 + dy + (d / 2.0f);
        draw_card(0.0f, h2, 0.0f, 0.0f, 3.0f, d, 2.0f);

        // TIER 3 (Top, 1 triangle = 2 cards)
        float y3 = h2 + (d / 2.0f) + dy;
        draw_card(-dx, y3, 0.0f, -angle, d, h, w);
        draw_card( dx, y3, 0.0f,  angle, d, h, w);
    }

    m_swap_chain->present(false);

    m_old_delta = m_new_delta;
    m_new_delta = (float)::GetTickCount64();
    m_delta_time = (m_old_delta) ? ((m_new_delta - m_old_delta) / 1000.0f) : 0.0f;
}

// -----------------------------------------------------------------------
//  onDestroy
// -----------------------------------------------------------------------
void AppWindow::onDestroy()
{
    Window::onDestroy();

    if (m_cubeVB)    { m_cubeVB->release();     m_cubeVB    = nullptr; }
    if (m_cubeIB)    { m_cubeIB->release();     m_cubeIB    = nullptr; }
    if (m_cb)        { m_cb->release();         m_cb        = nullptr; }
    if (m_vs)        { m_vs->release();         m_vs        = nullptr; }
    if (m_ps)        { m_ps->release();         m_ps        = nullptr; }
    if (m_swap_chain){ m_swap_chain->release(); m_swap_chain = nullptr; }

    GraphicsEngine::get()->release();
}

// -----------------------------------------------------------------------
//  Focus
// -----------------------------------------------------------------------
void AppWindow::onFocus()
{
    InputSystem::get()->addListener(this);
}

void AppWindow::onKillFocus()
{
    InputSystem::get()->removeListener(this);
}

// -----------------------------------------------------------------------
//  Keyboard
// -----------------------------------------------------------------------
void AppWindow::onKeyDown(int key)
{
    if (key == 'W') m_forward = true;
    if (key == 'S') m_backward = true;
    if (key == 'A') m_left = true;
    if (key == 'D') m_right = true;
    if (key == 'Q') m_down = true;
    if (key == 'E') m_up = true;
}

void AppWindow::onKeyUp(int key)
{
    if (key == 'W') m_forward = false;
    if (key == 'S') m_backward = false;
    if (key == 'A') m_left = false;
    if (key == 'D') m_right = false;
    if (key == 'Q') m_down = false;
    if (key == 'E') m_up = false;

    if (key == VK_ESCAPE)
        m_is_run = false;
}

// -----------------------------------------------------------------------
//  Mouse
// -----------------------------------------------------------------------
void AppWindow::onMouseMove(const Point& delta_mouse_pos)
{
    // Accumulate rotation: horizontal delta → Y-axis, vertical delta → X-axis
    const float sensitivity = 0.003f;
    m_rot_y += delta_mouse_pos.m_x * sensitivity;
    m_rot_x += delta_mouse_pos.m_y * sensitivity;
}

void AppWindow::onLeftMouseDown(const Point& mouse_pos)
{
    m_lmb_down = true;
}

void AppWindow::onLeftMouseUp(const Point& mouse_pos)
{
    m_lmb_down = false;
}

void AppWindow::onRightMouseDown(const Point& mouse_pos)
{
    m_rmb_down = true;
}

void AppWindow::onRightMouseUp(const Point& mouse_pos)
{
    m_rmb_down = false;
}
