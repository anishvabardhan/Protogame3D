#include "Game/App.hpp"

#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Window/Window.hpp"

#include "Game/Game.hpp"

App* g_theApp = nullptr;
Renderer* g_theRenderer = nullptr;
AudioSystem* g_theAudio = nullptr;
Window* g_theWindow = nullptr;
Game* g_theGame = nullptr;

App::App()
{
}

App::~App()
{
}

void App::StartUp()
{
	EventSystemConfig eventSystemConfig;
	g_theEventSystem = new EventSystem(eventSystemConfig);

	DevConsoleConfig consoleConfig;
	consoleConfig.m_fontFilePath = "Data/Fonts/RobotoMonoSemiBold128.png";
	g_theConsole = new DevConsole(consoleConfig);

	InputConfig inputConfig;
	g_theInputSystem = new InputSystem(inputConfig);

	WindowConfig windowConfig;
	windowConfig.m_inputSystem = g_theInputSystem;
	windowConfig.m_windowTitle = "Protogame3D";
	windowConfig.m_clientAspect = 2.0f;
	windowConfig.m_isFullScreen = false;
	g_theWindow = new Window(windowConfig);

	RenderConfig renderConfig;
	renderConfig.m_window = g_theWindow;
	g_theRenderer = new Renderer(renderConfig);

	g_theAudio = new AudioSystem();
	g_theGame = new Game();

	g_theEventSystem->StartUp();
	g_theConsole->StartUp();
	g_theInputSystem->StartUp();
	g_theWindow->StartUp();
	g_theRenderer->StartUp();
	
	DebugRenderConfig debugConfig;
	debugConfig.m_renderer = g_theRenderer;

	DebugRenderSystemStartup(debugConfig);
	
	g_theAudio->Startup();
	g_theGame->StartUp();

	SubscribeEventCallbackFunction("QUIT", App::QuitApp);
}

void App::Run()
{
	while (!IsQuitting())
	{
		RunFrame();
	}
}

void App::ShutDown()
{
	g_theGame->Shutdown();
	g_theAudio->Shutdown();
	DebugRenderSystemShutdown();
	g_theRenderer->ShutDown();
	g_theWindow->ShutDown();
	g_theInputSystem->ShutDown();
	g_theConsole->ShutDown();
	g_theEventSystem->ShutDown();

	DELETE_PTR(g_theGame);
	DELETE_PTR(g_theConsole);
	DELETE_PTR(g_theEventSystem);
	DELETE_PTR(g_theRenderer);
	DELETE_PTR(g_theAudio);
	DELETE_PTR(g_theWindow);
	DELETE_PTR(g_theInputSystem);
}

void App::RunFrame()
{
	Clock::GetSystemClock().TickSystemClock();

	BeginFrame();
	Update(Clock::GetSystemClock().GetDeltaSeconds());
	Render();
	EndFrame();
}

void App::BeginFrame()
{
	g_theEventSystem->BeginFrame();
	g_theConsole->BeginFrame();
	g_theWindow->BeginFrame();
	g_theInputSystem->BeginFrame();
	g_theRenderer->BeginFrame();
	g_theAudio->BeginFrame();

	DebugRenderBeginFrame();
}

void App::Update(float deltaseconds)
{
	/*if (g_theConsole->IsOpen() && g_theGame->m_isAttractMode)
	{
		g_theInputSystem->SetCursorMode(false, false);
	}
	else
	{
		g_theInputSystem->SetCursorMode(true, true);
	}*/

	if (!m_isCursorVisible)
	{
		g_theInputSystem->SetCursorMode(true, true);
	}
	else
	{
		g_theInputSystem->SetCursorMode(false, false);
	}

	if (g_theInputSystem->WasKeyJustPressed('C'))
	{
		m_isCursorVisible = !m_isCursorVisible;
	}

	if (g_theInputSystem->WasKeyJustPressed('P'))
	{
		m_isPaused = !m_isPaused;
	}

	if (g_theInputSystem->WasKeyJustPressed('O'))
	{
		g_theGame->Update(deltaseconds);
		m_isPaused = true;
	}

	if (g_theInputSystem->WasKeyJustPressed('T'))
	{
		m_isSlowMo = true;
	}
	if (g_theInputSystem->WasKeyJustReleased('T'))
	{
		m_isSlowMo = false;
	}

	if (g_theInputSystem->WasKeyJustPressed('Y'))
	{
		m_isFastMo = true;
	}
	if (g_theInputSystem->WasKeyJustReleased('Y'))
	{
		m_isFastMo = false;
	}

	if (g_theInputSystem->WasKeyJustPressed(119))
	{
		g_theGame->Shutdown();
		delete g_theGame;
		g_theGame = new Game();
		g_theGame->StartUp();
	}

	if (g_theInputSystem->WasKeyJustPressed(KEYCODE_TILDE))
	{
		g_theConsole->ToggleOpen();
	}

	if (m_isPaused && !m_isSlowMo && !m_isFastMo)
	{
		g_theGame->m_gameClock->SetTimeScale(0.0f);
	}
	else if (!m_isPaused && m_isSlowMo && !m_isFastMo)
	{
		g_theGame->m_gameClock->SetTimeScale(0.1f);
	}
	else if (!m_isPaused && !m_isSlowMo && m_isFastMo)
	{
		g_theGame->m_gameClock->SetTimeScale(4.0f);
	}
	else if (!m_isPaused && !m_isSlowMo && !m_isFastMo)
	{
		g_theGame->m_gameClock->SetTimeScale(1.0f);
	}

	g_theGame->Update(g_theGame->m_gameClock->GetDeltaSeconds());
}

void App::Render() const
{
	g_theRenderer->ClearScreen(Rgba8(25, 25, 25, 255));

	g_theGame->Render();

	if (g_theConsole->IsOpen())
	{
		g_theRenderer->BeginCamera(*g_theGame->m_screenCamera, RootSig::DEFAULT_PIPELINE);
	
		g_theConsole->Render(AABB2(g_theGame->m_screenCamera->GetOrthoBottomLeft(), g_theGame->m_screenCamera->GetOrthoTopRight()), g_theRenderer);
	
		g_theRenderer->EndCamera(*g_theGame->m_screenCamera);
	}
}

void App::EndFrame()
{
	g_theAudio->EndFrame();
	g_theRenderer->EndFrame();
	g_theWindow->EndFrame();
	g_theInputSystem->EndFrame();
	g_theConsole->EndFrame();
	g_theEventSystem->EndFrame();

	DebugRenderEndFrame();
}

void App::HandleKeyPressed(unsigned char keyCode)
{
	g_theInputSystem->HandleKeyPressed(keyCode);
}

void App::HandleKeyReleased(unsigned char keyCode)
{
	g_theInputSystem->HandleKeyReleased(keyCode);
}

void App::HandleQuitRequested()
{
	m_isQuitting = true;
}

float App::GetDeltaTime() const
{
	return m_deltaTime;
}

void App::SetDeltaTime(float newDeltaTime)
{
	m_deltaTime = newDeltaTime;
}

bool App::QuitApp(EventArgs& args)
{
	UNUSED(args);

	g_theApp->HandleQuitRequested();

	return true;
}
