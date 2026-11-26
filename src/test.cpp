// g++ -std=c++17 src/test.cpp -o /tmp/rm_test && /tmp/rm_test
#include "filter.hpp"
#include <cassert>
#include <cstdio>
int main()
{
	assert(Glob("models/props_junk/garbage_milkcarton*", "models/props_junk/garbage_milkcarton001a.mdl"));
	assert(Glob("PROP_*", "prop_physics") && !Glob("prop_*", "func_prop"));
	const char *lump =
		"{\n\"classname\" \"worldspawn\"\n}\n"
		"{\n\"classname\" \"prop_physics\"\n\"model\" \"models/props_junk/garbage_milkcarton001a.mdl\"\n}\n"
		"{\n\"classname\" \"info_particle_system\"\n\"effect_name\" \"map_fog_1\"\n\"targetname\" \"a{b}\"\n}\n"
		"{\n\"classname\" \"info_player_start\"\n}\n";
	std::vector<Rule> rules = {{"", "*garbage_milkcarton*"}, {"effect_name", "*_fog_*"}};
	int n;
	std::string r = FilterLump(lump, rules, n);
	assert(n == 2);
	assert(r.find("worldspawn") != std::string::npos && r.find("info_player_start") != std::string::npos);
	assert(r.find("milkcarton") == std::string::npos && r.find("particle") == std::string::npos);
	std::string t = FilterLump("{ \"classname\" \"x", rules, n); // truncated lump stays intact
	assert(n == 0 && t == "{ \"classname\" \"x");
	std::vector<Rule> all = {{"", "*"}};
	std::string w = FilterLump(lump, all, n);
	assert(n == 3 && w.find("worldspawn") != std::string::npos); // "*" keeps worldspawn
	puts("ok");
}
