struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 color : COLOR;
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

float4 psmain (PS_INPUT input) :  SV_TARGET
{
    return float4(m_color, 1.0f);
}