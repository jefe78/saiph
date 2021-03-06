#include "Tile.h"
#include "World.h"

using namespace std;

/* constructors/destructor */
Tile::Tile(const Coordinate& coordinate) : _coordinate(coordinate), _symbol(SOLID_ROCK), _monster(ILLEGAL_MONSTER), _direction(ILLEGAL_DIRECTION), _search(0), _distance(UNREACHABLE), _cost(UNREACHABLE), _updated(0) {
}

Tile::~Tile() {
}

/* methods */
const Coordinate& Tile::coordinate() const {
	return _coordinate;
}

unsigned int Tile::cost() {
	if (_updated != World::internalTurn() && _coordinate.level() == Saiph::position().level())
		_cost = UNREACHABLE;
	return _cost;
}

unsigned char Tile::direction() {
	if (_updated != World::internalTurn() && _coordinate.level() == Saiph::position().level())
		_direction = ILLEGAL_DIRECTION;
	return _direction;
}

unsigned char Tile::direction(unsigned char direction) {
	_direction = direction;
	return this->direction();
}

unsigned int Tile::distance() {
	if (_updated != World::internalTurn() && _coordinate.level() == Saiph::position().level())
		_distance = UNREACHABLE;
	return _distance;
}

unsigned char Tile::monster() const {
	return _monster;
}

unsigned char Tile::monster(unsigned char monster) {
	_monster = monster;
	return this->monster();
}

unsigned int Tile::search() const {
	return _search;
}

unsigned int Tile::search(unsigned int search) {
	_search = search;
	return this->search();
}

unsigned int Tile::searchInc(int count) {
	if (_search < TILE_FULLY_SEARCHED)
		_search += count;
	return _search;
}

unsigned char Tile::symbol() const {
	return _symbol;
}

unsigned char Tile::symbol(unsigned char symbol) {
	_symbol = symbol;
	return this->symbol();
}

void Tile::updatePath(unsigned char direction, unsigned int distance, unsigned int cost) {
	_direction = direction;
	_distance = distance;
	_cost = cost;
	_updated = World::internalTurn();
}

ostream & operator<<(ostream& os, Tile& t) {
	return os << "(coordinate=" << t.coordinate() << ", symbol=" << t.symbol() << ", monster=" << t.monster() << ", direction=" << t.direction() << ", search=" << t.search() << ", distance=" << t.distance() << ", cost=" << t.cost() << ")";
}
