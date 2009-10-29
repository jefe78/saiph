#include <stdlib.h>
#include "Fight.h"
#include "../Debug.h"
#include "../EventBus.h"
#include "../Inventory.h"
#include "../Saiph.h"
#include "../World.h"
#include "../Actions/Fight.h"
#include "../Actions/Move.h"
#include "../Actions/Select.h"
#include "../Actions/Throw.h"
#include "../Data/Dagger.h"
#include "../Data/Dart.h"
#include "../Data/Monster.h"
#include "../Data/Spear.h"
#include "../Events/ChangedInventoryItems.h"
#include "../Events/ReceivedItems.h"
#include "../Events/WantItems.h"

using namespace analyzer;
using namespace event;
using namespace std;

/* constructors/destructor */
Fight::Fight() : Analyzer("Fight") {
	/* FIXME
	 * this should be dynamic, this varies with the class we play.
	 * this'll work for valks for the time being */
	/* daggers */
	for (map<const string, const data::Dagger*>::const_iterator i = data::Dagger::daggers().begin(); i != data::Dagger::daggers().end(); ++i)
		_projectiles.insert(i->first);
	/* spears */
	for (map<const string, const data::Spear*>::const_iterator i = data::Spear::spears().begin(); i != data::Spear::spears().end(); ++i)
		_projectiles.insert(i->first);
	/* darts */
	for (map<const string, const data::Dart*>::const_iterator i = data::Dart::darts().begin(); i != data::Dart::darts().end(); ++i)
		_projectiles.insert(i->first);

	/* register events */
	EventBus::registerEvent(ChangedInventoryItems::ID, this);
	EventBus::registerEvent(ReceivedItems::ID, this);
	EventBus::registerEvent(WantItems::ID, this);
}

/* methods */
void Fight::analyze() {
	/* if engulfed try to fight our way out */
	if (Saiph::engulfed()) {
		World::setAction(static_cast<action::Action*> (new action::Fight(this, NW, PRIORITY_FIGHT_MELEE_MAX)));
		return;
	}
	/* fight monsters */
	int attack_score = INT_MIN;
	for (map<Point, Monster>::iterator m = World::level(Saiph::position().level()).monsters().begin(); m != World::level(Saiph::position().level()).monsters().end(); ++m) {
		if (m->second.symbol() == PET)
			continue; // we're not fighting pets :)
		else if (m->second.attitude() == FRIENDLY)
			continue; // don't attack friendlies
		else if (m->second.symbol() == S_UNICORN && ((m->second.color() == BOLD_WHITE && Saiph::alignment() == LAWFUL) || (m->second.color() == WHITE && Saiph::alignment() == NEUTRAL) || (m->second.color() == BLUE && Saiph::alignment() == CHAOTIC)))
			continue; // don't attack unicorns of same alignment
		else if (m->second.data() == NULL) {
			/* seems like MonsterInfo haven't had the chance to farlook monster. set attack_score to max */
			Debug::analyzer(name()) << "Found monster we don't know data about. Hmm" << endl;
			attack_score = data::Monster::saiphDifficultyMax();
		} else {
			/* figure out the attack score */
			/* FIXME
			 * this needs to be improved later.
			 * currently we use "saiph_difficulty", whether we can melee/throw at it and how far away it is.
			 * this is a too simple solution, we need to consider stuff as how hard they hit, if they steal
			 * stuff, if they use wands, etc */
			attack_score = m->second.data()->saiphDifficulty();
		}
		int distance = max(abs(m->first.row() - Saiph::position().row()), abs(m->first.col() - Saiph::position().col()));
		bool floating_eye = (m->second.symbol() == S_EYE && m->second.color() == BLUE);
		if (m->second.visible() && (distance > 1 || floating_eye) && _projectile_slots.size() > 0 && distance <= Saiph::strength() / 2) {
			/* got projectiles and monster is not next to us or it's a floating eye.
			 * should check if we can throw projectile at the monster */
			unsigned char in_line = World::directLine(m->first, false, true);
			if (in_line != ILLEGAL_DIRECTION) {
				/* we can throw at monster */
				attack_score -= distance;
				int priority = (attack_score - data::Monster::saiphDifficultyMin()) * (PRIORITY_FIGHT_THROW_MAX - PRIORITY_FIGHT_THROW_MIN) / (data::Monster::saiphDifficultyMax() - data::Monster::saiphDifficultyMin()) + PRIORITY_FIGHT_THROW_MIN;
				World::setAction(static_cast<action::Action*> (new action::Throw(this, *_projectile_slots.begin(), in_line, priority)));
				Debug::analyzer(name()) << "Setting action to throw at '" << m->second.symbol() << "' which is " << distance << " squares away with priority " << priority << endl;
				continue;
			}
		} else if (distance == 1 && !floating_eye) {
			/* next to monster, and it's not a floating eye. melee */
			int priority = (attack_score - data::Monster::saiphDifficultyMin()) * (PRIORITY_FIGHT_MELEE_MAX - PRIORITY_FIGHT_MELEE_MIN) / (data::Monster::saiphDifficultyMax() - data::Monster::saiphDifficultyMin()) + PRIORITY_FIGHT_MELEE_MIN;
			World::setAction(static_cast<action::Action*> (new action::Fight(this, World::shortestPath(m->first).direction(), priority)));
			Debug::analyzer(name()) << "Setting action to melee '" << m->second.symbol() << "' with priority " << priority << endl;
			continue;
		}
		/* we can neither melee nor throw at the monster, move towards it */
		const Tile& tile = World::shortestPath(m->first);
		if (tile.direction() == ILLEGAL_DIRECTION)
			continue; // can't move to monster
		int priority = (attack_score - data::Monster::saiphDifficultyMin()) * (PRIORITY_FIGHT_MOVE_MAX - PRIORITY_FIGHT_MOVE_MIN) / (data::Monster::saiphDifficultyMax() - data::Monster::saiphDifficultyMin()) + PRIORITY_FIGHT_MOVE_MIN;
		priority = action::Move::calculatePriority(priority, tile.cost());
		World::setAction(static_cast<action::Action*> (new action::Move(this, tile.direction(), priority)));
		Debug::analyzer(name()) << "Setting action to move towards '" << m->second.symbol() << "' which is " << distance << " squares away with priority " << priority << endl;
	}
}

void Fight::onEvent(Event * const event) {
	if (event->id() == ChangedInventoryItems::ID) {
		ChangedInventoryItems* e = static_cast<ChangedInventoryItems*> (event);
		for (set<unsigned char>::iterator k = e->keys().begin(); k != e->keys().end(); ++k) {
			map<unsigned char, Item>::iterator i = Inventory::items().find(*k);
			if (i == Inventory::items().end()) {
				/* we lost this item, remove it from projectile_slots */
				_projectile_slots.erase(*k);
				Debug::analyzer(name()) << "Removing key '" << *k << "' from projectile_slots" << endl;
			} else {
				/* this item is new or changed.
				 * if we intend to throw it, add it to projectile_slots.
				 * otherwise remove it from projectile_slots */
				if (wantItem(i->second)) {
					_projectile_slots.insert(*k);
					Debug::analyzer(name()) << "Adding key '" << *k << "' to projectile_slots, item: " << i->second << endl;
				} else {
					_projectile_slots.erase(*k);
					Debug::analyzer(name()) << "Removing key '" << *k << "' from projectile_slots" << endl;
				}
			}
		}
	} else if (event->id() == ReceivedItems::ID) {
		ReceivedItems* e = static_cast<ReceivedItems*> (event);
		for (map<unsigned char, Item>::iterator i = e->items().begin(); i != e->items().end(); ++i) {
			if (wantItem(i->second))
				_projectile_slots.insert(i->first);
		}
	} else if (event->id() == WantItems::ID) {
		WantItems* e = static_cast<WantItems*> (event);
		for (map<unsigned char, Item>::iterator i = e->items().begin(); i != e->items().end(); ++i) {
			if (wantItem(i->second))
				i->second.want(i->second.count());
		}
	}
}

/* private methods */
bool Fight::wantItem(const Item& item) {
	/* return whether we want this item or not */
	return _projectiles.find(item.name()) != _projectiles.end();
}
