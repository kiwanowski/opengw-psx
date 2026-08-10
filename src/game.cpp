#include "game.hh"

#include "world.hh"

namespace gw {

void Game::init(Player* player) {
    m_player = player;
    g_world.player1 = player;

    g_world.grid.init();
    g_world.enemies.init();
    g_world.spawner.init();
    g_world.particles.init();
    g_world.bomb.init();
    g_world.pointDisplays.init();
    g_world.camera.center();

    m_player->initForGame();
    m_player->setEnabled(true);
}

int Game::score() const { return m_player ? m_player->score() : 0; }

void Game::runCollisions() {
    if (!m_player) return;

    PlayerMissile* const* live = m_player->liveMissiles();
    const int liveCount = m_player->liveMissileCount();

    for (int i = 0; i < liveCount; i++) {
        PlayerMissile& m = *live[i];
        if (m.getState() != Entity::STATE_RUNNING) continue;

        Entity* hit = g_world.enemies.hitTestSegment(m.lastPos(), m.getPos(), m.getRadius());
        if (!hit) continue;

        if (hit->getType() == Entity::TYPE_BLACKHOLE) {
            auto* hole = static_cast<BlackHole*>(hit);

            if (!hole->activated()) {
                hole->activate();
            } else if (hole->weaken()) {
                g_world.enemies.explodeEntity(*hole);
                if (int banked = hole->bankedPoints()) {
                    m_player->addKillAtLocation(banked, hole->getPos());
                }
                hole->setState(Entity::STATE_DESTROY_TRANSITION);
            }

            m.setState(Entity::STATE_INACTIVE);
            continue;
        }

        m_player->addKillAtLocation(hit->getScoreValue(), hit->getHitPos());

        if (hit->getType() == Entity::TYPE_SPINNER) {
            g_world.enemies.splitSpinner(*static_cast<Spinner*>(hit), m.getSpeed());
        }

        g_world.enemies.explodeEntity(*hit);

        Pen burst = hit->getPen();
        burst.a = 200;
        g_world.particles.emit(hit->getPos(), Angle(), Fixed(2, 0), Angle(2.0L), 14, burst, 200);

        hit->setState(Entity::STATE_DESTROY_TRANSITION);

        if (Attractor* att = g_world.grid.getAttractor()) {
            att->pos = hit->getPos();
            att->radius = Fixed(12, 0);
            att->strength = Fixed(60, 0);
            att->enabled = true;
        }

        m.setState(Entity::STATE_INACTIVE);
    }

    if (m_player->getState() == Entity::STATE_RUNNING) {
        Entity* hit = g_world.enemies.hitTestAtPosition(m_player->getPos(),
                                                        m_player->getRadius() * Fixed(0.75L));
        if (hit) {
            if (m_player->shielded()) {
                g_world.enemies.explodeEntity(*hit);
                hit->setState(Entity::STATE_DESTROY_TRANSITION);
            } else {
                m_player->killed(hit);
            }
        }
    }
}

void Game::run(const Controls& controls) {
    m_player->runPlayer(controls);
    g_world.enemies.run();

    g_world.enemies.repelMissiles(m_player->liveMissiles(), m_player->liveMissileCount());
    g_world.enemies.runBlackHoles();

    runCollisions();

    g_world.particles.run();
    g_world.pointDisplays.run();
    g_world.bomb.run(g_world.enemies);

    const bool playerActive = (m_player->getState() == Entity::STATE_RUNNING);
    g_world.spawner.run(g_world.enemies, playerActive, g_world.bomb.isBombing());

    g_world.camera.follow(m_player->getPos());
    g_world.camera.run();

    g_world.grid.run();
}

void Game::draw(Renderer& r) {
    r.setCamera(g_world.camera.currentPos, g_world.camera.currentZoom);

    g_world.grid.draw(r);
    g_world.particles.draw(r);
    g_world.enemies.draw(r);
    m_player->draw(r);
    g_world.bomb.draw(r);

    g_world.pointDisplays.draw(r);
}

}
