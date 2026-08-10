
#pragma once

#include <stdint.h>

#include "fixed.hh"
#include "render.hh"

namespace gw {

struct Edge {
    uint8_t from;
    uint8_t to;
};

struct Model {
    const Vec2* vertices;
    const Edge* edges;
    uint8_t vertexCount;
    uint8_t edgeCount;

    void draw(Renderer& r, const Vec2& pos, Angle angle, Vec2 scale, const Pen& pen) const;

    void drawStrip(Renderer& r, const Vec2& pos, Angle angle, Vec2 scale, const Pen& pen) const;

    Vec2 transform(const Vec2& v, const Vec2& pos, Fixed sinA, Fixed cosA, const Vec2& scale) const {
        Vec2 s(v.x * scale.x, v.y * scale.y);
        return Vec2(pos.x + s.x * cosA - s.y * sinA, pos.y + s.x * sinA + s.y * cosA);
    }
};

}
