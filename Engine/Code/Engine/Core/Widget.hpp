#pragma once

#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/AABB2.hpp"

#include <vector>
#include <string>

class Texture;
class IndexBuffer;
class VertexBuffer;

class Widget
{
public:
	Rgba8					m_color			= Rgba8::WHITE;
	AABB2					m_bounds		= AABB2::ZERO_TO_ONE;
	std::string				m_name;
	std::string				m_value;
	Texture*				m_texture		= nullptr;
	VertexBuffer*			m_vbo			= nullptr;
	std::vector<Vertex_PCU> m_vertices;
	std::vector<Widget*>		m_children;
	float					m_fontAspect	= 0.5f;
	float					m_titleFontSize = 0.0f;
	float					m_menuFontSize	= 0.0f;
	float					m_gameFontSize	= 0.0f;
	float					m_unitImageSize = 0.0f;
	float					m_iconSize		= 0.0f;
	float					m_padding		= 0.0f;
public:
							Widget();
							~Widget();

	void					Update();
	void					Render() const;

	void					CreateBuffers();

	void					InitializeWidgetMesh(int paddingRate = 1.0f);

	Widget&					SetBounds(AABB2 const& bounds);
	Widget&					SetName(std::string const& name);
	Widget&					SetColor(Rgba8 const& color);
	Widget&					SetTexture(Texture* texture);

	Widget&					AddChild(Widget& widget, Vec2 const& alignment = Vec2::ZERO);
};