#include "Player.h"

/* constructors */
Player::Player() {
	/* zero attributes */
	this->attributes.strength = 0;
	this->attributes.dexterity = 0;
	this->attributes.constitution = 0;
	this->attributes.intelligence = 0;
	this->attributes.wisdom = 0;
	this->attributes.charisma = 0;
	this->attributes.alignment = 0;

	/* zero status */
	this->status.dungeon = 0;
	this->status.zorkmids = 0;
	this->status.hitpoints = 0;
	this->status.hitpoints_max = 0;
	this->status.power = 0;
	this->status.power_max = 0;
	this->status.armor_class = 0;
	this->status.experience = 0;
	this->status.turn = 0;
	this->status.encumbrance = 0;
	this->status.hunger = 0;
	this->status.blind = false;
	this->status.confused = false;
	this->status.foodpoisoned = false;
	this->status.hallucinating = false;
	this->status.ill = false;
	this->status.slimed = false;
	this->status.stunned = false;

	/* zero position */
	this->position.col = 0;
	this->position.row = 0;
}

/* destructors */
Player::~Player() {
}

/* methods */
void Player::parseAttributesAndStatus(Map map) {
	/* fetch attributes */
	char alignment[COLS];
	alignment[0] = '\0';
	int matched = sscanf(map.map[22], "%*[^:]:%d%*[^:]:%d%*[^:]:%d%*[^:]:%d%*[^:]:%d%*[^:]:%d%s", &attributes.strength, &attributes.dexterity, &attributes.constitution, &attributes.intelligence, &attributes.wisdom, &attributes.charisma, alignment);
	if (matched < 7) {
		cerr << "Error parsing attribute line, expected 7 values, found " << matched << endl;
		exit(11);
	}
	if (alignment[0] == 'L')
		attributes.alignment = 1;
	else if (alignment[0] == 'N')
		attributes.alignment = 0;
	else
		attributes.alignment = -1;

	/* fetch status */
	status.encumbrance = 0;
	status.hunger = 0;
	status.blind = false;
	status.confused = false;
	status.foodpoisoned = false;
	status.hallucinating = false;
	status.ill = false;
	status.slimed = false;
	status.stunned = false;
	char effects[5][COLS];
	matched = sscanf(map.map[23], "%*[^:]:%d%*[^:]:%d%*[^:]:%d(%d%*[^:]:%d(%d%*[^:]:%d%*[^:]:%d%*[^:]:%d%s%s%s%s%s", &status.dungeon, &status.zorkmids, &status.hitpoints, &status.hitpoints_max, &status.power, &status.power_max, &status.armor_class, &status.experience, &status.turn, effects[0], effects[1], effects[2], effects[3], effects[4]);
	if (matched < 9) {
		cerr << "Error parsing status line, expected at least 9 values, found " << matched << endl;
		exit(12);
	}
	int effects_found = matched - 9;
	for (int a = 0; a < effects_found; ++a) {
		if (strcmp(effects[a], "Burdened") == 0) {
			status.encumbrance = 1;
		} else if (strcmp(effects[a], "Stressed") == 0) {
			status.encumbrance = 2;
		} else if (strcmp(effects[a], "Strained") == 0) {
			status.encumbrance = 3;
		} else if (strcmp(effects[a], "Overtaxed") == 0) {
			status.encumbrance = 4;
		} else if (strcmp(effects[a], "Overloaded") == 0) {
			status.encumbrance = 5;
		} else if (strcmp(effects[a], "Fainting") == 0) {
			status.hunger = -3;
		} else if (strcmp(effects[a], "Weak") == 0) {
			status.hunger = -2;
		} else if (strcmp(effects[a], "Hungry") == 0) {
			status.hunger = -1;
		} else if (strcmp(effects[a], "Satiated") == 0) {
			status.hunger = 1;
		} else if (strcmp(effects[a], "Oversatiated") == 0) {
			status.hunger = 2;
		} else if (strcmp(effects[a], "Blind") == 0) {
			status.blind = true;
		} else if (strcmp(effects[a], "Conf") == 0) {
			status.confused = true;
		} else if (strcmp(effects[a], "FoodPois") == 0) {
			status.foodpoisoned = true;
		} else if (strcmp(effects[a], "Hallu") == 0) {
			status.hallucinating = true;
		} else if (strcmp(effects[a], "Ill") == 0) {
			status.ill = true;
		} else if (strcmp(effects[a], "Slime") == 0) {
			status.slimed = true;
		} else if (strcmp(effects[a], "Stun") == 0) {
			status.stunned = true;
		}
	}

	/* fetch position */
	position.row = map.row;
	position.col = map.col;
}