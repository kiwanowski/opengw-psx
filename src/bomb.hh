
#pragma once

#include <stdint.h>

#include "fixed.hh"
#include "render.hh"

namespace gw {

class Enemies;

static constexpr int kMaxBombRings = 4;

static constexpr int kRingSegments = 32;

class Bomb {
  public:
    void init();

    void start(const Vec2& pos, Fixed radius, Fixed speed, int timeToLive, const Pen& pen);

    void run(Enemies& enemies);
    void draw(Renderer& r);

    bool isBombing() const;

  private:
    struct Ring {
        Vec2 pos;
        Fixed radius;
        Fixed speed;
        Pen pen;
        int16_t timeToLive = 0;
        int16_t life = 0;
    };

    Ring m_rings[kMaxBombRings];
};

}
