#include "PixelShader.h"
#include "GraphicsEngine.h"

PixelShader::PixelShader()
	: m_ps(nullptr) // ADD: initialize to avoid undefined behaviour
{
}

bool PixelShader::init(const void* shader_byte_code, size_t byte_code_size)
{
	if (FAILED(GraphicsEngine::get()->m_d3d_device->CreatePixelShader(
		shader_byte_code, byte_code_size, nullptr, &m_ps)))
		return false;

	return true;
}

bool PixelShader::release()
{
	if (m_ps) m_ps->Release();
	delete this;

	return true; // FIX: was missing — function declared bool but returned nothing
}

PixelShader::~PixelShader()
{
	
}
