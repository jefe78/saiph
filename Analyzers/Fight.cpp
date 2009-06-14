#include <stdlib.h>
#include "Fight.h"
#include "../Debug.h"
#include "../Inventory.h"
#include "../Saiph.h"
#include "../World.h"
#include "../Actions/Fight.h"
#include "../Actions/Move.h"
#include "../Actions/Throw.h"
#include "../Data/Dagger.h"
#include "../Data/Dart.h"
#include "../Data/Monster.h"
#include "../Data/Spear.h"
#include "../Events/ChangedInventoryItems.h"

using namespace analyzer;
using namespace event;
using namespace std;

/* constructors/destructor */
Fight::Fight() : Analyzer("Fight") {
	/* FIXME
	 * this should be dynamic, this varies with the class we play.
	 * this'll work for valks for the time being */
	/* daggers */
	for (map<string, data::Dagger *>::iterator i = data::Dagger::daggers.begin(); i != data::Dagger::daggers.end(); ++i)
		projectiles.insert(i->first);
	/* spears */
	for (map<string, data::Spear *>::iterator i = data::Spear::spears.begin(); i != data::Spear::spears.end(); ++i)
		projectiles.insert(i->first);
	/* darts */
	for (map<string, data::Dart *>::iterator i = data::Dart::darts.begin(); i != data::Dart::darts.end(); ++i)
		projectiles.insert(i->first);
}

/* methods */
void Fight::analyze() {
	/* if engulfed try to fight our way out */
	if (World::engulfed) {
		World::setAction(static_cast<action::Action *>(new action::Fight(this, NW, PRIORITY_FIGHT_MELEE_MAX)));
		return;
	}
	/* fight monsters */
	int attack_score = INT_MIN;
	for (map<Point, Monster>::iterator m = World::levels[Saiph::position.level].monsters.begin(); m != World::levels[Saiph::position.level].monsters.end(); ++m) {
		if (m->second.symbol == PET)
			continue; // we're not fighting pets :)
		else if (m->second.attitude == FRIENDLY)
			continue; // don't attack friendlies
		else if (m->second.symbol == S_UNICORN && ((m->second.color == BOLD_WHITE && Saiph::alignment == LAWFUL) || (m->second.color == WHITE && Saiph::alignment == NEUTRAL) || (m->second.color == BLUE && Saiph::alignment == CHAOTIC)))
			continue; // don't attack unicorns of same alignment
		else if (m->second.data == NULL) {
			/* seems like MonsterInfo haven't had the chance to farlook monster. set attack_score to max */
			Debug::analyzer(name) << "Found monster we don't know data about. Hmm" << endl;
			attack_score = data::Monster::saiph_difficulty_max;
		} else {
			/* figure out the attack score */
			/* FIXME
			 * this needs to be improved later.
			 * currently we use "saiph_difficulty", whether we can melee/throw at it and how far away it is.
			 * this is a too simple solution, we need to consider stuff as how hard they hit, if they steal
			 * stuff, if they use wands, etc */
			attack_score = m->second.data->saiph_difficulty;
		}
		int distance = max(abs(m->first.row - Saiph::position.row), abs(m->first.col - Saiph::position.col));
		bool floating_eye = (m->second.symbol == S_EYE && m->second.color == BLUE);
		if ((distance > 1 || floating_eye) && projectile_slots.size() > 0) {
			/* got projectiles and monster is not next to us or it's a floating eye.
			 * should check if we can throw projectile at the monster */
			unsigned char in_line = World::directLine(m->first, false, true);
			if (in_line != ILLEGAL_DIRECTION) {
				/* we can throw at monster */
				attack_score -= distance;
				int priority = (attack_score - data::Monster::saiph_difficulty_min) * (PRIORITY_FIGHT_THROW_MAX - PRIORITY_FIGHT_THROW_MIN) / (data::Monster::saiph_difficulty_max - data::Monster::saiph_difficulty_min) + PRIORITY_FIGHT_THROW_MIN;
				World::setAction(static_cast<action::Action *>(new action::Throw(this, *projectile_slots.begin(), in_line, priority)));
				Debug::analyzer(name) << "Setting action to throw at '" << m->second.symbol << "' which is " << distance << " squares away with priority " << priority << endl;
				continue;
			}
		} else if (distance == 1 && !floating_eye) {
			/* next to monster, and it's not a floating eye. melee */
			int priority = (attack_score - data::Monster::saiph_difficulty_min) * (PRIORITY_FIGHT_MELEE_MAX - PRIORITY_FIGHT_MELEE_MIN) / (data::Monster::saiph_difficulty_max - data::Monster::saiph_difficulty_min) + PRIORITY_FIGHT_MELEE_MIN;
			World::setAction(static_cast<action::Action *>(new action::Fight(this, World::shortestPath(m->first).dir, priority)));
			Debug::analyzer(name) << "Setting action to melee '" << m->second.symbol << "' with priority " << priority << endl;
			continue;
		}
		/* we can neither melee nor throw at the monster, move towards it */
		int priority = (attack_score - data::Monster::saiph_difficulty_min) * (PRIORITY_FIGHT_MOVE_MAX - PRIORITY_FIGHT_MOVE_MIN) / (data::Monster::saiph_difficulty_max - data::Monster::saiph_difficulty_min) + PRIORITY_FIGHT_MOVE_MIN;
		const PathNode &node = World::shortestPath(m->first);
		World::setAction(static_cast<action::Action *>(new action::Move(this, node.dir, action::Move::calculatePriority(priority, node.moves))));
		Debug::analyzer(name) << "Setting action to move towards '" << m->second.symbol << "' which is " << distance << " squares away with priority " << priority << endl;
	}
}

void Fight::onEvent(Event *const event) {
	if (event->getID() == ChangedInventoryItems::id) {
		ChangedInventoryItems *e = static_cast<ChangedInventoryItems *>(event);
		for (set<unsigned char>::iterator k = e->keys.begin(); k != e->keys.end(); ++k) {
			map<unsigned char, Item>::iterator i = Inventory::items.find(*k);
			if (i == Inventory::items.end()) {
				/* we lost this item, remove it from projectile_slots */
				projectile_slots.erase(*k);
			} else {
				/* this item is new or changed.
				 * if we intend to throw it, add it to projectile_slots.
				 * otherwise remove it from projectile_slots */
				set<string>::iterator p = projectiles.find(i->second.name);
				if (p != projectiles.end())
					projectile_slots.insert(*k);
				else
					projectile_slots.erase(*k);
			}
		}
	}
}
