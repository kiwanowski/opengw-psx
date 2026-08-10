
#include "controls.hh"
#include "fixed.hh"
#include "font.hh"
#include "game.hh"
#include "player.hh"
#include "render.hh"
#include "world.hh"

#include "psyqo/application.hh"
#include "psyqo/font.hh"
#include "psyqo/gpu.hh"
#include "psyqo/scene.hh"

using namespace gw;

namespace {

class OpenGW final : public psyqo::Application {
    void prepare() override;
    void createScene() override;

  public:
    psyqo::Font<> m_font;
    Controls m_controls;
    Player m_player;
    Game m_game;
};

class GameScene final : public psyqo::Scene {
    void start(StartReason reason) override;
    void frame() override;

    void drawHud(Renderer& r);

    uint32_t m_lastFrameCount = 0;

    uint32_t m_fpsFrames = 0;
    uint32_t m_fpsLastVblank = 0;
    int m_fps = 0;
};

static constexpr unsigned kMaxLogicSteps = 3;

void formatScore(char* out, size_t size, int score, int multiplier) {
    const char* digits = font::withCommas(score);

    size_t n = 0;
    auto append = [&](const char* s) {
        while (*s && n + 1 < size) out[n++] = *s++;
    };

    append("SCORE ");
    append(digits);
    if (multiplier > 1) {
        append(" X");
        append(font::withCommas(multiplier));
    }
    out[n] = '\0';
}

void formatDebug(char* out, size_t size, unsigned lines, unsigned overflow, int index, int enemies,
                 int fps) {
    size_t n = 0;
    char num[16];
    auto append = [&](const char* s) {
        while (*s && n + 1 < size) out[n++] = *s++;
    };

    append(font::formatInt(num, sizeof(num), fps));
    append("FPS ");
    append(font::formatInt(num, sizeof(num), int(lines)));
    append("L I");
    append(font::formatInt(num, sizeof(num), index));
    append(" E");
    append(font::formatInt(num, sizeof(num), enemies));
    if (overflow) {
        append(" ");
        append(font::formatInt(num, sizeof(num), int(overflow)));
        append("X");
    }
    out[n] = '\0';
}

void drawShipIcon(Renderer& r, psyqo::Vertex at, const Pen& pen) {
    static constexpr int kIcon[][2] = {{0, -4}, {4, 1}, {2, 3}, {0, 1}, {-2, 3}, {-4, 1}};
    constexpr int count = sizeof(kIcon) / sizeof(kIcon[0]);

    for (int i = 0; i < count; i++) {
        const int(&a)[2] = kIcon[i];
        const int(&b)[2] = kIcon[(i + 1) % count];
        r.screenLine({{.x = int16_t(at.x + a[0]), .y = int16_t(at.y + a[1])}},
                     {{.x = int16_t(at.x + b[0]), .y = int16_t(at.y + b[1])}}, pen);
    }
}

void drawBombIcon(Renderer& r, psyqo::Vertex at, const Pen& pen) {
    static constexpr int kOct[][2] = {{0, -5}, {3, -3}, {5, 0}, {3, 3},
                                      {0, 5},  {-3, 3}, {-5, 0}, {-3, -3}};
    constexpr int count = sizeof(kOct) / sizeof(kOct[0]);

    auto at2 = [&](const int(&p)[2]) {
        return psyqo::Vertex{{.x = int16_t(at.x + p[0]), .y = int16_t(at.y + p[1])}};
    };

    for (int i = 0; i < count; i++) {
        r.screenLine(at2(kOct[i]), at2(kOct[(i + 1) % count]), pen);
    }

    static constexpr int kCross[][2] = {{0, -2}, {0, 2}, {-2, 0}, {2, 0}};
    r.screenLine(at2(kCross[0]), at2(kCross[1]), pen);
    r.screenLine(at2(kCross[2]), at2(kCross[3]), pen);
}

OpenGW g_app;
GameScene g_gameScene;

}

void OpenGW::prepare() {
    psyqo::GPU::Configuration config;
    config.set(psyqo::GPU::Resolution::W320)
        .set(psyqo::GPU::VideoMode::AUTO)
        .set(psyqo::GPU::ColorMode::C15BITS)
        .set(psyqo::GPU::Interlace::PROGRESSIVE);
    gpu().initialize(config);
}

void OpenGW::createScene() {
    m_font.uploadSystemFont(gpu());
    m_controls.init();
    g_world.renderer.init(&gpu());
    pushScene(&g_gameScene);
}

void GameScene::start(StartReason) {
    mathutils::seedRandom(0x1234abcdu);
    g_app.m_game.init(&g_app.m_player);
    m_lastFrameCount = g_app.gpu().getFrameCount();
}

void GameScene::frame() {
    g_app.m_controls.poll();

    if (g_app.m_player.isGameOver() && g_app.m_controls.startPressed()) {
        g_app.m_game.init(&g_app.m_player);
    }

    uint32_t frames = g_app.gpu().getFrameCount();
    unsigned steps = frames - m_lastFrameCount;
    m_lastFrameCount = frames;

    ++m_fpsFrames;
    const unsigned refresh = g_app.gpu().getRefreshRate();
    const uint32_t elapsed = frames - m_fpsLastVblank;
    if (elapsed >= refresh && elapsed > 0) {
        m_fps = int((m_fpsFrames * refresh) / elapsed);
        m_fpsFrames = 0;
        m_fpsLastVblank = frames;
    }

    if (steps < 1) steps = 1;
    if (steps > kMaxLogicSteps) steps = kMaxLogicSteps;

    for (unsigned i = 0; i < steps; i++) {
        g_app.m_game.run(g_app.m_controls);
    }


    Renderer& r = g_world.renderer;
    r.beginFrame();
    g_app.m_game.draw(r);
    drawHud(r);
    r.endFrame();
}

void GameScene::drawHud(Renderer& r) {
    const Player& player = g_app.m_player;

    static constexpr Fixed kHudScale = 5.0L;
    Pen scorePen(0, 255, 0, 200, 1);

    char buf[32];
    formatScore(buf, sizeof(buf), player.score(), player.multiplier());
    font::printScreen(r, font::ALIGN_LEFT, 8, 16, kHudScale, scorePen, buf);

    Pen livesPen(255, 255, 255, 190, 1);
    for (int i = 0; i < player.numLives() && i < 5; i++) {
        psyqo::Vertex at{{.x = int16_t(12 + i * 12), .y = 36}};
        drawShipIcon(r, at, livesPen);
    }

    Pen bombPen(120, 200, 255, 190, 1);
    for (int i = 0; i < player.numBombs() && i < 5; i++) {
        psyqo::Vertex at{{.x = int16_t(kScreenWidth - 14 - i * 12), .y = 36}};
        drawBombIcon(r, at, bombPen);
    }

    Pen debugPen(80, 80, 120, 255, 1);
    char dbg[40];
    formatDebug(dbg, sizeof(dbg), r.lineCount(), r.overflowCount(), g_world.spawner.spawnIndex(),
                g_world.enemies.countActive(), m_fps);
    font::printScreen(r, font::ALIGN_RIGHT, kScreenWidth - 6, 14, 3.0L, debugPen, dbg);

    if (player.isGameOver()) {
        Pen overPen(255, 255, 255, 255, 2);
        font::printScreen(r, font::ALIGN_CENTER, kScreenWidth / 2, 110, 7.0L, overPen, "GAME OVER");
        font::printScreen(r, font::ALIGN_CENTER, kScreenWidth / 2, 140, 4.0L, overPen, "PRESS START");
    }
}

int main() { return g_app.run(); }
