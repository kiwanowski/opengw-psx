
#pragma once

#include <stdint.h>

#include "fixed.hh"
#include "render.hh"

namespace gw {

static constexpr int kGridSpacing = 4;

static constexpr int kGridExtentX = 33 * 4 + 1;
static constexpr int kGridExtentY = 22 * 4 + 1;

static constexpr int kGridPointsX = (kGridExtentX - 1) / kGridSpacing + 1;
static constexpr int kGridPointsY = (kGridExtentY - 1) / kGridSpacing + 1;

struct Attractor {
    Vec2 pos;
    Fixed strength;
    Fixed radius;
    bool enabled = false;
    bool attractsParticles = false;
};

static constexpr int kMaxAttractors = 32;

class Grid {
  public:
    void init();

    void run();

    void draw(Renderer& r);

    Attractor* getAttractor();
    void clearAttractors();

    bool hitTest(const Vec2& pos, Fixed radius, Vec2* hitPos = nullptr, Vec2* speed = nullptr);

    static constexpr int extentX() { return kGridExtentX; }
    static constexpr int extentY() { return kGridExtentY; }

    Fixed brightness = Fixed(1, 0);

  private:
    struct Point {
        Vec2 pos;
        Vec2 vel;
        bool atRest = true;
    };

    Point& at(int x, int y) { return m_points[y * kGridPointsX + x]; }

    void applyAttractors();

    Point m_points[kGridPointsX * kGridPointsY];
    Attractor m_attractors[kMaxAttractors];
};

}
