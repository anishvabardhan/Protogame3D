#include "Game/Game.hpp"

#include "Game/Player.hpp"
//#include "Game/Prop.hpp"

#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/SimpleTriangleFont.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"

Game::Game()
{
}

Game::~Game()
{
}

void Game::StartUp()
{
	m_gameClock = new Clock(Clock::GetSystemClock());

	m_screenCamera = new Camera();

	m_bitmapFont = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont.png");

	m_gridGPUMesh = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PCU), std::wstring(L"Grid"));
	m_sphereGPUMesh = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PCU), std::wstring(L"Sphere"));
	m_sphereModelCBO = g_theRenderer->CreateConstantBuffer(sizeof(ModelConstants), std::wstring(L"Sphere Model"));

	m_player = new Player(this, Vec3(-5.0f, 0.0f, 2.0f));
	//m_prop = new Prop(this, Vec3(2.0f, 2.0f, 0.0f), Rgba8::WHITE);
	//m_prop2 = new Prop(this, Vec3(-2.0f, -2.0f, 0.0f), Rgba8::WHITE);

	AddVertsForSphere3D(m_sphereCPUMesh, Vec3(0.0f, 0.0f, 0.0f), 1.0f, Rgba8::WHITE, AABB2::ZERO_TO_ONE);
	g_theRenderer->CopyCPUToGPU(m_sphereCPUMesh.data(), (int)m_sphereCPUMesh.size() * sizeof(Vertex_PCU), m_sphereGPUMesh);

	m_sphereTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Textures/TestUV.png");
	
	Mat44 xTextModelMatrix;
	xTextModelMatrix.SetTranslation3D(Vec3(0.7f, 0.0f, 0.15f));
	xTextModelMatrix.Append(EulerAngles(90.0f, 0.0f, 0.0f).GetAsMatrix_XFwd_YLeft_ZUp());
	DebugAddWorldText("X - FORWARD", xTextModelMatrix, 0.1f, Vec2(0.5f, 0.5f), -1.0f, Rgba8(255, 0, 0, 255));

	Mat44 yTextModelMatrix;
	yTextModelMatrix.SetTranslation3D(Vec3(0.0f, 0.6f, 0.15f));
	DebugAddWorldText("Y - LEFT", yTextModelMatrix, 0.1f, Vec2(0.5f, 0.5f), -1.0f, Rgba8(0, 255, 0, 255));

	Mat44 textModelMatrix;
	textModelMatrix.SetTranslation3D(Vec3(0.0f, -0.15f, 0.5f));
	textModelMatrix.Append(EulerAngles(0.0f, 0.0f, 90.0f).GetAsMatrix_XFwd_YLeft_ZUp());
	DebugAddWorldText("Z - UP", textModelMatrix, 0.1f, Vec2(0.5f, 0.5f), -1.0f, Rgba8(0, 0, 255, 255));

	DebugAddWorldArrow(Vec3(0.0f, 0.0f, 0.0f), Vec3(1.0f, 0.0f, 0.0f), 0.05f, -1.0f, Rgba8(255, 0, 0, 255));
	DebugAddWorldArrow(Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f), 0.05f, -1.0f, Rgba8(0, 255, 0, 255));
	DebugAddWorldArrow(Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f), 0.05f, -1.0f, Rgba8(0, 0, 255, 255));

	InitializeGrid();

	InitDevConsoleInfo();
}

void Game::Shutdown()
{
	//DELETE_PTR(m_prop);
	//DELETE_PTR(m_prop2);
	DELETE_PTR(m_player);
	DELETE_PTR(m_gridGPUMesh);
	DELETE_PTR(m_sphereGPUMesh);
	DELETE_PTR(m_sphereModelCBO);

	DELETE_PTR(m_screenCamera);
}

void Game::Update(float deltaseconds)
{
	//static float rate = 0.0f;
	//rate += 100.0f * deltaseconds;
	//m_thickness = 5.0f * fabsf(SinDegrees(rate));

	if (m_isAttractMode)
	{
		UpdateAttractMode(deltaseconds);

 		m_screenCamera->SetOrthoView(Vec2(0.0f, 0.0f), Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y));
	}
	else
	{
		static float brightness = 0.0f;
		brightness += 50.0f * deltaseconds;

		//m_prop2->m_color = Rgba8(unsigned char(255 * fabsf(SinDegrees(brightness))), unsigned char(255 * fabsf(SinDegrees(brightness))), unsigned char(255 * fabsf(SinDegrees(brightness))), 255);
		//
		//m_prop->m_orientationDegrees.m_rollDegrees += 30.0f * deltaseconds;
		//m_prop->m_orientationDegrees.m_pitchDegrees += 30.0f * deltaseconds;

		m_sphereOrientation.m_yawDegrees += 45.0f * deltaseconds;

		//m_prop->Update(deltaseconds);
		//m_prop2->Update(deltaseconds);
		m_player->Update(deltaseconds);

		float fps = 1.0f / Clock::GetSystemClock().GetDeltaSeconds();
		float timeScale = Clock::GetSystemClock().GetTimeScale();
		float totalTime = Clock::GetSystemClock().GetTotalSeconds();

		Mat44 screenTextModelMatrix;
		screenTextModelMatrix.SetTranslation3D(Vec3(SCREEN_SIZE_X * 0.5f, SCREEN_SIZE_Y - 20.0f, 0.0f));
		DebugAddScreenText(Stringf("POSITION:-    %.1f   %.1f   %.1f                         TIME: %.1fs FPS: %.1f TIMESCALE: %.1f", m_player->m_position.x, m_player->m_position.y, m_player->m_position.z, totalTime, fps, timeScale), screenTextModelMatrix, 15.0f, Vec2(0.5f, 0.5f), 0.0f);

		m_screenCamera->SetOrthoView(Vec2(0.0f, 0.0f), Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y));
	}
	
	if (!g_theConsole->IsOpen())
	{
		HandleInput();
		UpdateFromController(deltaseconds);
	} 
}

void Game::Render() const
{
	if (m_isAttractMode)
	{
		g_theRenderer->BeginCamera(*m_screenCamera, RootSig::DEFAULT_PIPELINE);
	
		RenderAttractMode();
	
		g_theRenderer->EndCamera(*m_screenCamera);
	}
	else
	{
		g_theRenderer->BeginCamera(*m_player->m_worldCamera, RootSig::DEFAULT_PIPELINE);
		
		RenderGrid();
		//m_prop->Render();
		//m_prop2->Render();
		RenderSphere();

		g_theRenderer->EndCamera(*m_player->m_worldCamera);
	
 		DebugRenderWorld(*m_player->m_worldCamera);
		DebugRenderScreen(*m_screenCamera);
	}
}

void Game::InitDevConsoleInfo()
{
	DevConsoleLine line = DevConsoleLine("----------------------------------------", DevConsole::INFO_MAJOR);
	g_theConsole->m_lines.push_back(line);

	line = DevConsoleLine("CONTROLS", DevConsole::COMMAND_ECHO);
	g_theConsole->m_lines.push_back(line);

	line = DevConsoleLine("PRESS Q/LEFT TRIGGER          ----> ROLL CAMERA LEFT", DevConsole::INFO_MINOR);
	g_theConsole->m_lines.push_back(line);

	line = DevConsoleLine("PRESS E/RIGHT TRIGGER         ----> ROLL CAMERA RIGHT", DevConsole::INFO_MINOR);
	g_theConsole->m_lines.push_back(line);

	line = DevConsoleLine("PRESS H/START BUTTON          ----> RESET CAMERA POSITION AND ORIENTATION TO ZERO", DevConsole::INFO_MINOR);
	g_theConsole->m_lines.push_back(line);

	line = DevConsoleLine("HOLD SHIFT/A BUTTON           ----> INCREASE CAMERA SPEED BY A FACTOR OF 10 WHILE HELD", DevConsole::INFO_MINOR);
	g_theConsole->m_lines.push_back(line);

	line = DevConsoleLine("PRESS Z/LEFT SHOULDER         ----> MOVE UP RELATIVE TO WORLD", DevConsole::INFO_MINOR);
	g_theConsole->m_lines.push_back(line);

	line = DevConsoleLine("PRESS C/RIGHT SHOULDER        ----> MOVE DOWN RELATIVE TO WORLD", DevConsole::INFO_MINOR);
	g_theConsole->m_lines.push_back(line);

	line = DevConsoleLine("PRESS W/LEFT STICK Y-AXIS     ----> MOVE CAMERA FORWARD", DevConsole::INFO_MINOR);
	g_theConsole->m_lines.push_back(line);

	line = DevConsoleLine("PRESS S/LEFT STICK Y-AXIS     ----> MOVE CAMERA BACKWARD", DevConsole::INFO_MINOR);
	g_theConsole->m_lines.push_back(line);

	line = DevConsoleLine("PRESS D/LEFT STICK X-AXIS     ----> MOVE CAMERA RIGHT", DevConsole::INFO_MINOR);
	g_theConsole->m_lines.push_back(line);

	line = DevConsoleLine("PRESS A/LEFT STICK X-AXIS     ----> MOVE CAMERA LEFT", DevConsole::INFO_MINOR);
	g_theConsole->m_lines.push_back(line);

	line = DevConsoleLine("PRESS UP/MOUSE                ----> PITCH CAMERA UP", DevConsole::INFO_MINOR);
	g_theConsole->m_lines.push_back(line);

	line = DevConsoleLine("PRESS DOWN/MOUSE              ----> PITCH CAMERA DOWN", DevConsole::INFO_MINOR);
	g_theConsole->m_lines.push_back(line);

	line = DevConsoleLine("PRESS RIGHT/MOUSE             ----> YAW CAMERA RIGHT", DevConsole::INFO_MINOR);
	g_theConsole->m_lines.push_back(line);

	line = DevConsoleLine("PRESS LEFT/MOUSE              ----> YAW CAMERA LEFT", DevConsole::INFO_MINOR);
	g_theConsole->m_lines.push_back(line);

	line = DevConsoleLine("PRESS 1                       ----> Spawn a line from the player along their forward direction 20 units in length. Duration 10 seconds, draw in x-ray mode, color yellow.", DevConsole::INFO_MINOR);
	g_theConsole->m_lines.push_back(line);

	line = DevConsoleLine("PRESS 2                       ----> Spawn a sphere directly below the player position on the xy-plane. Duration 60 seconds, draw with depth, color RGB values 150, 75, 0.", DevConsole::INFO_MINOR);
	g_theConsole->m_lines.push_back(line);

	line = DevConsoleLine("PRESS 3                       ----> Spawn a wireframe sphere 2 units in front of player camera. Radius 1, duration 5 seconds, draw with depth, color transitions from green to red.", DevConsole::INFO_MINOR);
	g_theConsole->m_lines.push_back(line);

	line = DevConsoleLine("PRESS 4                       ----> Spawn a basis using the player current model matrix. Duration 20 seconds, draw with depth, color white.", DevConsole::INFO_MINOR);
	g_theConsole->m_lines.push_back(line);

	line = DevConsoleLine("PRESS 5                       ----> Spawn billboarded text showing the player position and orientation. Duration 10 seconds, draw with depth, color transitions from white to red.", DevConsole::INFO_MINOR);
	g_theConsole->m_lines.push_back(line);

	line = DevConsoleLine("PRESS 6                       ----> Spawn a wireframe cylinder at the player position. Duration 10 seconds, draw with depth, color transitions from white to red.", DevConsole::INFO_MINOR);
	g_theConsole->m_lines.push_back(line);

	line = DevConsoleLine("PRESS 7                       ----> Spawn a wireframe cylinder at the player position. Duration 10 seconds, draw with depth, color transitions from white to red.", DevConsole::INFO_MINOR);
	g_theConsole->m_lines.push_back(line);

	line = DevConsoleLine("----------------------------------------", DevConsole::INFO_MAJOR);
	g_theConsole->m_lines.push_back(line);
}

void Game::InitializeGrid()
{
	for (int k = 0; k < 100; k++)
	{
		AddVertsForAABB3D(m_gridCPUMesh, AABB3(-50.0f, -50.01f + k, -0.01f, 50.0f, -49.99f + k, 0.01f), Rgba8(120, 120, 120, 255));
	}

	for (int l = 0; l < 100; l++)
	{
		AddVertsForAABB3D(m_gridCPUMesh, AABB3(-50.01f + l, -50.0f, -0.01f, -49.99f + l, 50.0f, 0.01f), Rgba8(120, 120, 120, 255));
	}

	for (int x = 0; x < 105; x += 5)
	{
		if (x == 0 || x == 100 || x == 50.0f)
		{
			AddVertsForAABB3D(m_gridCPUMesh, AABB3(-50.04f + x, -50.0f, -0.04f, -49.96f + x, 50.0f, 0.04f), Rgba8(0, 255, 0, 255));
		}
		else
		{
			AddVertsForAABB3D(m_gridCPUMesh, AABB3(-50.04f + x, -50.0f, -0.04f, -49.96f + x, 50.0f, 0.04f), Rgba8(24, 128, 55, 255));
		}
	}

	for (int y = 0; y < 105; y += 5)
	{
		if (y == 0 || y == 100 || y == 50.0f)
		{
			AddVertsForAABB3D(m_gridCPUMesh, AABB3(-50.0f, -50.04f + y, -0.04f, 50.0f, -49.96f + y, 0.04f), Rgba8(255, 0, 0, 255));
		}
		else
		{
			AddVertsForAABB3D(m_gridCPUMesh, AABB3(-50.0f, -50.04f + y, -0.04f, 50.0f, -49.96f + y, 0.04f), Rgba8(136, 0, 21, 255));
		}
	}

	g_theRenderer->CopyCPUToGPU(m_gridCPUMesh.data(), (int)m_gridCPUMesh.size() * sizeof(Vertex_PCU), m_gridGPUMesh);
}

void Game::RenderGrid() const
{
	g_theRenderer->SetModelConstants(RootSig::DEFAULT_PIPELINE, Mat44(), Rgba8::WHITE);

	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->BindTexture();
	g_theRenderer->BindShader();

	g_theRenderer->DrawVertexBuffer(m_gridGPUMesh, (int)m_gridCPUMesh.size(), sizeof(Vertex_PCU));
}

void Game::RenderSphere() const
{
	Mat44 modelMatrix;

	modelMatrix.SetTranslation3D(Vec3(10.0f, -5.0f, 1.0f));
	modelMatrix.Append(m_sphereOrientation.GetAsMatrix_XFwd_YLeft_ZUp());

	g_theRenderer->SetModelConstants(RootSig::DEFAULT_PIPELINE, modelMatrix, Rgba8::WHITE, m_sphereModelCBO);

	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->BindTexture(0, m_sphereTexture);
	g_theRenderer->BindShader();

	g_theRenderer->DrawVertexBuffer(m_sphereGPUMesh, (int)m_sphereCPUMesh.size(), sizeof(Vertex_PCU));
}

void Game::RenderAttractMode() const
{
	DrawDebugRing(Vec2(800.0f, 400.0f), 200.0f, 0.0f, m_thickness, Rgba8(0, 255, 0, 255));
}

void Game::HandleInput()
{
	if (m_isAttractMode)
	{
		if (g_theInputSystem->WasKeyJustPressed(KEYCODE_ESC))
		{
			g_theApp->HandleQuitRequested();
		}

		if (g_theInputSystem->WasKeyJustPressed(KEYCODE_SPACE))
		{
			m_isAttractMode = false;
		}
	}
	else
	{
		if (g_theInputSystem->WasKeyJustPressed(KEYCODE_ESC))
		{
			m_isAttractMode = true;
		}
	}
}

void Game::UpdateAttractMode(float deltaseconds)
{
	UNUSED(deltaseconds);
}

void Game::UpdateFromController(float deltaseconds)
{
	UNUSED(deltaseconds);

	XboxController const& controller = g_theInputSystem->GetController(0);

	if (m_isAttractMode)
	{
		if (controller.WasButtonJustPressed(XboxButtonID::BUTTON_B))
		{
			g_theApp->HandleQuitRequested();
		}
	}
}