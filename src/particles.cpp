#include "particles.hh"

#include "world.hh"

namespace gw {

static constexpr Angle kQuarterTurn = 0.5L;

static constexpr Fixed kMaxSpeed = 2.0L;

static constexpr Fixed kSpeedToAlpha = 0.8L;

static constexpr int kMinAlpha = 13;

static constexpr int kAlphaFalloff = 26;

void Particles::init() { killAll(); }

void Particles::killAll() {
    for (auto& p : m_particles) p.timeToLive = 0;
    m_index = 0;
}

int Particles::activeCount() const {
    int n = 0;
    for (const auto& p : m_particles) {
        if (p.timeToLive > 0) n++;
    }
    return n;
}

void Particles::assign(const Vec2& position, const Vec2& speed, int timeToLive, const Pen& color,
                       bool gravity, bool gridBound, Fixed drag, unsigned alphaScale) {
    Particle& p = m_particles[m_index++];
    if (m_index >= kMaxParticles) m_index = 0;

    Vec2 pos = position;
    if (gridBound) {
        Vec2 hitPoint;
        if (g_world.grid.hitTest(pos, Fixed(), &hitPoint)) pos = hitPoint;
    }

    for (int i = 0; i < kTrailLength; i++) p.trail[i] = pos;

    p.speed = speed;
    p.color = color;
    unsigned scaled = unsigned(color.a) * alphaScale;
    p.alphaScaled = uint16_t(scaled > 0xffffu ? 0xffffu : scaled);
    p.drag = drag;
    p.gravity = gravity;
    p.gridBound = gridBound;

    p.timeToLive = int16_t((Fixed(timeToLive, 0) * mathutils::frand()).value >> 12);
}

void Particles::emit(const Vec2& position, Angle angle, Fixed speed, Angle spread, int num,
                     const Pen& color, int timeToLive, bool gravity, bool gridBound, Fixed drag,
                     unsigned alphaScale) {
    for (int i = 0; i < num; i++) {
        Angle jitter = spread.value ? Angle(int32_t(mathutils::random32() % uint32_t(spread.value)), Angle::RAW) - spread / 2
                                    : Angle();
        Angle dir = angle + jitter + kQuarterTurn;

        Vec2 velocity = mathutils::fromAngle(dir, speed * mathutils::frand());
        assign(position, velocity, timeToLive, color, gravity, gridBound, drag, alphaScale);
    }
}

void Particles::run() {
    for (auto& p : m_particles) {
        if (p.timeToLive <= 0) continue;

        if (--p.timeToLive <= 0) {
            p.timeToLive = 0;
            continue;
        }

        p.speed *= p.drag;
        p.speed = mathutils::clamp(p.speed, kMaxSpeed);

        Vec2 pos = p.trail[0] + p.speed;

        if (p.gridBound) {
            Vec2 hitPoint;
            Vec2 speed = p.speed;
            if (g_world.grid.hitTest(pos, Fixed(), &hitPoint, &speed)) {
                pos = hitPoint;
                p.speed = speed;
            }
        }

        for (int i = kTrailLength - 2; i >= 0; i--) p.trail[i + 1] = p.trail[i];
        p.trail[0] = pos;
    }
}

void Particles::draw(Renderer& r) {
    for (auto& p : m_particles) {
        if (p.timeToLive <= 0) continue;

        Fixed speedNormal = p.speed.length();
        int alpha = int((Fixed(p.alphaScaled, 0) * speedNormal * kSpeedToAlpha).value >> 12);

        if (alpha < kMinAlpha) {
            p.timeToLive = 0;
            continue;
        }
        if (alpha > 255) alpha = 255;

        Pen pen = p.color;
        int midpoint = alpha - kAlphaFalloff * (kTrailLength - 2);
        pen.a = uint8_t(midpoint > 0 ? midpoint : alpha);

        psyqo::Vertex strip[kTrailLength];
        for (int i = 0; i < kTrailLength; i++) strip[i] = r.project(p.trail[i]);
        r.screenStrip(strip, kTrailLength, pen);
    }
}

}
