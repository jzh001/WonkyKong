#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "Level.h"
#include "Actor.h"
#include <string>
#include <vector>

// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp

const int ENEMY_DIE_POINTS = 100;
const int MIN_EUCLID_DISTANCE = 2;

class StudentWorld : public GameWorld
{
public:
  StudentWorld(std::string assetPath);
  ~StudentWorld();
  virtual int init();
  virtual int move();
  virtual void cleanUp();
  bool checkPassable(int xx, int yy) const; // check if square has walls
  bool checkClimbable(int xx, int yy) const; // check if square has ladders
  void addActor(Actor* ap); // add an object of base class Actor to the vector m_actors
  void attackEnemy(int xx, int yy, bool isBonfire = false); // attack (i.e. kill) all enemies in square (xx, yy). If isBonfire is set to true, we only attack barrels.
  void attackPlayer(); // kill the player
  void freezePlayer(); // freeze the player
  bool isPlayerLocation(int xx, int yy); // check if player is at square (xx, yy)
  void increaseBurps(int k); // increment player's number of burps by k
  bool checkPlayerAlive() const; // checks if player is still alive
  void dropGarlic(int xx, int yy); // adds Garlic Goodie Actor to m_actors
  void dropExtraLife(int xx, int yy); // adds ExtraLife Goodie Actor to m_actors
  bool closeToPlayer(int xx, int yy); // check if (xx,yy) is less than the euclidean distance MIN_EUCLID_DISTANCE from player
  void setLevelComplete(); // setter function for m_levelComplete

private:
	Player* m_player;
	Level* m_level;
	std::vector<Actor* > m_actors;
	bool m_levelComplete; // initially set to false
	int loadLevel(); // helper function to load level from file
	void updateDisplayText(); // sets the game stats text
	int checkGameStatus(); // returns player died, finished level or continue game
};

#endif // STUDENTWORLD_H_
