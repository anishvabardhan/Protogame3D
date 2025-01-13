#pragma once

#include "Game/Entity.hpp"

class Camera;

class Player : public Entity
{
public:
	Camera* m_worldCamera = nullptr;
public:
	Player();
	Player(Game* owner, Vec3 position);
	~Player();

	virtual void	Update(float deltaseconds) override;
	virtual void	Render() const override;

	void			HandleInput();
};