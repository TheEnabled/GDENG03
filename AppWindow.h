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
#include "AGameObject.h"
#include "ParticleEffects.h"
#include "InputListener.h"
#include "Vector3D.h"

#include <vector>

struct CircleData
{
    Vector3D position;
    Vector3D velocity;
    Vector3D color;
    float radius = 0.0f;
};

class AppWindow : public Window, public InputListener
{
public:
    AppWindow();

    void updateCircles(float dt);

    ~AppWindow();

    virtual void onCreate()  override;
    virtual void onUpdate()  override;
    virtual void onDestroy() override;

    // Inherited via InputListener
    void onKeyDown(int key) override;
    void onKeyUp(int key) override;

private:
    SwapChain*    m_swap_chain;
    VertexBuffer* m_circleVB;
    IndexBuffer*  m_circleIB;
    VertexShader* m_vs;
    PixelShader*  m_ps;
	ConstantBuffer* m_cb;         
private:
    float m_old_delta;
    float m_new_delta;
    float m_delta_time;

    float m_delta_pos;

    std::vector<CircleData> m_circles;
};