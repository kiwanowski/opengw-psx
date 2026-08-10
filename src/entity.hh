
#pragma once

#include <stdint.h>

#include "fixed.hh"
#include "model.hh"
#include "render.hh"

namespace gw {

class Entity {
  public:
    enum Type : uint8_t {
        TYPE_UNDEF = 0,
        TYPE_PLAYER1,
        TYPE_PLAYER_MISSILE,
        TYPE_GRUNT,
        TYPE_WANDERER,
        TYPE_WEAVER,
        TYPE_SPINNER,
        TYPE_TINYSPINNER,
        TYPE_MAYFLY,
        TYPE_SNAKE,
        TYPE_SNAKE_SEGMENT,
        TYPE_BLACKHOLE,
        TYPE_REPULSOR,
        TYPE_PROTON,
        TYPE_LINE,
        NUM_TYPES
    };

    enum State : uint8_t {
        STATE_INACTIVE = 0,
        STATE_SPAWN_TRANSITION,
        STATE_SPAWNING,
        STATE_RUN_TRANSITION,
        STATE_RUNNING,
        STATE_DESTROY_TRANSITION,
        STATE_DESTROYED,
        STATE_INDICATE_TRANSITION,
        STATE_INDICATING
    };

    virtual ~Entity() = default;

    virtual void run();
    virtual void spawnTransition();
    virtual void spawn();
    virtual void destroyTransition();
    virtual void destroy();
    virtual void indicateTransition();
    virtual void indicating();
    virtual void draw(Renderer& r);

    void tick();

    Type getType() const { return m_type; }

    const Vec2& getPos() const { return m_pos; }
    void setPos(const Vec2& p) { m_pos = p; }

    virtual Vec2 getHitPos() const { return m_pos; }

    virtual bool overlaps(const Vec2& pos, Fixed radius) const {
        Fixed reach = m_radius + radius;
        return (getHitPos() - pos).lengthSquared() < reach * reach;
    }

    virtual bool overlapsSegment(const Vec2& from, const Vec2& to, Fixed radius) const {
        Fixed reach = m_radius + radius;
        return mathutils::pointSegmentDistanceSquared(from, to, getHitPos()) < reach * reach;
    }

    virtual Fixed boundingRadius() const { return m_radius; }

    const Vec2& getSpeed() const { return m_speed; }
    void setSpeed(const Vec2& s) { m_speed = s; }

    Vec2 getDrift() const { return m_drift; }
    void setDrift(const Vec2& d) { m_drift = d; }

    Angle getAngle() const { return m_angle; }
    void setAngle(Angle a) { m_angle = wrapAngle(a); }

    Fixed getRadius() const { return m_radius; }

    Vec2 getScale() const { return m_scale; }

    const Model* getModel() const { return m_model; }

    bool getEnabled() const { return m_state != STATE_INACTIVE; }
    void setEnabled(bool e) { m_state = e ? STATE_SPAWN_TRANSITION : STATE_INACTIVE; }

    State getState() const { return m_state; }
    void setState(State s) { m_state = s; }

    int getScoreValue() const { return m_scoreValue; }

    uint16_t getGenId() const { return m_genId; }

    const Pen& getPen() const { return m_pen; }
    void setPen(const Pen& p) { m_pen = p; }

    static constexpr uint8_t kNoWave = 0xff;
    uint8_t getWaveId() const { return m_waveId; }
    void setWaveId(uint8_t id) { m_waveId = id; }

  protected:
    Type m_type = TYPE_UNDEF;
    State m_state = STATE_INACTIVE;

    Vec2 m_pos;
    Vec2 m_speed;
    Vec2 m_drift;
    Vec2 m_scale = Vec2(Fixed(1, 0), Fixed(1, 0));

    Angle m_angle;
    Angle m_rotationRate;

    Fixed m_radius;
    Fixed m_aggression = Fixed(1, 0);

    const Model* m_model = nullptr;
    Pen m_pen;

    int16_t m_stateTimer = 0;
    int16_t m_spawnTime = 40;
    int16_t m_destroyTime = 3;
    int16_t m_indicateTime = 75;

    int16_t m_scoreValue = 0;
    uint16_t m_genId = 0;

    uint8_t m_waveId = kNoWave;

    bool m_edgeBounce = false;
    bool m_gridBound = true;
};

}
