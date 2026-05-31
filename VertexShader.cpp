#include "VertexShader.h"
#include "GraphicsEngine.h"

VertexShader::VertexShader()
	: m_vs(nullptr) // ADD: initialize to avoid undefined behaviour
{
}

bool VertexShader::init(const void* shader_byte_code, size_t byte_code_size)
{
	if (FAILED(GraphicsEngine::get()->m_d3d_device->CreateVertexShader(
		shader_byte_code, byte_code_size, nullptr, &m_vs)))
		return false;

	return true;
}

bool VertexShader::release()
{
	if (m_vs) m_vs->Release();
	delete this;

	return true; // FIX: was missing — function declared bool but returned nothing
}

VertexShader::~VertexShader()
{
}