#include "Actor.h"
#include "StudentWorld.h"
#include "GameConstants.h"

#include <algorithm>

// Students:  Add code to this file, Actor.h, StudentWorld.h, and StudentWorld.cpp

bool checkIndex(int xx, int yy); // checks if index is valid based on grid size
bool sampleChance(); // returns true randomly 1/3 of the time

Actor::Actor(StudentWorld* sw, int imageID, int startX, int startY, int startDirection)
: GraphObject(imageID, startX, startY, startDirection), m_isAlive(true), m_world(sw), 
m_isPassable(true), m_isClimbable(false), m_isBlastable(false), m_isBurnable(false), m_nTicks(0) {

}

Actor::~Actor() {
	// do nothing
}

bool Actor::passable() const {
	return m_isPassable;
}

bool Actor::climbable() const {
	return m_isClimbable;
}

bool Actor::blastable() const {
	return m_isBlastable;
}

bool Actor::alive() const{
	return m_isAlive;
}

bool Actor::burnable() const {
	return m_isBurnable;
}

void Actor::kill() {
	m_isAlive = false;
}

bool Actor::tryMoveTo(int xx, int yy) {
	// returns true if success
	if (m_world->checkPassable(xx, yy)) {
		moveTo(xx, yy);
		return true;
	}
	return false;
}

void Actor::incTicks() {
	m_nTicks++;
	m_nTicks %= MAX_MOD_FACTOR; // to ensure m_nTicks does not exceed integer limits
}

bool Actor::checkModMTick(int m) {
	return m_nTicks % m == 0;
}


StudentWorld* Actor::getWorld() const {
	return m_world;
}

void Actor::doSomething() {
	// do nothing by default
}

bool Actor::tryMoveInDirection() {
	if (getDirection() == left) return tryMoveTo(getX() - 1, getY());
	else return tryMoveTo(getX() + 1, getY()); // direction == right
}

void Actor::setPassable(bool b) {
	m_isPassable = b;
}

void Actor::setClimbable(bool b) {
	m_isClimbable = b;
}

void Actor::setBlastable(bool b) {
	m_isBlastable = b;
}

void Actor::setBurnable(bool b) {
	m_isBurnable = b;
}

Player::Player(StudentWorld* sw, int startX, int startY, int startDirection) 
: Actor(sw, IID_PLAYER, startX, startY, startDirection), m_nBurps(0), m_nJumpTicks(0), m_freezeCounter(0) {

}

void Player::doSomething() {
	if (!alive()) return;
	if (m_nJumpTicks > 0) {
		if (getWorld()->checkClimbable(getX(), getY())) {
			// If new position is on climbable surface, terminate the jump sequence (eg on ladder)
			m_nJumpTicks = 0;
		}
		else { // execute normal jump sequence step by step
			m_nJumpTicks--;
			switch (m_nJumpTicks) {
			case 3: case 2: case 1:
				// move in direction D
				if (!tryMoveInDirection()) m_nJumpTicks = 0;
				break;
			case 0:
				// move down
				tryMoveTo(getX(), getY() - 1);
				break;
			}
			
		}
		
	}
	else if (!(getWorld()->checkClimbable(getX(), getY())) && !(getWorld()->checkClimbable(getX(), getY() - 1)) && (getWorld()->checkPassable(getX(), getY() - 1))) {
		tryMoveTo(getX(), getY() - 1); // fall under gravity if no ladder or floor
	}
	else if (m_freezeCounter > 0) {
		// frozen
		m_freezeCounter--;
	}
	else { // read user input
		int ch;
		if (getWorld()->getKey(ch)) {
			switch (ch) {
			case KEY_PRESS_LEFT: // move left
				if (getDirection() == right) setDirection(left);
				else tryMoveTo(getX() - 1, getY());
				break;
			case KEY_PRESS_RIGHT: // move right
				if (getDirection() == left) setDirection(right);
				else tryMoveTo(getX() + 1, getY());
				break;
			case KEY_PRESS_SPACE: // jump
				if (tryMoveTo(getX(), getY() + 1)) m_nJumpTicks = 4;
				getWorld()->playSound(SOUND_JUMP);
				break;
			case KEY_PRESS_UP: // climb ladder up
				if (getWorld()->checkClimbable(getX(), getY())) tryMoveTo(getX(), getY() + 1);
				break;
			case KEY_PRESS_DOWN: // climb ladder down
				if (getWorld()->checkClimbable(getX(), getY()) || getWorld()->checkClimbable(getX(), getY() - 1)) tryMoveTo(getX(), getY() - 1);
				break;
			case KEY_PRESS_TAB: // burp if able to
				if (m_nBurps > 0) {
					int x = getX();
					int y = getY();
					if (getDirection() == left) x--;
					else x++;

					if (checkIndex(x, y)) {
						getWorld()->addActor(new Burp(getWorld(), x, y, getDirection()));
						m_nBurps--;
					}
				}
					
				break;
			}
		}

	}
}

void Player::kill() {
	if ((alive())) {
		Actor::kill();
		getWorld()->decLives();
	}
}

void Player::increaseBurps(int k) {
	m_nBurps += k;
}

int Player::getBurps() const {
	return m_nBurps;
}

void Player::freeze(){
	m_freezeCounter += NUM_FREEZE_TICKS;
}

Floor::Floor(StudentWorld* sw, int startX, int startY)
: Actor(sw, IID_FLOOR, startX, startY) {
	setPassable(false);
}

Ladder::Ladder(StudentWorld* sw, int startX, int startY)
	: Actor(sw, IID_LADDER, startX, startY) {
	setClimbable(true);
}

Burp::Burp(StudentWorld* sw, int startX, int startY, int startDirection)
	: Actor(sw, IID_BURP, startX, startY, startDirection), m_lifetime(5){
	sw->playSound(SOUND_BURP);
}

void Burp::doSomething() {
	if (m_lifetime > 0) m_lifetime--;
	if (m_lifetime == 0) {
		kill();
		return;
	}
	getWorld()->attackEnemy(getX(), getY());
}

Enemy::Enemy(StudentWorld* sw, int imageID, int startX, int startY, int startDirection)
	: Actor(sw, imageID, startX, startY, startDirection) {
	setBlastable(true);
}

Enemy::~Enemy() {}

void Enemy::doSomething() {
	if (getWorld()->isPlayerLocation(getX(), getY())) getWorld()->attackPlayer();
}

bool Enemy::checkCliffInDirection() const {
	if (getDirection() == left) return (getWorld()->checkPassable(getX() - 1, getY() - 1) && !(getWorld()->checkClimbable(getX() - 1, getY() - 1)));
	else return (getWorld()->checkPassable(getX() + 1, getY() - 1) && !(getWorld()->checkClimbable(getX() + 1, getY() - 1)));
}

void Enemy::toggleDirection() {
	if (getDirection() == left) setDirection(right);
	else setDirection(left);
}

void Enemy::lateralMove() {
	if (checkCliffInDirection() || !tryMoveInDirection()) toggleDirection(); // move in original direction, unless blocked by cliff or wall, then reverse direction
}

Bonfire::Bonfire(StudentWorld* sw, int startX, int startY) 
	: Enemy(sw, IID_BONFIRE, startX, startY) {

}

void Bonfire::doSomething() {
	increaseAnimationNumber();
	getWorld()->attackEnemy(getX(), getY(), true); // set to true to attack burnable items (i.e. barrel)
	Enemy::doSomething(); // kill player if same square
}

Fireball::Fireball(StudentWorld* sw, int startX, int startY)
	: Enemy(sw, IID_FIREBALL, startX, startY), m_climbingState(STATE_NOT_CLIMBING) {
	if (randInt(0, 1) == 0) setDirection(left);
	else setDirection(right);
}

void Fireball::doSomething() {
	incTicks();
	if (!alive()) return;
	Enemy::doSomething(); // kill player if same square
	if (!(getWorld()->checkPlayerAlive())) return;
	if (!checkModMTick(TICK_FACTOR)) return;
	bool hasMoved = false;
	if (m_climbingState != STATE_CLIMBING_DOWN && getWorld()->checkClimbable(getX(), getY()) && getWorld()->checkPassable(getX(), getY() + 1)) {
		if (m_climbingState == STATE_CLIMBING_UP || sampleChance()) { 
			// climb up ladder, by following original motion or randomly deciding if originally not climbing
			m_climbingState = STATE_CLIMBING_UP;
			tryMoveTo(getX(), getY() + 1);
			hasMoved = true;
		}
	}
	else if (m_climbingState != STATE_CLIMBING_UP && getWorld()->checkClimbable(getX(), getY() - 1)){
		if (m_climbingState == STATE_CLIMBING_DOWN || sampleChance()) {
			// climb down ladder, by following original motion or randomly deciding if originally not climbing
			m_climbingState = STATE_CLIMBING_DOWN;
			tryMoveTo(getX(), getY() - 1);
			hasMoved = true;
		}
	}
	if (!hasMoved) {
		// One action at a time. If action executed previously, we skip the body of the if statement
		if ((m_climbingState == STATE_CLIMBING_UP && (!(getWorld()->checkPassable(getX(), getY() + 1))
			|| !(getWorld()->checkClimbable(getX(), getY())))) ||
			(m_climbingState == STATE_CLIMBING_DOWN && (!(getWorld()->checkPassable(getX(), getY() - 1)) ||
				!(getWorld()->checkClimbable(getX(), getY() - 1))))) {
			m_climbingState = STATE_NOT_CLIMBING;
		}
		lateralMove(); // move left and right
	}
	Enemy::doSomething(); // kill player if same square
}

void Fireball::kill() {
	Actor::kill();
	if (sampleChance()) getWorld()->dropGarlic(getX(), getY());
}

Koopa::Koopa(StudentWorld* sw, int startX, int startY)
	: Enemy(sw, IID_KOOPA, startX, startY), m_freezeCooldown(0){
	if (randInt(0, 1) == 0) setDirection(left);
	else setDirection(right);
}

void Koopa::doSomething() {
	incTicks();
	if (!alive()) return;
	if (tryFreezePlayer()) return;
	if (m_freezeCooldown > 0) m_freezeCooldown--;
	if (checkModMTick(TICK_FACTOR)) lateralMove();
	if (tryFreezePlayer()) return;
}

bool Koopa::tryFreezePlayer() {
	if (m_freezeCooldown == 0 && getWorld()->isPlayerLocation(getX(), getY())) {
		getWorld()->freezePlayer();
		m_freezeCooldown = FREEZE_COOLDOWN_TICKS;
		return true;
	}
	return false;
}

void Koopa::kill() {
	Actor::kill();
	if (sampleChance()) getWorld()->dropExtraLife(getX(), getY());
}

Barrel::Barrel(StudentWorld* sw, int startX, int startY, int startDirection)
	: Enemy(sw, IID_BARREL, startX, startY, startDirection) {
	setBurnable(true);
}

void Barrel::doSomething() {
	incTicks();
	if (!alive()) return;
	Enemy::doSomething(); // kill player if same square
	if (!(getWorld()->checkPlayerAlive())) return;

	if (getWorld()->checkPassable(getX(), getY() - 1)) {
		tryMoveTo(getX(), getY() - 1); // fall under gravity
		if (!(getWorld()->checkPassable(getX(), getY() - 1))) toggleDirection(); // reverse direction upon landing on the ground
	}

	if (checkModMTick(TICK_FACTOR)) { // once every 10 ticks
		if (!tryMoveInDirection()) toggleDirection(); // change direction if encounter wall. Continue moving in same direction if encounter cliff.
	}
	Enemy::doSomething(); // kill player if same square
}

Goodie::Goodie(StudentWorld* sw, int imageID, int startX, int startY)
	: Actor(sw, imageID, startX, startY), m_extraPoints(0) {

}

Goodie::~Goodie() {}

void Goodie::doSomething() {
	if (alive() && getWorld()->isPlayerLocation(getX(), getY())) {
		getWorld()->playSound(SOUND_GOT_GOODIE);
		getWorld()->increaseScore(m_extraPoints);
		kill(); // Goodie disappears
	}
}

void Goodie::setExtraPoints(int p) {
	m_extraPoints = p;
}

int Goodie::extraPoints() const {
	return m_extraPoints;
}

ExtraLifeGoodie::ExtraLifeGoodie(StudentWorld* sw, int startX, int startY)
	: Goodie(sw, IID_EXTRA_LIFE_GOODIE, startX, startY) {
	setExtraPoints(EXTRA_LIFE_POINTS);
}

void ExtraLifeGoodie::doSomething() {
	if (!alive()) return;
	if (getWorld()->isPlayerLocation(getX(), getY())) getWorld()->incLives();
	Goodie::doSomething();
}

GarlicGoodie::GarlicGoodie(StudentWorld* sw, int startX, int startY)
	: Goodie(sw, IID_GARLIC_GOODIE, startX, startY) {
	setExtraPoints(GARLIC_POINTS);
}

void GarlicGoodie::doSomething() {
	if (!alive()) return;
	if (getWorld()->isPlayerLocation(getX(), getY())) getWorld()->increaseBurps(NUM_EXTRA_BURPS);
	Goodie::doSomething();
}

Kong::Kong(StudentWorld* sw, int startX, int startY, int startDirection) 
	: Actor(sw, IID_KONG, startX, startY, startDirection), m_flee(false) {

}

void Kong::doSomething() {
	incTicks();
	if (!alive()) return;
	increaseAnimationNumber();
	if (getWorld()->closeToPlayer(getX(), getY())) m_flee = true;

	int N = std::max(200 - 50 * getWorld()->getLevel(), 50); // N

	if (!m_flee && checkModMTick(N)) { // create barrels
		int x = getX();
		int y = getY();
		if (getDirection() == left) x--;
		else x++;

		if (checkIndex(x, y)) getWorld()->addActor(new Barrel(getWorld(), x, y, getDirection()));
	}

	if (checkModMTick(KONG_TICK_FACTOR) && m_flee) {
		tryMoveTo(getX(), getY() + 1); // try moving up if m_flee
		if (getY() == VIEW_HEIGHT - 1) { // Condition for finishing level
			getWorld()->increaseScore(FINISH_LEVEL_SCORE);
			getWorld()->playSound(SOUND_FINISHED_LEVEL);
			getWorld()->setLevelComplete();
		}
	}
}

// ===== Helper Functions =====

bool sampleChance() {
	if (randInt(0, SAMPLE_DENOMINATOR - 1) == 0) return true;
	return false;
}