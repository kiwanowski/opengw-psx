#include "font.hh"

#include "font_data.hh"

namespace gw {
namespace font {

static constexpr Fixed kGlyphSqueeze = 0.65L;
static constexpr Fixed kLowercaseHeight = 0.6L;
static constexpr Fixed kUppercaseLift = 0.4L;

static const Glyph* glyphFor(char c) {
    if (c < kFirstChar || c > kLastChar) return nullptr;
    const Glyph& g = kGlyphs[c - kFirstChar];
    return g.segments ? &g : nullptr;
}

Fixed measure(const char* text, Fixed scale) {
    int count = 0;
    for (const char* p = text; *p && *p != '\n'; p++) count++;
    return Fixed(count, 0) * kCellWidth * scale;
}

static Fixed alignOrigin(Align align, Fixed x, const char* text, Fixed scale) {
    if (align == ALIGN_LEFT) return x;

    Fixed width = measure(text, scale);
    if (align == ALIGN_RIGHT) return x - width;

    return x - width / 2 + (kCellWidth * scale) / 2;
}

template <typename Emit>
static void layout(Align align, Fixed x, Fixed y, Fixed scale, const char* text, Emit&& emit) {
    Fixed cursorX = alignOrigin(align, x, text, scale);
    Fixed cursorY = y;

    for (const char* p = text; *p; p++) {
        char c = *p;

        if (c == '\n') {
            cursorX = alignOrigin(align, x, p + 1, scale);
            cursorY -= kCellHeight * scale;
            continue;
        }

        const Glyph* g = glyphFor(c);
        if (g) {
            bool lower = (c >= 'a' && c <= 'z');
            Fixed heightScale = scale * (lower ? kLowercaseHeight : Fixed(1, 0));
            Fixed lift = lower ? Fixed() : scale * kUppercaseLift;

            const Vec2* verts = &kFontVerts[g->offset];
            for (int i = 0; i < g->segments; i++) {
                const Vec2& a = verts[i * 2];
                const Vec2& b = verts[i * 2 + 1];

                emit(Vec2(cursorX + a.x * scale * kGlyphSqueeze, cursorY + a.y * heightScale + lift),
                     Vec2(cursorX + b.x * scale * kGlyphSqueeze, cursorY + b.y * heightScale + lift));
            }
        }

        cursorX += kCellWidth * scale;
    }
}

void printWorld(Renderer& r, Align align, const Vec2& pos, Fixed scale, const Pen& pen,
                const char* text) {
    layout(align, pos.x, pos.y, scale, text,
           [&](const Vec2& a, const Vec2& b) { r.line(a, b, pen); });
}

void printScreen(Renderer& r, Align align, int x, int y, Fixed scale, const Pen& pen,
                 const char* text) {
    layout(align, Fixed(x, 0), Fixed(), scale, text, [&](const Vec2& a, const Vec2& b) {
        psyqo::Vertex va{{.x = int16_t(a.x.value >> 12), .y = int16_t(y - (a.y.value >> 12))}};
        psyqo::Vertex vb{{.x = int16_t(b.x.value >> 12), .y = int16_t(y - (b.y.value >> 12))}};
        r.screenLine(va, vb, pen);
    });
}

char* formatInt(char* out, int size, int value) {
    char digits[12];
    int n = 0;
    bool negative = value < 0;
    unsigned v = negative ? unsigned(-value) : unsigned(value);

    if (v == 0) digits[n++] = '0';
    for (; v > 0; v /= 10) digits[n++] = char('0' + (v % 10));
    if (negative) digits[n++] = '-';

    int i = 0;
    while (n > 0 && i < size - 1) out[i++] = digits[--n];
    out[i] = '\0';
    return out;
}

const char* withCommas(int value) {
    static char out[16];

    char digits[16];
    int n = 0;
    bool negative = value < 0;
    unsigned v = negative ? unsigned(-value) : unsigned(value);

    if (v == 0) digits[n++] = '0';
    for (int group = 0; v > 0; v /= 10) {
        if (group == 3) {
            digits[n++] = ',';
            group = 0;
        }
        digits[n++] = char('0' + (v % 10));
        group++;
    }
    if (negative) digits[n++] = '-';

    int i = 0;
    while (n > 0 && i < int(sizeof(out)) - 1) out[i++] = digits[--n];
    out[i] = '\0';
    return out;
}

}
}
