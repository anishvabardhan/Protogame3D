#pragma once

#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/Camera.hpp"

#include "Game/GameCommon.hpp"

#include <vector>

class Entity;
class Player;
class Prop;
class Clock;
class BitmapFont;
class VertexBuffer;
class ConstantBuffer;

class Game
{
public:
	Camera*					m_screenCamera					= nullptr;
	std::vector<Vertex_PCU> m_gridCPUMesh;
	VertexBuffer*			m_gridGPUMesh					= nullptr;
	std::vector<Vertex_PCU> m_sphereCPUMesh;
	VertexBuffer*			m_sphereGPUMesh					= nullptr;
	ConstantBuffer*			m_sphereModelCBO				= nullptr;
	Player*					m_player						= nullptr;
	//Prop*					m_prop							= nullptr;
	//Prop*					m_prop2							= nullptr;
	Clock*					m_gameClock						= nullptr;
	BitmapFont*				m_bitmapFont					= nullptr;

	float					m_thickness						= 1.0f;

	Vec2					m_attractModePosition			= Vec2(500.0f, 400.0f);
	bool					m_isAttractMode					= true;
	Texture*				m_sphereTexture					= nullptr;
	EulerAngles				m_sphereOrientation;
public:
							Game();
							~Game();

	void					StartUp();
	void					Shutdown();

	void					Update(float deltaseconds);
	void					Render() const ;

	void					InitDevConsoleInfo();

	void					HandleInput();
	void					UpdateFromController(float deltaseconds);
	void					UpdateAttractMode(float deltaseconds);

	void					InitializeGrid();
	void					RenderGrid() const;

	void					RenderSphere() const;

	void					RenderAttractMode() const;
};