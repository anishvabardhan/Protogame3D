#pragma once

#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Rgba8.hpp"

class Game;

class Entity
{
public:
	Vec3			m_position;
	Vec3			m_velocity;
	Rgba8			m_color					= Rgba8::WHITE;
	EulerAngles		m_orientationDegrees;
	EulerAngles		m_angularVelocity;
	Game*			m_game					= nullptr;
public:
					Entity() {}
					Entity(Game* owner);
	virtual			~Entity();

	virtual void	Update(float deltaseconds) = 0;
	virtual void	Render() const = 0;

	void			SetOrientation();

	Mat44			GetModelMatrix() const;
};