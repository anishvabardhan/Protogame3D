#include "Game/Entity.hpp"

#include "Game/GameCommon.hpp"

Entity::Entity(Game* owner)
	: m_game(owner)
{
}

Entity::~Entity()
{
}

void Entity::Update(float deltaseconds)
{
	UNUSED(deltaseconds);
}

void Entity::Render() const
{
}

Mat44 Entity::GetModelMatrix() const
{
	Mat44 modelmatrix;

	modelmatrix.SetTranslation3D(m_position);
	modelmatrix.Append(m_orientationDegrees.GetAsMatrix_XFwd_YLeft_ZUp());

	return modelmatrix;
}

void Entity::SetOrientation()
{
}
