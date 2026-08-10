#include "render.hh"

#include "psyqo/primitives/control.hh"
#include "psyqo/primitives/misc.hh"

namespace gw {

static constexpr Fixed kCotHalfFov = 1.428148L;

void Renderer::init(psyqo::GPU* gpu) {
    m_gpu = gpu;
    m_parity = 0;
    m_count = 0;
    m_overflow = 0;
}

void Renderer::beginFrame() {
    m_parity ^= 1;
    m_count = 0;
    m_stripCount = 0;
    m_overflow = 0;
}

void Renderer::endFrame() {
    if (m_count == 0 && m_stripCount == 0) return;

    ClearFragment& clear = m_clearFragment[m_parity];
    m_gpu->getNextClear(clear.primitive, {{.r = 0, .g = 0, .b = 0}});
    m_gpu->chain(clear);

    ModeFragment& mode = m_modeFragment[m_parity];
    mode.primitive.attr.set(psyqo::Prim::TPageAttr::FullBackAndFullFront).setDithering(false);
    m_gpu->chain(mode);

    unsigned remaining = m_count;
    for (unsigned chunk = 0; remaining > 0; chunk++) {
        unsigned used = remaining < kLinesPerChunk ? remaining : kLinesPerChunk;
        LineChunk& batch = m_chunks[m_parity][chunk];
        batch.count = used;
        m_gpu->chain(batch);
        remaining -= used;
    }

    for (unsigned i = 0; i < m_stripCount; i++) {
        m_gpu->chain(m_strips[m_parity][i]);
    }
}

void Renderer::setCamera(const Vec2& center, Fixed zoom) {
    m_center = center;
    if (zoom.value <= 0) zoom = Fixed(1, 0);
    m_scale = (kCotHalfFov * kScreenHeight) / (zoom * 2);
}

void Renderer::emit(psyqo::Vertex a, psyqo::Vertex b, psyqo::Color color, uint8_t width) {
    if (width == 0) width = 1;

    int dx = b.x - a.x;
    int dy = b.y - a.y;
    bool steep = (dx < 0 ? -dx : dx) < (dy < 0 ? -dy : dy);

    for (uint8_t i = 0; i < width; i++) {
        if (m_count >= kMaxLines) {
            m_overflow++;
            return;
        }

        int offset = (i + 1) / 2;
        if ((i & 1) == 0) offset = -offset;

        LineChunk& chunk = m_chunks[m_parity][m_count >> kChunkShift];
        psyqo::Prim::Line& prim = chunk.primitives[m_count & kChunkMask];
        m_count++;
        prim.setColor(color);
        prim.setSemiTrans();
        if (steep) {
            prim.pointA = psyqo::Vertex{{.x = int16_t(a.x + offset), .y = a.y}};
            prim.pointB = psyqo::Vertex{{.x = int16_t(b.x + offset), .y = b.y}};
        } else {
            prim.pointA = psyqo::Vertex{{.x = a.x, .y = int16_t(a.y + offset)}};
            prim.pointB = psyqo::Vertex{{.x = b.x, .y = int16_t(b.y + offset)}};
        }
    }
}

void Renderer::line(const Vec2& from, const Vec2& to, const Pen& pen) {
    if (pen.a == 0) return;
    psyqo::Vertex a = project(from);
    psyqo::Vertex b = project(to);
    if (cullSegment(a, b)) return;
    emit(a, b, pen.resolved(), pen.width);
}

void Renderer::screenLine(psyqo::Vertex from, psyqo::Vertex to, const Pen& pen) {
    if (pen.a == 0) return;
    if (cullSegment(from, to)) return;
    emit(from, to, pen.resolved(), pen.width);
}

void Renderer::screenStrip(const psyqo::Vertex* points, int count, const Pen& pen) {
    if (pen.a == 0 || count < 2) return;
    if (count > kStripPoints) count = kStripPoints;

    if (m_stripCount >= kMaxStrips) {
        m_overflow++;
        return;
    }

    bool allLeft = true, allRight = true, allAbove = true, allBelow = true;
    for (int i = 0; i < count; i++) {
        const psyqo::Vertex& p = points[i];
        if (p.x >= 0) allLeft = false;
        if (p.x < kScreenWidth) allRight = false;
        if (p.y >= 0) allAbove = false;
        if (p.y < kScreenHeight) allBelow = false;
    }
    if (allLeft || allRight || allAbove || allBelow) return;

    StripFragment& frag = m_strips[m_parity][m_stripCount++];

    frag.command = 0x48000000 | 0x02000000 | (pen.resolved().packed & 0x00ffffff);
    frag.pointCount = unsigned(count);

    for (int i = 0; i < count; i++) frag.points[i] = points[i];

    frag.points[count].packed = 0x50005000;
}

}
