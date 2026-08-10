#include "bomb.hh"

#include "enemies.hh"
#include "world.hh"

namespace gw {

static constexpr Fixed kMaxRadius = 100.0L;

static constexpr Fixed kKillBand = 10.0L;

static constexpr Fixed kGridStrength = 200.0L;

void Bomb::init() {
    for (auto& ring : m_rings) ring.timeToLive = 0;
}

void Bomb::start(const Vec2& pos, Fixed radius, Fixed speed, int timeToLive, const Pen& pen) {
    Ring* slot = nullptr;
    for (auto& ring : m_rings) {
        if (ring.timeToLive <= 0) {
            slot = &ring;
            break;
        }
    }
    if (!slot) return;

    slot->pos = pos;
    slot->radius = radius;
    slot->speed = speed;
    slot->pen = pen;
    slot->timeToLive = int16_t(timeToLive);
    slot->life = int16_t(timeToLive);

    Pen sparks = pen;
    sparks.a = 76;
    g_world.particles.emit(pos, Angle(), speed, Angle(2.0L), 24, sparks, timeToLive, false, false,
                           Fixed(1, 0));
}

void Bomb::run(Enemies& enemies) {
    for (auto& ring : m_rings) {
        if (ring.timeToLive <= 0) continue;

        ring.radius += ring.speed;
        --ring.timeToLive;

        if (ring.radius > kMaxRadius) {
            ring.timeToLive = 0;
            continue;
        }

        if (Attractor* att = g_world.grid.getAttractor()) {
            att->strength = kGridStrength;
            att->radius = ring.radius;
            att->pos = ring.pos;
            att->enabled = true;
            att->attractsParticles = true;
        }

        Fixed inner = ring.radius - kKillBand;
        if (inner.value < 0) inner = Fixed();
        enemies.destroyInShell(ring.pos, inner, ring.radius);
    }
}

void Bomb::draw(Renderer& r) {
    for (const auto& ring : m_rings) {
        if (ring.timeToLive <= 0) continue;

        Pen pen = ring.pen;
        pen.a = uint8_t((ring.pen.a * ring.timeToLive) / (ring.life ? ring.life : 1));

        Vec2 prev = ring.pos + Vec2(ring.radius, Fixed());
        for (int i = 1; i <= kRingSegments; i++) {
            Angle a(int32_t((kTurnRaw * i) / kRingSegments), Angle::RAW);
            Vec2 next = ring.pos + mathutils::fromAngle(a, ring.radius);
            r.line(prev, next, pen);
            prev = next;
        }
    }
}

bool Bomb::isBombing() const {
    for (const auto& ring : m_rings) {
        if (ring.timeToLive > 0) return true;
    }
    return false;
}

}
