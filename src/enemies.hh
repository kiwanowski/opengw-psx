
#pragma once

#include "entity.hh"

namespace gw {

class Player;

static constexpr int kMaxGrunts = 64;
static constexpr int kMaxWanderers = 32;
static constexpr int kMaxWeavers = 32;
static constexpr int kMaxSpinners = 20;
static constexpr int kMaxTinySpinners = 32;
static constexpr int kMaxMayflies = 48;
static constexpr int kMaxProtons = 24;
static constexpr int kMaxSnakes = 6;
static constexpr int kMaxBlackHoles = 4;
static constexpr int kMaxRepulsors = 2;

static constexpr int kMaxLineDebris = 64;

class Grunt final : public Entity {
  public:
    Grunt();
    void run() override;
    void spawnTransition() override;

  private:
    Angle m_animationIndex;
};

class Wanderer final : public Entity {
  public:
    Wanderer();
    void run() override;
    void spawnTransition() override;

  private:
    Angle m_currentHeading;
    bool m_flipped = false;
};

class Weaver final : public Entity {
  public:
    Weaver();
    void run() override;
    void spawnTransition() override;
};

class Spinner final : public Entity {
  public:
    Spinner();
    void run() override;
    void spawnTransition() override;

    void setHitSpeed(const Vec2& speed) { m_hitSpeed = speed; }
    const Vec2& hitSpeed() const { return m_hitSpeed; }
    void clearHitSpeed() { m_hitSpeed = Vec2(); }

  private:
    Angle m_animationIndex;
    Vec2 m_hitSpeed;
};

class TinySpinner final : public Entity {
  public:
    TinySpinner();
    void run() override;
    void spawnTransition() override;
    void spawn() override;
    void draw(Renderer& r) override;

    Vec2 getHitPos() const override { return m_virtualPos; }

    void setInitialSpeed(const Vec2& speed) { m_initialSpeed = speed; }

  private:
    Angle m_animationIndex;
    Vec2 m_virtualPos;
    Vec2 m_initialSpeed;
};

class Mayfly final : public Entity {
  public:
    Mayfly();
    void run() override;
    void spawnTransition() override;

  private:
    Vec2 m_target;
    int16_t m_flipTimer = 0;
    int8_t m_flipDirection = 1;
};

class Proton final : public Entity {
  public:
    Proton();
    void run() override;
    void spawnTransition() override;
};

static constexpr int kSnakeSegments = 12;

static constexpr int kSnakeStreamItems = 5;

class Snake final : public Entity {
  public:
    Snake();

    void run() override;
    void spawnTransition() override;
    void draw(Renderer& r) override;

    bool overlaps(const Vec2& pos, Fixed radius) const override;

    Fixed boundingRadius() const override;

    bool overlapsSegment(const Vec2& from, const Vec2& to, Fixed radius) const override;

  private:
    struct Segment {
        Vec2 streamPos[kSnakeStreamItems];
        Angle streamAngle[kSnakeStreamItems];
        Vec2 pos;
        Angle angle;
        Vec2 scale;
        Fixed tail;
    };

    void updateTarget();

    Vec2 m_target;
    Segment m_segments[kSnakeSegments];
};

static constexpr int kRepulsorShields = 3;

class BlackHole final : public Entity {
  public:
    BlackHole();

    void run() override;
    void spawnTransition() override;
    void draw(Renderer& r) override;

    Fixed ringRadius() const;

    Fixed absorbRadius() const;

    Fixed boundingRadius() const override { return absorbRadius(); }

    void activate();

    void feed(int points);

    bool weaken();

    bool activated() const { return m_activated; }

    int bankedPoints() const { return m_points; }

    bool shouldExplode() const { return m_strength > kBurstStrength; }

  private:
    static constexpr Fixed kBurstStrength = 2.3L;

    static constexpr Fixed kCollapseStrength = 0.7L;

    int m_points = 0;

    bool m_activated = false;
    Fixed m_strength = Fixed(1, 0);
    Fixed m_balance;
    Fixed m_balanceRate;
    Angle m_animationIndex;
    Angle m_gridPullIndex;
};

class Repulsor final : public Entity {
  public:
    Repulsor();

    void run() override;
    void spawnTransition() override;
    void draw(Renderer& r) override;

    void repelEntity(Entity& e) const;

  private:
    enum AIState : uint8_t { State_Thinking = 0, State_Aiming, State_Charging };

    AIState m_aiState = State_Thinking;
    int16_t m_timer = 0;
    bool m_shieldsEnabled = false;
    Angle m_shieldPhase;
};

class LineDebris final : public Entity {
  public:
    LineDebris();
    void run() override;
    void draw(Renderer& r) override;

    void launch(const Vec2& from, const Vec2& to, const Vec2& drift, const Pen& pen);

  private:
    Vec2 m_to;
    int16_t m_life = 0;
};

class Enemies {
  public:
    void init();

    void run();
    void draw(Renderer& r);

    Entity* getUnusedEnemyOfType(Entity::Type type);

    Entity* hitTestAtPosition(const Vec2& pos, Fixed radius);

    Entity* hitTestSegment(const Vec2& from, const Vec2& to, Fixed radius);

    void destroyInShell(const Vec2& center, Fixed innerRadius, Fixed outerRadius);

    int countActiveOfType(Entity::Type type) const;
    int countActive() const;

    int poolSize(Entity::Type type) const;

    bool anyAliveFromWave(uint8_t waveId) const;

    void explodeEntity(Entity& e);

    void splitSpinner(Spinner& spinner, const Vec2& missileSpeed);

    void runBlackHoles();

    void repelMissiles(class PlayerMissile* const* missiles, int count) const;

    void disableAll();

    void destroyAllWithExplosion(Entity* spare = nullptr);

    static constexpr int kTotalPoolSlots = kMaxGrunts + kMaxWanderers + kMaxWeavers +
                                           kMaxSpinners + kMaxTinySpinners + kMaxMayflies +
                                           kMaxProtons + kMaxSnakes + kMaxBlackHoles +
                                           kMaxRepulsors;

  private:
    struct ActiveEnemy {
        Entity* entity;
        Vec2 hitPos;
        Fixed radius;
    };

    void rebuildActiveList();

    template <typename Fn>
    void forEachPool(Fn&& fn);

    ActiveEnemy m_active[kTotalPoolSlots];
    int m_activeCount = 0;

    Grunt m_grunts[kMaxGrunts];
    Wanderer m_wanderers[kMaxWanderers];
    Weaver m_weavers[kMaxWeavers];
    Spinner m_spinners[kMaxSpinners];
    TinySpinner m_tinySpinners[kMaxTinySpinners];
    Mayfly m_mayflies[kMaxMayflies];
    Proton m_protons[kMaxProtons];
    Snake m_snakes[kMaxSnakes];
    BlackHole m_blackHoles[kMaxBlackHoles];
    Repulsor m_repulsors[kMaxRepulsors];
    LineDebris m_debris[kMaxLineDebris];
};

}
