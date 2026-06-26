#pragma once
#include "Window.h"
#include "GraphicsEngine.h"
#include "SwapChain.h"
#include "DeviceContext.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ConstantBuffer.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "InputListener.h"
#include "Vector3D.h"

class AppWindow : public Window, public InputListener
{
public:
    AppWindow();
    ~AppWindow();

    virtual void onCreate()     override;
    virtual void onUpdate()     override;
    virtual void onDestroy()    override;
    virtual void onFocus()      override;
    virtual void onKillFocus()  override;

    // Keyboard
    void onKeyDown(int key) override;
    void onKeyUp(int key)   override;

    // Mouse
    virtual void onMouseMove(const Point& delta_mouse_pos)  override;
    virtual void onLeftMouseDown(const Point& mouse_pos)    override;
    virtual void onLeftMouseUp(const Point& mouse_pos)      override;
    virtual void onRightMouseDown(const Point& mouse_pos)   override;
    virtual void onRightMouseUp(const Point& mouse_pos)     override;

private:
    SwapChain*      m_swap_chain  = nullptr;
    VertexBuffer*   m_cubeVB      = nullptr;
    IndexBuffer*    m_cubeIB      = nullptr;
    VertexShader*   m_vs          = nullptr;
    PixelShader*    m_ps          = nullptr;
    ConstantBuffer* m_cb          = nullptr;

private:
    float m_old_delta  = 0;
    float m_new_delta  = 0;
    float m_delta_time = 0;

    // Cube transform state
    float m_rot_x      = 0.0f;
    float m_rot_y      = 0.0f;
    float m_cube_scale = 1.0f;

    // Case 2: continuous animated rotation accumulators
    float m_anim_rot_x = 0.0f;
    float m_anim_rot_y = 0.0f;
    float m_anim_rot_z = 0.0f;

    // Case 3: general animation time accumulator (seconds)
    float m_anim_time  = 0.0f;

    // Case 4: 50 independent cube instances
    struct CubeInstance
    {
        Vector3D position;
        float    rot_speed_x = 0.0f, rot_speed_y = 0.0f, rot_speed_z = 0.0f;
        float    rot_x       = 0.0f, rot_y       = 0.0f, rot_z       = 0.0f;
        float    scale       = 0.5f;
    };
    static const int CUBE_COUNT = 50;
    CubeInstance     m_cubes[CUBE_COUNT] = {};

    // Mouse button held state
    bool m_lmb_down = false;
    bool m_rmb_down = false;

    // Camera transform and movement state
    Vector3D m_cam_pos = Vector3D(0.0f, 0.0f, -2.0f);
    bool m_forward  = false;
    bool m_backward = false;
    bool m_right    = false;
    bool m_left     = false;
    bool m_up       = false;
    bool m_down     = false;
};