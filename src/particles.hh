
#pragma once

#include <stdint.h>

#include "fixed.hh"
#include "render.hh"

namespace gw {

static constexpr int kMaxParticles = 96;

static constexpr int kTrailLength = 4;

class Particles {
  public:
    void init();

    void run();
    void draw(Renderer& r);

    void emit(const Vec2& position, Angle angle, Fixed speed, Angle spread, int num,
              const Pen& color, int timeToLive, bool gravity = true, bool gridBound = true,
              Fixed drag = Fixed(0.93L), unsigned alphaScale = 1);

    void killAll();

    int activeCount() const;

  private:
    struct Particle {
        Vec2 trail[kTrailLength];
        Vec2 speed;
        Pen color;
        uint16_t alphaScaled;
        Fixed drag;
        int16_t timeToLive;
        bool gravity;
        bool gridBound;
    };

    void assign(const Vec2& position, const Vec2& speed, int timeToLive, const Pen& color,
                bool gravity, bool gridBound, Fixed drag, unsigned alphaScale);

    Particle m_particles[kMaxParticles];

    uint16_t m_index = 0;
};

}
