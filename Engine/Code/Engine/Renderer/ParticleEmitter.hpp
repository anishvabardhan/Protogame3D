#pragma once

#include <map>
#include <string>

#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"

class Camera;
class Texture;
class Renderer;
class VertexBuffer;
//
//struct EmitterDefinition
//{
//	std::string                     m_name;
//    int                             m_count;
//    std::string                     m_textureName;
//    std::string                     m_shaderName;
//    Vec3                            m_position; 
//    Vec3                            m_direction;
//    float                           m_spread;
//    float                           m_minLifetime;
//    float                           m_maxLifetime;
//    float                           m_minOffset;
//    float                           m_maxOffset;
//    float                           m_minRotation;
//    float                           m_maxRotation;
//    Rgba8                           m_minColor;
//    Rgba8                           m_maxColor;
//    float                           m_minSize;
//    float                           m_maxSize;
//    float                           m_minSpeed;
//    float                           m_maxSpeed;
//    float                           m_minRotationSpeed;
//    float                           m_maxRotationSpeed;
//    float                           m_startAlpha;
//    float                           m_endAlpha;
//    float                           m_startAlphaTime;
//    float                           m_endAlphaTime;
//    float                           m_startSpeed;
//    float                           m_endSpeed;
//    float                           m_startSpeedTime;
//    float                           m_endSpeedTime;
//    float                           startSize;
//    float                           m_endSize;
//    float                           m_startSizeTime;
//    float                           m_endSizeTime; 
//
//                                    EmitterDefinition() {};
//                                    EmitterDefinition(const XmlElement* element);
//
//    static void						InitializeEmitterDef();
//    static EmitterDefinition*		GetEmiterDefByName(const std::string& effectName, const std::string& emitterName);
//};
//
//static std::map<std::string, std::vector<EmitterDefinition>>	s_effectDefinitions;

struct Particle
{
public:
                                Particle(Renderer* renderer, Vec3 position, float lifeTime = 5.f, Texture* texture = nullptr, Vec2 size = Vec2(1.f, 1.f));
                                ~Particle();

    void                        Update(float deltaSeconds);
    void                        Render(Camera* camera, BillboardType type = BillboardType::NONE) const;

    Mat44                       GetModelMatrix() const;

public:
    Renderer*                   m_renderer          = nullptr;
    Texture*                    m_texture           = nullptr;

    float                       m_lifeTime          = 0.f;

    Vec3                        m_position;
    Vec3                        m_velocity;
    EulerAngles                 m_orientation;
    EulerAngles                 m_angular;
    Rgba8                       m_color             = Rgba8::WHITE;

    Vec2                        m_size              = Vec2(1.f, 1.f);
    Vec2                        m_scale             = Vec2(0.f, 0.f);

    std::vector<Vertex_PCU>     m_verts;
    VertexBuffer*               m_buffer;

    DepthMode                   m_depthMode         = DepthMode::DISABLED;
    BlendMode                   m_blendMode         = BlendMode::ADDITIVE;

    bool                        m_isGarbage         = false;
};

struct Emitter
{
                            Emitter(std::string name, Vec3 position, float timer = 2.f, unsigned int seed = 0U);
                            ~Emitter();

    void                    Update(float deltaSeconds);
    void                    Render(Camera* camera) const;

    void                    SetParticlePositionAndOrientation(Vec3 position, EulerAngles orientation);

    void                    Activate();
    void                    Stop();

public:
    Renderer*               m_renderer                  = nullptr;
    RandomNumberGenerator   m_rng;
    BillboardType           m_billboardType             = BillboardType::NONE;
    Vec3                    m_position;

    std::string             m_name;

    bool                    m_isStopped                 = true;

    float                   m_timer                     = 0.f;
    float                   m_lifeTime                  = 0.f;
    float                   m_spawnTimer                = 0.f;
    Vec3                    m_particlePosition;
    Vec3                    m_particleVelDirection;
    EulerAngles             m_particleOrientation       = EulerAngles();
    EulerAngles             m_particleAngular           = EulerAngles();
    Vec2                    m_particleScale             = Vec2();
    Rgba8                   m_particleColor             = Rgba8::WHITE;
    IntRange                m_numParticleEachSpawn      = IntRange(1, 3);
    FloatRange              m_particleLifeTime          = FloatRange(1.f, 3.f);
    FloatRange              m_particleSize              = FloatRange(1.f, 1.f);
    FloatRange              m_particleSpeed             = FloatRange(-1.f, 1.f);
    FloatRange              m_emitSpawnTimer            = FloatRange(1.f, 3.f);

    std::vector<Particle*>  m_currentParticles;
    std::vector<Texture*>   m_textures;

private:

    void                    SpawnParticle(Particle* particle);
};

class ParticleSystem
{
public:

    ParticleSystem(Renderer* renderer);
    ~ParticleSystem();

    void AddEmitter(Emitter* emitter);

    void Update(float deltaSeconds);
    void Render(Camera* camera) const;

    Emitter* GetEmitter(std::string name) const;

    void ActivateAll();
    void StopAll();

private:
    std::vector<Emitter*> m_emitters;
    Renderer* m_renderer;
};
