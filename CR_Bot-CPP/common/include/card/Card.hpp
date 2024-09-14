#pragma once

#include <string>

#include <cpp-dump.hpp>

namespace card {
	struct BaseCard {
		std::string path;
		int id = 0;
		int elixir = 0;
		std::optional<std::string> evo_path;
		bool china = false;
	};

	enum class Card {
		Blank,

		// Common Troop Cards
		Minions,
		Archers,
		Knight,
		Spear_Goblins,
		Goblins,
		Bomber,
		Skeletons,
		Barbarians,
		Electro_Spirit,
		Skeleton_Dragons,
		Fire_Spirit,
		Bats,
		Royal_Recruits,
		Royal_Giant,
		Ice_Spirit,
		Skeleton_Barrel,
		Goblin_Gang,
		Elite_Barbarians,
		Minion_Horde,
		Firecracker,
		Rascals,

		// Rare Troop Cards
		Mini_Pekka,
		Musketeer,
		Giant,
		Valkyrie,
		Mega_Minion,
		Battle_Ram,
		Wizard,
		Flying_Machine,
		Hog_Rider,
		Royal_Hogs,
		Three_Musketeers,
		Battle_Healer,
		Ice_Golem,
		Dart_Goblin,
		Zappies,
		Heal_Spirit,
		Elixir_Golem,
		Goblin_Demolisher,
		Suspicious_Bush,

		// Epic Troop Cards
		Guards,
		Baby_Dragon,
		Skeleton_Army,
		Witch,
		Pekka,
		Dark_Prince,
		Prince,
		Balloon,
		Giant_Skeleton,
		Goblin_Giant,
		Hunter,
		Golem,
		Electro_Dragon,
		Wall_Breakers,
		Electro_Giant,
		Bowler,
		Executioner,
		Cannon_Cart,

		// Legendary Troop Cards
		Mega_Knight,
		Ram_Rider,
		Electro_Wizard,
		Inferno_Dragon,
		Sparky,
		Miner,
		Princess,
		Phoenix,
		Royal_Ghost,
		Ice_Wizard,
		Magic_Archer,
		Bandit,
		Lava_Hound,
		Night_Witch,
		Lumberjack,
		Mother_Witch,
		Fisherman,
		Goblin_Machine,

		// Champion Troop Cards
		Golden_Knight,
		Skeleton_King,
		Archer_Queen,
		Mighty_Miner,
		Little_Prince,
		Monk,

		// Common Spell Cards
		Arrows,
		Zap,
		Giant_Snowball,
		Royal_Delivery,

		// Rare Spell Cards
		Fireball,
		Rocket,
		Earthquake,

		// Epic Spell Cards
		Goblin_Barrel,
		Lightning,
		Freeze,
		Barbarian_Barrel,
		Poison,
		Rage,
		Clone,
		Tornado,
		Mirror,
		Void,
		Goblin_Curse,
		The_Log,
		Graveyard,

		// Common Building Cards
		Cannon,
		Mortar,
		Tesla,

		// Rare Building Cards
		Goblin_Cage,
		Goblin_Hut,
		Tombstone,
		Bomb_Tower,
		Inferno_Tower,
		Barbarian_Hut,
		Furnace,
		Elixir_Collector,

		// Epic Building Cards
		X_Bow,
		Goblin_Drill
	};
}

CPP_DUMP_DEFINE_EXPORT_OBJECT(card::BaseCard, id, elixir, path, evo_path);
