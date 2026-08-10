#include "entity.hh"

#include "world.hh"

namespace gw {

static constexpr Fixed kAggressionRate = 0.0002L;
static constexpr Fixed kDriftDecay = 0.95L;

void Entity::run() {
    m_aggression += kAggressionRate;

    m_pos += m_speed;
    m_pos += m_drift;
    m_angle = wrapAngle(m_angle + m_rotationRate);

    if (m_gridBound) {
        Vec2 hitPoint;
        Vec2 speed = m_speed;
        if (g_world.grid.hitTest(m_pos, m_radius, &hitPoint, &speed)) {
            m_pos = hitPoint;
            if (m_edgeBounce) m_speed = speed;
        }
    }

    m_drift *= kDriftDecay;
}

void Entity::spawnTransition() {
    setState(STATE_SPAWNING);
    m_stateTimer = m_spawnTime;
    m_speed = Vec2();
    m_drift = Vec2();
    m_angle = Angle();
    m_rotationRate = Angle();
    m_aggression = Fixed(1, 0);
    spawn();
}

void Entity::spawn() {
    if (--m_stateTimer <= 0) setState(STATE_RUNNING);
}

void Entity::destroyTransition() {
    setState(STATE_DESTROYED);
    m_stateTimer = m_destroyTime;
    ++m_genId;
}

void Entity::destroy() {
    if (--m_stateTimer <= 0) setState(STATE_INACTIVE);
}

void Entity::indicateTransition() {
    ++m_genId;
    m_stateTimer = m_indicateTime;
    setState(STATE_INDICATING);
}

void Entity::indicating() {
    if (--m_stateTimer <= 0) setState(STATE_INACTIVE);
}

void Entity::draw(Renderer& r) {
    if (!m_model) return;

    switch (m_state) {
    case STATE_INDICATING:
        if ((m_stateTimer / 5) & 1) m_model->draw(r, m_pos, m_angle, m_scale, m_pen);
        break;

    case STATE_SPAWNING: {
        if (m_spawnTime <= 0) break;
        Fixed progress = Fixed(m_stateTimer, 0) / m_spawnTime;
        Fixed grow = Fixed(1, 0) - progress;
        if (grow.value < 0) grow = Fixed();

        Pen pen = m_pen;
        pen.a = uint8_t((Fixed(pen.a, 0) * grow).value >> 12);
        m_model->draw(r, m_pos, m_angle, m_scale * grow, pen);
        break;
    }

    case STATE_INACTIVE:
    case STATE_SPAWN_TRANSITION:
        break;

    default:
        m_model->draw(r, m_pos, m_angle, m_scale, m_pen);
        break;
    }
}

void Entity::tick() {
    switch (m_state) {
    case STATE_SPAWN_TRANSITION: spawnTransition(); break;
    case STATE_SPAWNING: spawn(); break;
    case STATE_RUN_TRANSITION: setState(STATE_RUNNING); break;
    case STATE_RUNNING: run(); break;
    case STATE_DESTROY_TRANSITION: destroyTransition(); break;
    case STATE_DESTROYED: destroy(); break;
    case STATE_INDICATE_TRANSITION: indicateTransition(); break;
    case STATE_INDICATING: indicating(); break;
    case STATE_INACTIVE: break;
    }
}

}
