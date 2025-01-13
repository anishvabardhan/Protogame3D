#include "Game/Player.hpp"

#include "Game/GameCommon.hpp"

#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/MathUtils.hpp"

#include <vector>

Player::Player()
{
	m_position = Vec3(-2.0f, 0.0f, 0.0f);
	m_orientationDegrees = EulerAngles(0.0f, 0.0f, 0.0f);
	m_worldCamera = new Camera();
}

Player::Player(Game* owner, Vec3 position)
	: Entity(owner)
{
	m_position = position;
	m_orientationDegrees = EulerAngles(0.0f, 0.0f, 0.0f);
	m_worldCamera = new Camera();
}

Player::~Player()
{
	DELETE_PTR(m_worldCamera);
}

void Player::Update(float deltaseconds)
{
	m_velocity = Vec3(0.0f, 0.0f, 0.0f);
	m_angularVelocity = EulerAngles(0.0f, 0.0f, 0.0f);

	if(!g_theConsole->IsOpen())
		HandleInput();

	m_position += m_velocity * deltaseconds;
	m_orientationDegrees = EulerAngles(m_orientationDegrees.m_yawDegrees + m_angularVelocity.m_yawDegrees, m_orientationDegrees.m_pitchDegrees + m_angularVelocity.m_pitchDegrees, m_orientationDegrees.m_rollDegrees + m_angularVelocity.m_rollDegrees);
	m_orientationDegrees.m_pitchDegrees = GetClamped(m_orientationDegrees.m_pitchDegrees, -85.0f, 85.0f);
	m_orientationDegrees.m_rollDegrees = GetClamped(m_orientationDegrees.m_rollDegrees, -45.0f, 45.0f);

	m_worldCamera->SetTransform(m_position, m_orientationDegrees);
	m_worldCamera->SetRenderBasis(Vec3(0.0f, 0.0f, 1.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));
	m_worldCamera->SetPerspectiveView(2.0f, 60.0f, 0.1f, 100.0f);
}

void Player::Render() const
{
	
}

void Player::HandleInput()
{
	m_angularVelocity.m_yawDegrees = 0.05f * g_theInputSystem->GetCursorClientDelta().x;
	m_angularVelocity.m_pitchDegrees = -0.05f * g_theInputSystem->GetCursorClientDelta().y;
	
	XboxController const& controller = g_theInputSystem->GetController(0);

	if (controller.GetLeftStick().GetMagnitude() > 0.0f)
	{
		m_velocity = (-2.0f * controller.GetLeftStick().GetPosition().x * m_orientationDegrees.GetAsMatrix_XFwd_YLeft_ZUp().GetJBasis3D()) + (2.0f * controller.GetLeftStick().GetPosition().y * m_orientationDegrees.GetAsMatrix_XFwd_YLeft_ZUp().GetIBasis3D());
	}

	if (controller.GetButton(BUTTON_RIGHT_SHOULDER).m_isPressed)
	{
		m_velocity = 2.0f * m_orientationDegrees.GetAsMatrix_XFwd_YLeft_ZUp().GetKBasis3D();
	}

	if (controller.GetButton(BUTTON_LEFT_SHOULDER).m_isPressed)
	{
		m_velocity = -2.0f * m_orientationDegrees.GetAsMatrix_XFwd_YLeft_ZUp().GetKBasis3D();
	}

	if (controller.GetButton(BUTTON_START).m_isPressed || g_theInputSystem->WasKeyJustPressed('H'))
	{
		m_position = Vec3(0.0f, 0.0f, 0.0f);
		m_orientationDegrees = EulerAngles(0.0f, 0.0f, 0.0f);
	}
	
	if (controller.GetRightStick().GetMagnitude() > 0.0f)
	{
		m_angularVelocity.m_pitchDegrees = -controller.GetRightStick().GetPosition().y;
		m_angularVelocity.m_yawDegrees = -controller.GetRightStick().GetPosition().x;
	}

	m_angularVelocity.m_rollDegrees = (-1.0f * controller.GetLeftTrigger()) + (1.0f * controller.GetRightTrigger());

	if (g_theInputSystem->IsKeyDown('W'))
	{
		m_velocity = 2.0f * m_orientationDegrees.GetAsMatrix_XFwd_YLeft_ZUp().GetIBasis3D();
	}
	
	if (g_theInputSystem->IsKeyDown('S'))
	{
		m_velocity = -2.0f * m_orientationDegrees.GetAsMatrix_XFwd_YLeft_ZUp().GetIBasis3D();
	}
	
	if (g_theInputSystem->IsKeyDown('A'))
	{
		m_velocity = 2.0f * m_orientationDegrees.GetAsMatrix_XFwd_YLeft_ZUp().GetJBasis3D();
	}
	
	if (g_theInputSystem->IsKeyDown('D'))
	{
		m_velocity = -2.0f * m_orientationDegrees.GetAsMatrix_XFwd_YLeft_ZUp().GetJBasis3D();
	}
	
	if (g_theInputSystem->IsKeyDown('Z'))
	{
		m_velocity = -2.0f * m_orientationDegrees.GetAsMatrix_XFwd_YLeft_ZUp().GetKBasis3D();
	}

	if (g_theInputSystem->IsKeyDown('C'))
	{
		m_velocity = 2.0f * m_orientationDegrees.GetAsMatrix_XFwd_YLeft_ZUp().GetKBasis3D();
	}

	if (controller.GetButton(BUTTON_A).m_isPressed || g_theInputSystem->IsKeyDown(KEYCODE_SHIFT))
	{
		m_velocity *= 10.0f;
	}
	
	if (g_theInputSystem->IsKeyDown('Q'))
	{
		m_angularVelocity.m_rollDegrees = -1.5f;
	}

	if (g_theInputSystem->IsKeyDown('E'))
	{
		m_angularVelocity.m_rollDegrees = 1.5f;
	}

	if (g_theInputSystem->IsKeyDown(KEYCODE_UPARROW))
	{
		m_angularVelocity.m_pitchDegrees = -1.5f;
	}
	
	if (g_theInputSystem->IsKeyDown(KEYCODE_DOWNARROW))
	{
		m_angularVelocity.m_pitchDegrees = 1.5f;
	}
	
	if (g_theInputSystem->IsKeyDown(KEYCODE_LEFTARROW))
	{
		m_angularVelocity.m_yawDegrees = 1.5f;
	}
	
	if (g_theInputSystem->IsKeyDown(KEYCODE_RIGHTARROW))
	{
		m_angularVelocity.m_yawDegrees = -1.5f;
	}

	if (g_theInputSystem->WasKeyJustPressed('1'))
	{
		Vec3 end = m_position + Vec3::MakeFromPolarDegrees(m_orientationDegrees.GetPitch(), m_orientationDegrees.GetYaw(), 20.0f);

		DebugAddWorldLine(m_position, end, 0.1f, 10.0f, Rgba8::YELLOW, Rgba8::YELLOW, DebugRenderMode::X_RAY);
	}

	if (g_theInputSystem->IsKeyDown('2'))
	{
		Vec3 end = m_position + Vec3::MakeFromPolarDegrees(m_orientationDegrees.GetPitch(), m_orientationDegrees.GetYaw(), 20.0f);

		DebugAddWorldPoint(Vec3(m_position.x, m_position.y, 0.0f), 2.0f, 60.0f, Rgba8(150, 75, 0, 255), Rgba8(150, 75, 0, 255));
	}

	if (g_theInputSystem->WasKeyJustPressed('3'))
	{
		Vec3 pos = m_position + Vec3::MakeFromPolarDegrees(m_orientationDegrees.GetPitch(), m_orientationDegrees.GetYaw(), 2.0f);

		DebugAddWorldWireSphere(pos, 1.0f, 5.0f, Rgba8::GREEN, Rgba8::RED);
	}

	if (g_theInputSystem->WasKeyJustPressed('4'))
	{
		DebugAddWorldBasis(GetModelMatrix(), 10.0f, 1.0f, 0.05f);
	}

	if (g_theInputSystem->WasKeyJustPressed('5'))
	{
		DebugAddWorldBillboardText(Stringf("POSITION:-  X:%.1f Y:%.1f Z:%.1f, ORIENTATION:- YAW:%.1f PITCH:%.1f ROLL:%.1f", m_position.x, m_position.y, m_position.z, m_orientationDegrees.GetYaw(), m_orientationDegrees.GetPitch(), m_orientationDegrees.GetRoll()), m_worldCamera->GetModelMatrix(), 0.25f, Vec2(0.5f, 0.5f), 10.0f, Rgba8::WHITE, Rgba8::RED);
	}

	if (g_theInputSystem->WasKeyJustPressed('6'))
	{
		Vec3 top = m_position + Vec3::MakeFromPolarDegrees(-90.0f, 0.0f, 5.0f);
		DebugAddWorldWireCylinder(m_position, top, 1.5f, 10.0f, Rgba8::WHITE, Rgba8::RED);
	}

	if (g_theInputSystem->WasKeyJustPressed('7'))
	{
		DebugAddMessage(Stringf("ORIENTATION:- YAW:%.1f PITCH:%.1f ROLL:%.1f", m_orientationDegrees.GetYaw(), m_orientationDegrees.GetPitch(), m_orientationDegrees.GetRoll()), Vec3(), 15.0f, Vec2(0.5, 0.5f), -1.0f);
	}
}
