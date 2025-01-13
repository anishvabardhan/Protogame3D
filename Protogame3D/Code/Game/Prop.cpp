#include "Game/Prop.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"

#include "Game/GameCommon.hpp"

#include <math.h>

Prop::Prop()
{
}

Prop::Prop(Game* owner, Vec3 position, Rgba8 modelColor)
	: Entity(owner)
{
	m_position = position;
	m_color = modelColor;

	m_gpuMesh = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PCU), std::wstring(L"Prop"));
	m_modelCBO = g_theRenderer->CreateConstantBuffer(sizeof(ModelConstants), std::wstring(L"Prop Model"));

	//+X
	AddVertsForQuad3D(m_vertices, Vec3(0.5f, -0.5f, -0.5f), Vec3(0.5f, 0.5f, -0.5f), Vec3(0.5f, 0.5f, 0.5f), Vec3(0.5f, -0.5f, 0.5f), Rgba8(255, 0, 0, 255), Rgba8(255, 0, 0, 255));

	//-X
	AddVertsForQuad3D(m_vertices, Vec3(-0.5f, 0.5f, -0.5f), Vec3(-0.5f, -0.5f, -0.5f), Vec3(-0.5f, -0.5f, 0.5f), Vec3(-0.5f, 0.5f, 0.5f), Rgba8(0, 255, 255, 255), Rgba8(0, 255, 255, 255));

	//+Y
	AddVertsForQuad3D(m_vertices, Vec3(0.5f, 0.5f, -0.5f), Vec3(-0.5f, 0.5f, -0.5f), Vec3(-0.5f, 0.5f, 0.5f), Vec3(0.5f, 0.5f, 0.5f), Rgba8(0, 255, 0, 255), Rgba8(0, 255, 0, 255));

	//-Y
	AddVertsForQuad3D(m_vertices, Vec3(-0.5f, -0.5f, -0.5f), Vec3(0.5f, -0.5f, -0.5f), Vec3(0.5f, -0.5f, 0.5f), Vec3(-0.5f, -0.5f, 0.5f), Rgba8(255, 0, 255, 255), Rgba8(255, 0, 255, 255));

	//+Z
	AddVertsForQuad3D(m_vertices, Vec3(-0.5f, 0.5f, 0.5f), Vec3(-0.5f, -0.5f, 0.5f), Vec3(0.5f, -0.5f, 0.5f), Vec3(0.5f, 0.5f, 0.5f), Rgba8(0, 0, 255, 255), Rgba8(0, 0, 255, 255));

	//-Z
	AddVertsForQuad3D(m_vertices, Vec3(-0.5f, -0.5f, -0.5f), Vec3(-0.5f, 0.5f, -0.5f), Vec3(0.5f, 0.5f, -0.5f), Vec3(0.5f, -0.5f, -0.5f), Rgba8(255, 255, 0, 255), Rgba8(255, 255, 0, 255));

	g_theRenderer->CopyCPUToGPU(m_vertices.data(), (int)m_vertices.size() * sizeof(Vertex_PCU), m_gpuMesh);
}

Prop::~Prop()
{
	DELETE_PTR(m_gpuMesh);
	DELETE_PTR(m_modelCBO);
}

void Prop::Update(float deltaseconds)
{
	UNUSED(deltaseconds);
}

void Prop::Render() const
{
	g_theRenderer->SetModelConstants(RootSig::DEFAULT_PIPELINE, GetModelMatrix(), m_color, m_modelCBO);

	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->BindTexture();
	g_theRenderer->BindShader();

	g_theRenderer->DrawVertexBuffer(m_gpuMesh, (int)m_vertices.size(), sizeof(Vertex_PCU));
}
