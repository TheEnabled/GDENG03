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

// ------------------------------------------------------------------
//  TEST CASE 1, 3, 4 & 5: Rainbow pixel shader
//  Used by: Case 1 (static), Case 3 (XY move+scale), Case 4 (50 cubes), Case 5 (warp)
//  To activate: uncomment this block, comment out Case 2/6 below
// ------------------------------------------------------------------
/*
float3 hsv2rgb(float h, float s, float v)
{
    float3 rgb;
    float c  = v * s;
    float h6 = h * 6.0f;
    float x  = c * (1.0f - abs(fmod(h6, 2.0f) - 1.0f));
    if      (h6 < 1.0f) rgb = float3(c, x, 0);
    else if (h6 < 2.0f) rgb = float3(x, c, 0);
    else if (h6 < 3.0f) rgb = float3(0, c, x);
    else if (h6 < 4.0f) rgb = float3(0, x, c);
    else if (h6 < 5.0f) rgb = float3(x, 0, c);
    else                rgb = float3(c, 0, x);
    float m = v - c;
    return rgb + m;
}

float4 psmain(PS_INPUT input) : SV_TARGET
{
    float time_sec = (float)m_time * 0.001f;
    float hue = frac(time_sec * 0.25f + (input.local_pos.x + input.local_pos.y + input.local_pos.z) * 0.5f + 0.5f);
    float3 rainbow = hsv2rgb(hue, 1.0f, 1.0f);

    // Reconstruct face normal via screen-space derivatives for lighting
    float3 dx      = ddx(input.local_pos);
    float3 dy      = ddy(input.local_pos);
    float3 normal  = normalize(cross(dx, dy));
    float3 light_dir = normalize(float3(1.0f, 1.5f, -1.0f));
    float  diffuse   = saturate(dot(normal, light_dir));
    float  ambient   = 0.25f;
    float  lighting  = ambient + (1.0f - ambient) * diffuse;

    return float4(rainbow * lighting, 1.0f);
}
*/

// ------------------------------------------------------------------
//  TEST CASE 2 & 6: White pixel shader
//  Used by: Case 2 (single rotating white cube), Case 6 (reference scene)
//  To activate: uncomment this block, comment out Case 1/3/4/5 above
// ------------------------------------------------------------------
float4 psmain(PS_INPUT input) : SV_TARGET
{
    // Reconstruct face normal via screen-space derivatives for lighting
    float3 dx      = ddx(input.local_pos);
    float3 dy      = ddy(input.local_pos);
    float3 normal  = normalize(cross(dx, dy));
    
    // Directional light pointing down-left-forward
    float3 light_dir = normalize(float3(1.0f, 1.5f, -1.0f));
    float  diffuse   = saturate(dot(normal, light_dir));
    
    // Ambient light so shadows aren't pitch black
    float  ambient   = 0.35f;
    float  lighting  = ambient + (1.0f - ambient) * diffuse;

    // Apply lighting to solid white
    return float4(lighting, lighting, lighting, 1.0f);
}