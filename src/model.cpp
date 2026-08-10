#include "model.hh"

namespace gw {

template <typename Transform>
static void emitEdges(Renderer& r, const Model& model, const Pen& pen, Transform&& transformPoint) {
    psyqo::Vertex strip[kStripPoints];
    int count = 0;
    uint8_t previousTo = 0;

    for (uint8_t i = 0; i < model.edgeCount; i++) {
        const Edge& e = model.edges[i];

        if (count > 0 && e.from == previousTo && count < kStripPoints) {
            strip[count++] = transformPoint(e.to);
        } else {
            if (count >= 2) r.screenStrip(strip, count, pen);
            count = 0;
            strip[count++] = transformPoint(e.from);
            strip[count++] = transformPoint(e.to);
        }
        previousTo = e.to;
    }

    if (count >= 2) r.screenStrip(strip, count, pen);
}

void Model::draw(Renderer& r, const Vec2& pos, Angle angle, Vec2 scale, const Pen& pen) const {
    Fixed sinA = gw::sin(angle);
    Fixed cosA = gw::cos(angle);

    emitEdges(r, *this, pen, [&](uint8_t index) {
        return r.project(transform(vertices[index], pos, sinA, cosA, scale));
    });
}

void Model::drawStrip(Renderer& r, const Vec2& pos, Angle angle, Vec2 scale, const Pen& pen) const {
    draw(r, pos, angle, scale, pen);
}

}
