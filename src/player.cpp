#include "player.hh"

#include "font.hh"
#include "models.hh"
#include "world.hh"

namespace gw {

static constexpr Angle kQuarterTurn = 0.5L;

static constexpr Fixed kThrust = 0.6L;
static constexpr Fixed kStickMin = 0.1L;
static constexpr Fixed kStickFull = 0.6L;
static constexpr Fixed kTurnRate = 0.2L;


static constexpr int kMissileAttractorsPerFrame = 6;
static int s_missileAttractorBudget = 0;

PlayerMissile::PlayerMissile() {
    m_type = TYPE_PLAYER_MISSILE;
    m_model = &models::missile;
    m_scale = Vec2(Fixed(0.16L), Fixed(0.16L));
    m_radius = Fixed(1, 0);
    m_spawnTime = 0;
    m_destroyTime = 0;
    m_gridBound = false;
    m_state = STATE_INACTIVE;
}

void PlayerMissile::spawnTransition() {
    setState(STATE_RUNNING);
}

void PlayerMissile::run() {
    m_lastPos = m_pos;
    m_pos += m_speed;

    Vec2 hit;
    if (g_world.grid.hitTest(m_pos, m_radius, &hit, nullptr)) {
        setState(STATE_INACTIVE);
        ++m_genId;
        return;
    }

    if (s_missileAttractorBudget == 0) return;
    --s_missileAttractorBudget;

    if (Attractor* att = g_world.grid.getAttractor()) {
        static constexpr Fixed kPull[3] = {Fixed(10, 0), Fixed(20, 0), Fixed(30, 0)};
        att->strength = kPull[m_weaponType < 3 ? m_weaponType : 0];
        att->radius = Fixed(5, 0);
        att->pos = m_pos;
        att->enabled = true;
        att->attractsParticles = false;
    }
}

void PlayerMissile::draw(Renderer& r) {
    if (m_state != STATE_RUNNING || !m_model) return;
    m_model->draw(r, m_pos, m_angle, m_scale, m_pen);
}


Player::Player() {
    m_type = TYPE_PLAYER1;
    m_model = &models::player;
    m_scale = Vec2(Fixed(1.7L), Fixed(1.7L));
    m_radius = Fixed(2, 0);
    m_spawnTime = 40;
    m_destroyTime = 40;
    m_gridBound = true;

    m_pen = Pen(255, 255, 255, 255, 2);
    m_exhaustPen = Pen(230, 255, 89, 255, 1);
    m_missilesPen = Pen(230, 255, 89, 255, 1);
    m_fontPen = Pen(255, 255, 0, 255, 1);

    for (auto& m : m_missiles) m.setPen(m_missilesPen);

    initForGame();
}

void Player::initForGame() {
    m_numBombs = 5;
    m_numLives = 5;
    m_score = 0;
    m_multiplier = 1;
    m_killCounter = 0;
    m_currentWeapon = 0;
    m_weaponCounter = 0;
    m_lifeCounter = 0;
    m_bombCounter = 0;
    m_firingTimer = 0;
    m_shieldTimer = kPlayerShieldTime;
    m_bombInterimTimer = 0;
    m_respawnTimer = 0;

    m_pos = Vec2(Fixed(Grid::extentX(), 0) / 2, Fixed(Grid::extentY(), 0) / 2);
    m_speed = Vec2();
    m_drift = Vec2();
    m_angle = Angle();

    for (auto& m : m_missiles) m.setState(STATE_INACTIVE);
}

void Player::spawnTransition() {
    Entity::spawnTransition();
    m_shieldTimer = kPlayerShieldTime;
}

static constexpr int kKillsPerMultiplier = 25;
static constexpr int kPointsPerLife = 75000;
static constexpr int kPointsPerBomb = 100000;
static constexpr int kPointsPerWeapon = 10000;

static constexpr int kMaxMultiplier = 6;

void Player::addKillAtLocation(int points, const Vec2& pos) {
    const int pointsEarned = points * m_multiplier;

    m_score += pointsEarned;
    m_weaponCounter += pointsEarned;
    m_lifeCounter += pointsEarned;
    m_bombCounter += pointsEarned;

    if (m_weaponCounter >= kPointsPerWeapon) {
        m_weaponCounter = 0;
        switchWeapons();
    }

    if (m_bombCounter >= kPointsPerBomb) {
        m_bombCounter = 0;
        ++m_numBombs;
    }
    if (m_lifeCounter >= kPointsPerLife) {
        m_lifeCounter = 0;
        ++m_numLives;
    }

    bool showMultiplier = false;
    if (++m_killCounter >= kKillsPerMultiplier) {
        m_killCounter = 0;
        if (m_multiplier < kMaxMultiplier) {
            ++m_multiplier;
            showMultiplier = true;
        }
    }

    char message[kPointMessageLen];
    if (showMultiplier) {
        int n = 0;
        for (const char* s = "MULTIPLIER X"; *s && n < kPointMessageLen - 1; s++) message[n++] = *s;
        font::formatInt(message + n, kPointMessageLen - n, m_multiplier);
    } else {
        font::formatInt(message, kPointMessageLen, pointsEarned);
    }

    g_world.pointDisplays.show(pos, m_fontPen, message);
}

void Player::killed(Entity* killer) {
    if (m_state == STATE_DESTROYED || m_state == STATE_DESTROY_TRANSITION) return;
    m_killer = killer;
    setState(STATE_DESTROY_TRANSITION);
}

void Player::destroyTransition() {
    setState(STATE_DESTROYED);
    m_stateTimer = m_destroyTime;

    m_multiplier = 1;

    if (Attractor* att = g_world.grid.getAttractor()) {
        att->strength = Fixed(200, 0);
        att->radius = Fixed(30, 0);
        att->pos = m_pos;
        att->enabled = true;
        att->attractsParticles = true;
    }

    Pen debris = m_pen;
    debris.a = 200;
    g_world.particles.emit(m_pos, Angle(), Fixed(2, 0), Angle(2.0L), 32, debris, 200, true, true,
                           Fixed(0.97L));

    g_world.enemies.destroyAllWithExplosion(m_killer);
    m_killer = nullptr;

    for (auto& m : m_missiles) m.setState(STATE_INACTIVE);
}

void Player::destroy() {
    if (--m_stateTimer > 0) return;

    setState(STATE_INACTIVE);

    if (--m_numLives <= 0) {
        m_numLives = 0;
        m_respawnTimer = 0;
        return;
    }

    m_respawnTimer = kRespawnDelay;
}

PlayerMissile* Player::getFreeMissile() {
    for (auto& m : m_missiles) {
        if (!m.getEnabled()) return &m;
    }
    return nullptr;
}

static constexpr Fixed kExhaustSpeed = 0.8L;
static constexpr Angle kExhaustSpread = 0.0318L;
static constexpr Fixed kMainNozzle = -2.0L;
static constexpr Fixed kSwirlNozzle = -3.0L;
static constexpr Angle kSwirlSweep = 0.0955L;
static constexpr Angle kSwirlRate = 0.0573L;

static constexpr unsigned kExhaustOverbright = 100;

static constexpr int kExhaustLife = 60;

void Player::emitExhaust() {
    Pen exhaust = m_exhaustPen;
    exhaust.a = 255;

    const Angle back = m_angle + Angle(1.0L);
    const Angle forward = m_angle + kQuarterTurn;

    Vec2 nozzle = m_pos + mathutils::fromAngle(forward, kMainNozzle);
    g_world.particles.emit(nozzle, back, kExhaustSpeed, kExhaustSpread, 1, exhaust, kExhaustLife,
                           true, true, Fixed(0.92L), kExhaustOverbright);

    Fixed sweep = gw::sin(m_exhaustSpreadIndex);
    Angle offset = Angle(int32_t((Fixed(kSwirlSweep.value, Fixed::RAW) * sweep).value), Angle::RAW);

    for (int i = 0; i < 2; i++) {
        Angle swirl = forward + (i == 0 ? offset : -offset);
        Vec2 swirlNozzle = m_pos + mathutils::fromAngle(swirl, kSwirlNozzle);
        g_world.particles.emit(swirlNozzle, back, kExhaustSpeed, Angle(), 1, exhaust, kExhaustLife,
                               true, true, Fixed(0.92L), kExhaustOverbright);
    }

    m_exhaustSpreadIndex = wrapAngle(m_exhaustSpreadIndex + kSwirlRate);
}

PlayerMissile* Player::launchMissile(Angle posAngle, Angle velAngle, Fixed offset, Fixed speed,
                                     const Vec2& playerSpeed) {
    PlayerMissile* m = getFreeMissile();
    if (!m) return nullptr;

    m->setPos(m_pos + mathutils::fromAngle(posAngle, offset));
    m->setSpeed(mathutils::fromAngle(velAngle, speed) + playerSpeed * Fixed(0.5L));
    m->setAngle(velAngle - kQuarterTurn);
    m->m_velocity = speed;
    m->setWeaponType(uint8_t(m_currentWeapon));
    m->setState(STATE_SPAWN_TRANSITION);
    return m;
}

void Player::switchWeapons() {
    if (m_currentWeapon == 0) {
        m_currentWeapon = 1;
    } else {
        m_currentWeapon = (mathutils::randFromTo(0, 99) < 50) ? 1 : 2;
    }
}

void Player::firePattern1(Angle aim, const Vec2& playerSpeed) {
    if (--m_firingTimer > 0) return;
    m_firingTimer = 6;

    static constexpr Fixed kSpeed = 1.19L;
    static constexpr Angle kSpread = 0.1273L;
    static constexpr Fixed kOffset = 2.0L;

    launchMissile(aim + kSpread, aim, kOffset, kSpeed, playerSpeed);
    launchMissile(aim - kSpread, aim, kOffset, kSpeed, playerSpeed);
}

void Player::firePattern2(Angle aim, const Vec2& playerSpeed) {
    if (--m_firingTimer > 0) return;

    m_alternate = !m_alternate;
    m_firingTimer = m_alternate ? 4 : 1;

    static constexpr Fixed kSpeed = 2.04L;
    static constexpr Angle kSpread = 0.2546L;
    static constexpr Angle kAimOffset = 0.0191L;
    static constexpr Fixed kOffset = 0.5L;

    if (m_alternate) {
        launchMissile(aim + kSpread, aim + kAimOffset, kOffset, kSpeed, playerSpeed);
        launchMissile(aim - kSpread, aim - kAimOffset, kOffset, kSpeed, playerSpeed);
    } else {
        launchMissile(aim, aim, kOffset, kSpeed, playerSpeed);
    }
}

void Player::firePattern3(Angle aim, const Vec2& playerSpeed) {
    if (--m_firingTimer > 0) return;
    m_firingTimer = 7;

    static constexpr Fixed kSpeed = 1.53L;
    static constexpr Fixed kOffset = 2.0L;

    static constexpr Angle kStart1 = 0.0318L;
    static constexpr Angle kStart2 = 0.0477L;
    static constexpr Angle kSpread1 = 0.0159L;
    static constexpr Angle kSpread2 = 0.0286L;

    launchMissile(aim, aim, kOffset, kSpeed, playerSpeed);
    launchMissile(aim + kStart1, aim + kSpread1, kOffset, kSpeed, playerSpeed);
    launchMissile(aim - kStart1, aim - kSpread1, kOffset, kSpeed, playerSpeed);
    launchMissile(aim + kStart2, aim + kSpread2, kOffset, kSpeed, playerSpeed);
    launchMissile(aim - kStart2, aim - kSpread2, kOffset, kSpeed, playerSpeed);
}

void Player::runMissiles() {
    s_missileAttractorBudget = kMissileAttractorsPerFrame;

    m_liveCount = 0;
    for (auto& m : m_missiles) {
        if (!m.getEnabled()) continue;
        m.tick();
        if (m.getEnabled()) m_live[m_liveCount++] = &m;
    }
}

void Player::run() {
    Entity::run();
}

void Player::runPlayer(const Controls& controls) {
    Vec2 playerSpeed;

    switch (m_state) {
    case STATE_SPAWN_TRANSITION: spawnTransition(); break;
    case STATE_SPAWNING: spawn(); break;
    case STATE_RUN_TRANSITION: setState(STATE_RUNNING); break;
    case STATE_DESTROY_TRANSITION: destroyTransition(); break;
    case STATE_DESTROYED: destroy(); break;

    case STATE_INACTIVE:
        if (m_respawnTimer > 0 && --m_respawnTimer == 0) {
            m_pos = Vec2(Fixed(Grid::extentX(), 0) / 2, Fixed(Grid::extentY(), 0) / 2);
            m_speed = Vec2();
            m_drift = Vec2();
            setState(STATE_SPAWN_TRANSITION);
        }
        break;

    default: break;
    }

    if (m_state == STATE_RUNNING) {
        if (controls.trigger() && m_numBombs > 0 && m_bombInterimTimer <= 0) {
            --m_numBombs;
            m_bombInterimTimer = 50;
            g_world.bomb.start(m_pos, Fixed(1, 0), Fixed(2, 0), 200, Pen(255, 255, 255, 76, 2));
        }
        if (m_bombInterimTimer > 0) --m_bombInterimTimer;

        Vec2 left = controls.leftStick();
        Fixed distance = left.length();

        if (distance > kStickMin) {
            Fixed throttle = (distance > kStickFull) ? Fixed(1, 0) : Fixed(0.5L);

            Angle heading = gw::atan2(left.y, left.x);
            Angle target = heading - kQuarterTurn;

            Angle diff = diffAngles(target, m_angle);
            setAngle(m_angle + scaleAngle(diff, kTurnRate));

            Vec2 thrust = mathutils::fromAngle(m_angle + kQuarterTurn, throttle * kThrust);
            playerSpeed = thrust;
            m_pos += thrust;

            emitExhaust();
        }

        Vec2 right = controls.rightStick();
        if (right.length() > kStickMin) {
            Angle aim = gw::atan2(right.y, right.x);
            switch (m_currentWeapon) {
            case 1: firePattern2(aim, playerSpeed); break;
            case 2: firePattern3(aim, playerSpeed); break;
            default: firePattern1(aim, playerSpeed); break;
            }
        }
    }

    runMissiles();

    m_drawShield = false;
    if (m_shieldTimer > 0) --m_shieldTimer;

    m_drawShield = (m_shieldTimer < 60) ? ((m_shieldTimer / 6) & 1) : true;

    if (m_drawShield) {
        if (Attractor* att = g_world.grid.getAttractor()) {
            att->strength = Fixed(100, 0);
            att->radius = Fixed(3.5L);
            att->pos = m_pos;
            att->enabled = true;
            att->attractsParticles = false;
        }
    }

    if (m_state == STATE_RUNNING) Entity::run();
}

void Player::draw(Renderer& r) {
    for (int i = 0; i < m_liveCount; i++) m_live[i]->draw(r);

    if (!getEnabled() || m_state == STATE_DESTROYED || m_state == STATE_DESTROY_TRANSITION) return;

    Entity::draw(r);

    if (m_drawShield) {
        Pen shieldPen(120, 180, 255, 160, 1);
        static constexpr int kSegments = 12;
        Fixed radius(3.5L);
        Vec2 prev = m_pos + Vec2(radius, Fixed());
        for (int i = 1; i <= kSegments; i++) {
            Angle a(int32_t((kTurnRaw * i) / kSegments), Angle::RAW);
            Vec2 next = m_pos + mathutils::fromAngle(a, radius);
            r.line(prev, next, shieldPen);
            prev = next;
        }
    }
}

}
