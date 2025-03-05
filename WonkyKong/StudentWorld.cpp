#include "StudentWorld.h"
#include "GameConstants.h"
#include "GraphObject.h"
#include "Actor.h"
#include "Level.h"
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
using namespace std;

string num2string(int x, int digits);
bool checkIndex(int xx, int yy);

GameWorld* createStudentWorld(string assetPath)
{
    return new StudentWorld(assetPath);
}

// Students:  Add code to this file, StudentWorld.h, Actor.h, and Actor.cpp

StudentWorld::StudentWorld(string assetPath)
: GameWorld(assetPath), m_level(nullptr), m_player(nullptr), m_levelComplete(false)
{
}

StudentWorld::~StudentWorld() {
    cleanUp();
}

int StudentWorld::init()
{
    m_levelComplete = false; // resets with every init
    int loadResult = loadLevel();
    if (loadResult != GWSTATUS_CONTINUE_GAME) return loadResult; // depends on whether there are any errors with file loading, or win condition reached

    // create Actors based on level file
    for (int yy = 0; yy < VIEW_HEIGHT; yy++) {
        for (int xx = 0; xx < VIEW_WIDTH; xx++) {
            // uses the Level class to get item at xx, yy
            // (0,0) is bottom left corner
            Level::MazeEntry me = m_level->getContentsOf(xx, yy);
            switch (me) {
            case Level::floor:
                m_actors.push_back(new Floor(this, xx, yy));
                break;
            case Level::ladder:
                m_actors.push_back(new Ladder(this, xx, yy));
                break;
            case Level::bonfire:
                m_actors.push_back(new Bonfire(this, xx, yy));
                break;
            case Level::extra_life:
                m_actors.push_back(new ExtraLifeGoodie(this, xx, yy));
                break;
            case Level::garlic:
                m_actors.push_back(new GarlicGoodie(this, xx, yy));
                break;
            case Level::fireball:
                m_actors.push_back(new Fireball(this, xx, yy));
                break;
            case Level::koopa:
                m_actors.push_back(new Koopa(this, xx, yy));
                break;
            case Level::left_kong:
                m_actors.push_back(new Kong(this, xx, yy, GraphObject::left));
                break;
            case Level::right_kong:
                m_actors.push_back(new Kong(this, xx, yy, GraphObject::right));
                break;
            case Level::player:
                m_player = new Player(this, xx, yy, GraphObject::right); // player faces right initially
                break;
            }
        }
    }

    return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move()
{
    updateDisplayText(); // updates game stats text based on latest statistics

    if (checkGameStatus() != GWSTATUS_CONTINUE_GAME) return checkGameStatus(); // check if finish conditions reached

    m_player->doSomething(); // read user input if conditions met (eg. not frozen)
    for (int i = 0; i < m_actors.size(); i++) {
        if (checkGameStatus() != GWSTATUS_CONTINUE_GAME) return checkGameStatus(); // Continuously check if finish conditions are reached
        if (m_actors[i]->alive()) { // only execute actions on Actors who are alive
            m_actors[i]->doSomething(); // Each actor carries out their action for one tick
        }
    }
    if (checkGameStatus() != GWSTATUS_CONTINUE_GAME) return checkGameStatus(); // check if finish conditions reached

    // Remove dead game objects
    for (vector<Actor*>::iterator it = m_actors.begin(); it != m_actors.end();) {
        if (!((*it)->alive())) {
            delete (*it);
            it = m_actors.erase(it); // update it to next iterator after erased object
        }
        else it++;
    }
    
    return GWSTATUS_CONTINUE_GAME; // no finish conditions reached
}

void StudentWorld::cleanUp()
{
    while (!m_actors.empty()) { // delete and pop all actors in the m_actors vector
        delete m_actors.back();
        m_actors.pop_back();
    }
    delete m_player; // release memory for player
    m_player = nullptr;
    delete m_level; // release memory for level object
    m_level = nullptr;
}

int StudentWorld::loadLevel() {
    // cerr << "Lives " << getLives() << endl;

    // Generate file string based on level number
    string curLevel = "level";
    int n_level = getLevel();
    if (n_level > 99) return GWSTATUS_PLAYER_WON; // maximum level reached => win condition
    if (n_level < 10) curLevel += "0";
    else {
        curLevel += (n_level / 10) + '0';
    }
    curLevel += (n_level % 10) + '0';
    curLevel += ".txt";
    m_level = new Level(assetPath());
    Level::LoadResult result = m_level->loadLevel(curLevel); // try to load level
    if (result == Level::load_fail_file_not_found) {
        delete m_level;
        m_level = nullptr; // safe to delete nullptr later, if necessary. Also acts as a flag for unloaded level
        cerr << "File not found." << endl;
        return GWSTATUS_PLAYER_WON; // no more higher levels => win condition
    }
    else if (result == Level::load_fail_bad_format) {
        delete m_level;
        m_level = nullptr;
        cerr << "File bad format." << endl;
        return GWSTATUS_LEVEL_ERROR; // error loading file
    }

    // otherwise the load was successful and we can access contents of level
    cerr << "Level "<<getLevel()<<" successfully loaded" << endl;
    return GWSTATUS_CONTINUE_GAME; // successfully loaded => continue game
}

bool StudentWorld::checkPassable(int xx, int yy) const {
    // check if wall. If indexes are not valid, returns false
    if (!checkIndex(xx, yy)) return false; // check indexes are valid
    for (int i = 0; i < m_actors.size(); i++) {
        if (m_actors[i]->getX() == xx && m_actors[i]->getY() == yy && !m_actors[i]->passable()) return false;
    }
    return true;
}

bool StudentWorld::checkClimbable(int xx, int yy) const {
    // check if ladder. If indexes are not valid, returns false
    for (int i = 0; i < m_actors.size(); i++) {
        if (m_actors[i]->getX() == xx && m_actors[i]->getY() == yy && m_actors[i]->climbable()) return true;
    }
    return false;
}

void StudentWorld::addActor(Actor* ap) {
    m_actors.push_back(ap);
}

void StudentWorld::attackEnemy(int xx, int yy, bool isBonfire) {
    for (int i = 0; i < m_actors.size(); i++) {
        if (m_actors[i]->getX() == xx && m_actors[i]->getY() == yy) {
            if (!isBonfire && m_actors[i]->blastable()) { // attack Enemy with Burp
                m_actors[i]->kill();
                playSound(SOUND_ENEMY_DIE);
                increaseScore(ENEMY_DIE_POINTS);
            }
            else if (isBonfire && m_actors[i]->burnable()) { // attack Barrel with Bonfire
                m_actors[i]->kill();
            }
            
        }
    }
}

void StudentWorld::attackPlayer() {
    m_player->kill();
    playSound(SOUND_PLAYER_DIE);
}

void StudentWorld::freezePlayer() {
    m_player->freeze();
}

bool StudentWorld::isPlayerLocation(int xx, int yy) {
    return xx == m_player->getX() && yy == m_player->getY();
}

void StudentWorld::increaseBurps(int k) {
    m_player->increaseBurps(k);
}

void StudentWorld::updateDisplayText() {
    ostringstream oss;
    oss << "Score: " << num2string(getScore(), 7) << "  ";
    oss << "Level: " << num2string(getLevel(), 2) << "  ";
    oss << "Lives: " << num2string(getLives(), 2) << "  ";
    oss << "Burps: " << num2string(m_player->getBurps(), 2);
    setGameStatText(oss.str());
}

bool StudentWorld::checkPlayerAlive() const {
    return m_player->alive();
}

void StudentWorld::dropGarlic(int xx, int yy) {
    addActor(new GarlicGoodie(this, xx, yy));
}

void StudentWorld::dropExtraLife(int xx, int yy) {
    addActor(new ExtraLifeGoodie(this, xx, yy));
}

bool StudentWorld::closeToPlayer(int xx, int yy) {
    int squaredDist = (m_player->getX() - xx) * (m_player->getX() - xx) + (m_player->getY() - yy) * (m_player->getY() - yy);
        
    return squaredDist <= (MIN_EUCLID_DISTANCE * MIN_EUCLID_DISTANCE);
}

int StudentWorld::checkGameStatus() {
    if (!(m_player->alive())) return GWSTATUS_PLAYER_DIED;
    if (m_levelComplete) return GWSTATUS_FINISHED_LEVEL;
    return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::setLevelComplete() {
    m_levelComplete = true;
}

// ===== Helper Functions =====

bool checkIndex(int xx, int yy) {
    return xx > 0 && yy > 0 && xx < VIEW_WIDTH && yy < VIEW_HEIGHT;
}

string num2string(int x, int digits) {
    string numString = "";
    for (int i = 0; i < digits; i ++) {
        numString += ((x % 10) + '0');
        x /= 10;
    }
    reverse(numString.begin(), numString.end());
    return numString;
}