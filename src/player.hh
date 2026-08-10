
#pragma once

#include "controls.hh"
#include "entity.hh"

namespace gw {

static constexpr int kMaxMissiles = 96;

static constexpr int kPlayerShieldTime = 250;

static constexpr int kRespawnDelay = 50;

class PlayerMissile final : public Entity {
  public:
    PlayerMissile();

    void run() override;
    void spawnTransition() override;
    void draw(Renderer& r) override;

    const Vec2& lastPos() const { return m_lastPos; }

    void setWeaponType(uint8_t type) { m_weaponType = type; }

    Fixed m_velocity;

  private:
    Vec2 m_lastPos;
    uint8_t m_weaponType = 0;
};

class Player final : public Entity {
  public:
    Player();

    void initForGame();

    void runPlayer(const Controls& controls);

    void draw(Renderer& r) override;

    void run() override;
    void spawnTransition() override;

    int score() const { return m_score; }
    int multiplier() const { return m_multiplier; }
    int numLives() const { return m_numLives; }
    int numBombs() const { return m_numBombs; }

    bool shielded() const { return m_shieldTimer > 0; }

    bool isGameOver() const { return m_numLives <= 0 && !getEnabled() && m_respawnTimer == 0; }

    void killed(Entity* killer = nullptr);

    void destroyTransition() override;

    void destroy() override;

    void addKillAtLocation(int points, const Vec2& pos);

    void addPoints(int points) { m_score += points * m_multiplier; }

    PlayerMissile* missiles() { return m_missiles; }

    PlayerMissile* const* liveMissiles() const { return m_live; }
    int liveMissileCount() const { return m_liveCount; }

  private:
    void firePattern1(Angle aim, const Vec2& playerSpeed);

    void firePattern2(Angle aim, const Vec2& playerSpeed);

    void firePattern3(Angle aim, const Vec2& playerSpeed);

    PlayerMissile* launchMissile(Angle posAngle, Angle velAngle, Fixed offset, Fixed speed,
                                 const Vec2& playerSpeed);

    void switchWeapons();

    void emitExhaust();

    PlayerMissile* getFreeMissile();

    void runMissiles();

    PlayerMissile m_missiles[kMaxMissiles];

    PlayerMissile* m_live[kMaxMissiles];
    int m_liveCount = 0;

    Pen m_exhaustPen;
    Pen m_missilesPen;
    Pen m_fontPen;

    int m_score = 0;
    int m_multiplier = 1;
    int m_numLives = 5;
    int m_numBombs = 5;

    int m_killCounter = 0;

    int m_currentWeapon = 0;

    int m_weaponCounter = 0;

    bool m_alternate = false;

    Angle m_exhaustSpreadIndex;

    int m_lifeCounter = 0;
    int m_bombCounter = 0;

    int16_t m_firingTimer = 0;
    int16_t m_shieldTimer = kPlayerShieldTime;
    int16_t m_bombInterimTimer = 0;

    int16_t m_respawnTimer = 0;

    Entity* m_killer = nullptr;
    bool m_drawShield = false;
};

}
