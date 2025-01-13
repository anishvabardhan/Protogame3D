#pragma once

#include "Engine/Core/Vertex_PCU.hpp"

#include "Game/Entity.hpp"

#include <vector>

class VertexBuffer;
class ConstantBuffer;
class Texture;

class Prop : public Entity
{
	std::vector<Vertex_PCU>		m_vertices;
	VertexBuffer*				m_gpuMesh		= nullptr;
	ConstantBuffer*				m_modelCBO		= nullptr;
	Texture*					m_texture		= nullptr;
public:
	Prop();
	Prop(Game* owner, Vec3 position, Rgba8 modelColor);
	~Prop();

	virtual void	Update(float deltaseconds) override;
	virtual void	Render() const override;
};