#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"

// Students:  Add code to this file, Actor.cpp, StudentWorld.h, and StudentWorld.cpp

const int TOTAL_TICKS_JUMP = 4;
const int EXTRA_LIFE_POINTS = 50;
const int GARLIC_POINTS = 25;
const int NUM_EXTRA_BURPS = 5;
const int TICK_FACTOR = 10;
const int KONG_TICK_FACTOR = 5;
const int MAX_MOD_FACTOR = 100; // should be divisible by all other factors. Prevents nTicks from exceeding integer limits
const int SAMPLE_DENOMINATOR = 3; // sample with 1/3 chance
const int NUM_FREEZE_TICKS = 50;
const int FREEZE_COOLDOWN_TICKS = 50;
const int FINISH_LEVEL_SCORE = 1000;

// Climbing states
const int STATE_NOT_CLIMBING = 0;
const int STATE_CLIMBING_UP = 1;
const int STATE_CLIMBING_DOWN = 2;

class StudentWorld;

class Actor : public GraphObject { // class should never be instantiated -> base class for all actors
public:
	Actor(StudentWorld* sw, int imageID, int startX, int startY, int startDirection = none);
	virtual ~Actor() = 0; // pure virtual destructor to make the class an Abstract Base Class
	virtual void doSomething(); // does nothing, usually overwritten
	bool alive() const; // getter for m_isAlive
	bool blastable() const; // getter for m_isBlastable
	bool burnable() const; // getter for m_isBurnable
	bool passable() const; // getter for m_isPassable
	bool climbable() const; // getter for m_isClimbable
	virtual void kill(); // Actor can either kill itself or be killed. This function can be overwritten for custom kill() methods
protected:
	StudentWorld* getWorld() const; // getter for m_world
	void setPassable(bool b); // setter for m_isPassable
	void setClimbable(bool b); // setter for m_isClimbable
	void setBlastable(bool b); // setter for m_isBlastable
	void setBurnable(bool b); // setter for m_isBurnable
	void incTicks(); // increments ticks withing doSomething(), for functions which require tick tracking
	bool checkModMTick(int m); // checks the condition m_nTicks mod m == 0, m is determined based on Actor type
	bool tryMoveTo(int xx, int yy); // safe function to move to (xx, yy). If cannot move due to wall or out of bounds, returns false
	bool tryMoveInDirection(); // safe function to move in current direction by one step. If cannot move due to wall or out of bounds, returns false
private:
	StudentWorld* m_world; // reference to StudentWorld object which manages all Actors
	bool m_isAlive; // flag to check whether actor is alive
	bool m_isPassable; // flag to check whether player can pass through object, default true
	bool m_isClimbable; // flag to check whether player can climb object (i.e. ladder), default false
	bool m_isBlastable; // flag to check whether object can be destroyed by burp
	bool m_isBurnable; // flag to check whether object can be destroyed by bonfire
	int m_nTicks; // tracks the number of ticks (i.e. calls to doSomething()) since Actor is created
};

class Player : public Actor { // movable player controlled by keyboard
public:
	Player(StudentWorld* sw, int startX, int startY, int startDirection);
	virtual void doSomething(); // player controls
	virtual void kill(); // player dies
	void increaseBurps(int k); // increases number of burps. Controlled by StudentWorld
	int getBurps() const; // getter for number of burps
	void freeze(); // freezes the player for NUM_FREEZE_TICKS
private:
	int m_nBurps; // Current number of burps available. Starts at 0.
	int m_nJumpTicks; // Starts from 4, goes to 0. Indicates the number of substeps left in the jump routine. Starts at 0.
	int m_freezeCounter; // Number of ticks left to be frozen
};

class Floor : public Actor { // Actor has no behavior, inherits doSomething() from Actor
public:
	Floor(StudentWorld* sw, int startX, int startY); // m_isPassable is set to false
};

class Ladder : public Actor { // Actor has no behavior, inherits doSomething() from Actor
public:
	Ladder(StudentWorld* sw, int startX, int startY); // m_is_Climbable is set to true
};

class Burp : public Actor { // Burp created by player dynamically
public:
	Burp(StudentWorld* sw, int startX, int startY, int startDirection);
	virtual void doSomething(); // track lifetime and attack player if on same square
private:
	int m_lifetime; // initially 5, decrements with each doSomething()
};

class Enemy : public Actor {
public:
	Enemy(StudentWorld* sw, int imageID, int startX, int startY, int startDirection = none);
	virtual ~Enemy() = 0; // pure virtual destructor to make the class an Abstract Base Class
	virtual void doSomething(); // Attacks player if same square. This is called by most child functions.
	bool checkCliffInDirection() const; // Check if in that direction, if player takes one more step, if the player will fall under gravity
	void toggleDirection(); // toggles between left and right
	void lateralMove(); // moves left and right along a path with endpoints of either walls or cliffs
};

class Bonfire : public Enemy {
public:
	Bonfire(StudentWorld* sw, int startX, int startY);
	virtual void doSomething(); // attack player or barrel if on same square, as well as manage animations
};

class Fireball : public Enemy {
public:
	Fireball(StudentWorld* sw, int startX, int startY);
	virtual void doSomething(); // manages movement of fireball, attacks player if on same square
	virtual void kill(); // in addition to inherited kill(), it also drops garlic goodie with probability 1/3
private:
	int m_climbingState; // checks if fireball is currently not climbing, climbing up or climbing down
};

class Koopa : public Enemy {
public:
	Koopa(StudentWorld* sw, int startX, int startY);
	virtual void doSomething(); // controls movement, freeze player if conditions met. Decrements cooldown.
	virtual void kill(); // in addition to inherited kill(), it also drops extra life goodie with probability 1/3
private:
	int m_freezeCooldown; // tracks ticks left before cooldown expires
	bool tryFreezePlayer(); // Tries to freeze player if same square and not in cooldown mode. Resets cooldown. Returns true if successfully frozen, false otherwise.
};

class Barrel : public Enemy {
public:
	Barrel(StudentWorld* sw, int startX, int startY, int startDirection);
	virtual void doSomething(); // controls movement, kills player if same square
};

class Goodie : public Actor { // base class for all goodies
public:
	Goodie(StudentWorld* sw, int imageID, int startX, int startY);
	virtual ~Goodie() = 0; // pure virtual destructor to make the class an Abstract Base Class
	virtual void doSomething(); // manages common functionality for all goodies (sound, scope, destruction)
	void setExtraPoints(int p); // setter for extra points for collecting goodie
	int extraPoints() const; // getter for extra points for collecting goodie
private:
	int m_extraPoints; // extra points for collecting goodie

};

class ExtraLifeGoodie : public Goodie {
public:
	ExtraLifeGoodie(StudentWorld* sw, int startX, int startY);
	virtual void doSomething(); // increment lives by 1 in addition to inherited doSomething()
};

class GarlicGoodie : public Goodie {
public:
	GarlicGoodie(StudentWorld* sw, int startX, int startY);
	virtual void doSomething(); // increment burps by 5 in addition to inherited doSomething()
};

class Kong : public Actor {
public:
	Kong(StudentWorld* sw, int startX, int startY, int startDirection);
	virtual void doSomething(); // manage animation, manage flee behavior, check level finish condition
private:
	bool m_flee; // checks if player is close enough to Kong to flee. Initially set to false
};

#endif // ACTOR_H_
