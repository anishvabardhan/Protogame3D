#include "Widget.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"

Widget::Widget()
{
	m_titleFontSize = RangeMapClamped((float)g_theWindow->GetClientDimensions().y, 500.0f, 1000.0f, 0.5f * 128.0f, 128.0f);
	m_menuFontSize  = RangeMapClamped((float)g_theWindow->GetClientDimensions().y, 500.0f, 1000.0f, 0.5f *  64.0f,  64.0f);
	m_gameFontSize  = RangeMapClamped((float)g_theWindow->GetClientDimensions().y, 500.0f, 1000.0f, 0.5f *  32.0f,  32.0f);
	m_unitImageSize = RangeMapClamped((float)g_theWindow->GetClientDimensions().y, 500.0f, 1000.0f, 0.5f * 256.0f, 256.0f);
	m_iconSize      = RangeMapClamped((float)g_theWindow->GetClientDimensions().y, 500.0f, 1000.0f, 0.5f *  32.0f,  32.0f);
	m_padding		= RangeMapClamped((float)g_theWindow->GetClientDimensions().y, 500.0f, 1000.0f, 10.0f, 20.0f);
}

Widget::~Widget()
{/*
	if (m_texture && m_texture->m_texture && m_texture->GetDimensions().x >= 0)
	{
		DELETE_PTR(m_texture);
	}*/

	DELETE_PTR(m_vbo);

	for (int i = 0; i < m_children.size(); i++)
	{
		DELETE_PTR(m_children[i]);
	}
}

void Widget::Update()
{
	for (Widget* child : m_children)
	{
		child->Update();
	}
}

void Widget::Render() const
{
	g_theRenderer->SetModelConstants(RootSig::DEFAULT_PIPELINE, Mat44(), m_color);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(0, m_texture);
	g_theRenderer->DrawVertexBuffer(m_vbo, (int)m_vertices.size(), sizeof(Vertex_PCU));

	for (Widget const* child : m_children)
	{
		child->Render();
	}
}

void Widget::CreateBuffers()
{
	m_vbo = g_theRenderer->CreateVertexBuffer((int)m_vertices.size() * sizeof(Vertex_PCU));
	g_theRenderer->CopyCPUToGPU(m_vertices.data(), (int)m_vertices.size() * sizeof(Vertex_PCU), m_vbo);
}

void Widget::InitializeWidgetMesh(int paddingRate)
{
	AABB2 bounds;
	bounds = AABB2(Vec2(m_bounds.m_mins.x + (paddingRate * m_padding), m_bounds.m_mins.y + (paddingRate * m_padding)), Vec2(m_bounds.m_maxs.x - (paddingRate * m_padding), m_bounds.m_maxs.y - (paddingRate * m_padding)));

	AddVertsForAABB2D(m_vertices, bounds, Rgba8::WHITE);
}

Widget& Widget::SetBounds(AABB2 const& bounds)
{
	m_bounds = bounds;

	return *this;
}

Widget& Widget::SetName(std::string const& name)
{
	m_name = name;

	return *this;
}

Widget& Widget::SetColor(Rgba8 const& color)
{
	m_color = color;

	return *this;
}

Widget& Widget::SetTexture(Texture* texture)
{
	m_texture = texture;

	return *this;
}

Widget& Widget::AddChild(Widget& widget, Vec2 const& alignment)
{
	UNUSED(alignment);

	m_children.push_back(&widget);

	return *this;
}
