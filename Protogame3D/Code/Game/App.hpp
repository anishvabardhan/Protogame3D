#pragma once

#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/EventSystem.hpp"

class Camera;
class Game;

class App
{
	double				m_previousFrameTime = 0.0f;
	bool				m_isQuitting = false;
	bool				m_isPaused = false;
	bool				m_isSlowMo = false;
	bool				m_isFastMo = false;
	bool				m_isCursorVisible = false;
	float				m_deltaTime = 0.0f;
public:
						App();
						~App();

	void				StartUp();
	void				Run();
	void				ShutDown();

	bool				IsQuitting() const { return m_isQuitting; }
	void				HandleKeyPressed(unsigned char keyCode);
	void				HandleKeyReleased(unsigned char keyCode);
	void				HandleQuitRequested();

	float				GetDeltaTime() const;
	void				SetDeltaTime(float newDeltaTime);

	static bool			QuitApp(EventArgs& args);
private:
	void				RunFrame();
	void				BeginFrame();
	void				Update(float deltaseconds);
	void				Render() const;
	void				EndFrame();
};