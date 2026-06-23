
struct VS_INPUT
{
    float3 position: POSITION;
    float3 position1: POSITION1;
    float3 color: COLOR;
};

struct VS_OUTPUT
{
    float4 position: SV_POSITION;
    float3 color: COLOR;
    float3 local_pos: TEXCOORD0;
};

cbuffer constant : register(b0)
{
    row_major float4x4 m_world;
    row_major float4x4 m_view;
    row_major float4x4 m_proj;
    
    float3 m_color;
    unsigned int m_time;
}

VS_OUTPUT vsmain(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    //World Space
    output.position = mul(float4(input.position, 1.0f), m_world);
    //View Space
    output.position = mul(output.position, m_view);
    //Screen Space
    output.position = mul(output.position, m_proj);
    
    output.color = input.color;
    output.local_pos = input.position;
    return output;
}




