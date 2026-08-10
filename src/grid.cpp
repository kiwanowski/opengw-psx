#include "grid.hh"

namespace gw {

static constexpr Fixed kDt = 0.3L;
static constexpr Fixed kAccel = -3.6L;
static constexpr Fixed kDamping = 0.637628L;

static constexpr Fixed kAttractorScale = 0.005L;

void Grid::init() {
    for (int y = 0; y < kGridPointsY; y++) {
        for (int x = 0; x < kGridPointsX; x++) {
            Point& p = at(x, y);
            p.pos = Vec2(Fixed(x * kGridSpacing, 0), Fixed(y * kGridSpacing, 0));
            p.vel = Vec2();
            p.atRest = true;
        }
    }
    clearAttractors();
}

Attractor* Grid::getAttractor() {
    for (auto& a : m_attractors) {
        if (!a.enabled) return &a;
    }
    return nullptr;
}

void Grid::clearAttractors() {
    for (auto& a : m_attractors) a.enabled = false;
}

void Grid::applyAttractors() {
    for (auto& att : m_attractors) {
        if (!att.enabled) continue;

        int radiusCells = (att.radius.value >> 12) / kGridSpacing + 1;
        int cx = (att.pos.x.value >> 12) / kGridSpacing;
        int cy = (att.pos.y.value >> 12) / kGridSpacing;

        int x0 = cx - radiusCells, x1 = cx + radiusCells;
        int y0 = cy - radiusCells, y1 = cy + radiusCells;

        if (x0 < 1) x0 = 1;
        if (y0 < 1) y0 = 1;
        if (x1 > kGridPointsX - 2) x1 = kGridPointsX - 2;
        if (y1 > kGridPointsY - 2) y1 = kGridPointsY - 2;

        const Fixed radiusSq = att.radius * att.radius;

        for (int y = y0; y <= y1; y++) {
            for (int x = x0; x <= x1; x++) {
                Point& p = at(x, y);
                Vec2 delta = att.pos - p.pos;
                Fixed distSq = delta.lengthSquared();

                if (distSq >= radiusSq || distSq.value <= 0) continue;

                Fixed dist = gw::approxLength(delta.x, delta.y);
                if (dist.value == 0) continue;

                Vec2 push = delta * (att.strength * kAttractorScale * dist);
                p.vel -= push;

                p.atRest = false;
            }
        }

        att.enabled = false;
    }
}

void Grid::run() {
    applyAttractors();

    for (int y = 1; y < kGridPointsY - 1; y++) {
        for (int x = 1; x < kGridPointsX - 1; x++) {
            Point& p = at(x, y);

            if (p.atRest && at(x - 1, y).atRest && at(x + 1, y).atRest &&
                at(x, y - 1).atRest && at(x, y + 1).atRest) {
                continue;
            }

            const Vec2& p1 = at(x - 1, y).pos;
            const Vec2& p2 = at(x + 1, y).pos;
            const Vec2& p3 = at(x, y - 1).pos;
            const Vec2& p4 = at(x, y + 1).pos;

            Vec2 avg((p1.x + p2.x + p3.x + p4.x) / 4, (p1.y + p2.y + p3.y + p4.y) / 4);

            p.vel += (p.pos - avg) * kAccel;
            p.vel *= kDamping;
            p.pos += p.vel * kDt;

            if (p.pos.x.value < 0) p.pos.x = Fixed();
            else if (p.pos.x > Fixed(kGridExtentX - 1, 0)) p.pos.x = Fixed(kGridExtentX - 1, 0);
            if (p.pos.y.value < 0) p.pos.y = Fixed();
            else if (p.pos.y > Fixed(kGridExtentY - 1, 0)) p.pos.y = Fixed(kGridExtentY - 1, 0);

            static constexpr int32_t kRestEpsilon = 16;
            auto near = [](Fixed v, int32_t target) {
                int32_t d = v.value - target;
                return (d < 0 ? -d : d) < kRestEpsilon;
            };

            p.atRest = near(p.vel.x, 0) && near(p.vel.y, 0) &&
                       near(p.pos.x, (x * kGridSpacing) << 12) &&
                       near(p.pos.y, (y * kGridSpacing) << 12);
        }
    }
}

void Grid::draw(Renderer& r) {
    if (brightness <= Fixed(0.05L)) return;

    uint8_t alpha = uint8_t((Fixed(102, 0) * brightness).value >> 12);
    Pen pen(102, 102, 255, alpha, 1);

    Vec2 viewMin, viewMax;
    r.visibleBounds(viewMin, viewMax);

    auto toCell = [](Fixed v, int margin) {
        return int(v.value >> 12) / kGridSpacing + margin;
    };

    int x0 = toCell(viewMin.x, -1), x1 = toCell(viewMax.x, 1);
    int y0 = toCell(viewMin.y, -1), y1 = toCell(viewMax.y, 1);

    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > kGridPointsX - 1) x1 = kGridPointsX - 1;
    if (y1 > kGridPointsY - 1) y1 = kGridPointsY - 1;

    psyqo::Vertex strip[kStripPoints];

    for (int y = 0; y < kGridPointsY; y++) {
        if (y < y0 || y > y1) continue;
        for (int base = 0; base + kStripSegments < kGridPointsX; base += kStripSegments) {
            for (int i = 0; i < kStripPoints; i++) {
                strip[i] = r.project(at(base + i, y).pos);
            }
            r.screenStrip(strip, kStripPoints, pen);
        }
    }

    for (int x = 0; x < kGridPointsX; x++) {
        if (x < x0 || x > x1) continue;
        for (int base = 0; base + kStripSegments < kGridPointsY; base += kStripSegments) {
            for (int i = 0; i < kStripPoints; i++) {
                strip[i] = r.project(at(x, base + i).pos);
            }
            r.screenStrip(strip, kStripPoints, pen);
        }
    }

    uint8_t borderAlpha = uint8_t((Fixed(255, 0) * brightness).value >> 12);
    Pen borderPen(255, 255, 255, borderAlpha, 2);

    Vec2 bl(Fixed(0, 0), Fixed(0, 0));
    Vec2 br(Fixed(kGridExtentX - 1, 0), Fixed(0, 0));
    Vec2 tr(Fixed(kGridExtentX - 1, 0), Fixed(kGridExtentY - 1, 0));
    Vec2 tl(Fixed(0, 0), Fixed(kGridExtentY - 1, 0));

    r.line(bl, br, borderPen);
    r.line(br, tr, borderPen);
    r.line(tr, tl, borderPen);
    r.line(tl, bl, borderPen);
}

bool Grid::hitTest(const Vec2& pos, Fixed radius, Vec2* hitPos, Vec2* speed) {
    bool hit = false;

    if (hitPos) *hitPos = pos;

    const Fixed left = radius;
    const Fixed bottom = radius;
    const Fixed right = Fixed(kGridExtentX, 0) - radius;
    const Fixed top = Fixed(kGridExtentY, 0) - radius;

    if (pos.x < left) {
        if (hitPos) hitPos->x = left;
        if (speed) speed->x = -speed->x;
        hit = true;
    } else if (pos.x > right - Fixed(1, 0)) {
        if (hitPos) hitPos->x = right - Fixed(1, 0);
        if (speed) speed->x = -speed->x;
        hit = true;
    }

    if (pos.y < bottom) {
        if (hitPos) hitPos->y = bottom;
        if (speed) speed->y = -speed->y;
        hit = true;
    } else if (pos.y > top - Fixed(1, 0)) {
        if (hitPos) hitPos->y = top - Fixed(1, 0);
        if (speed) speed->y = -speed->y;
        hit = true;
    }

    return hit;
}

}
