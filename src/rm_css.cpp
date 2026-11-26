// rm-css: Metamod:Source plugin (CS:S, 32-bit). Strips unwanted entities from the map's entity
// lump before the map loads (like Stripper, but the config is just a list of patterns).
// Config: <gamedir>/addons/rm_css/rm.cfg (global) + addons/rm_css/maps/<map>.cfg (per map). Re-read on every map load.
#include <ISmmPlugin.h>
#include <eiface.h>
#include "filter.hpp"

PLUGIN_GLOBALVARS();
SH_DECL_HOOK6(IServerGameDLL, LevelInit, SH_NOATTRIB, 0, bool, const char *, const char *, const char *, const char *, bool, bool);

SH_DECL_HOOK0_void(IServerGameDLL, LevelShutdown, SH_NOATTRIB, 0);
SH_DECL_HOOK0(IVEngineServer, GetMapEntitiesString, SH_NOATTRIB, 0, const char *);

IServerGameDLL *server = nullptr;
IVEngineServer *engine = nullptr;
// Never destroyed: the server may keep the pointer past a "meta unload".
static std::string &g_lump = *new std::string;

class RmCss : public ISmmPlugin
{
public:
	bool Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late);
	bool Unload(char *error, size_t maxlen);
	const char *Hook_GetMapEntitiesString();
	void Hook_LevelShutdown() { g_lump.clear(); } // never serve the previous map's lump
	bool Hook_LevelInit(const char *map, const char *ents, const char *old, const char *lm, bool load, bool bg);
	const char *GetAuthor() { return "这不是 Hack 而是技能"; }
	const char *GetName() { return "rm-css"; }
	const char *GetDescription() { return "Removes map entities by classname/model/key patterns"; }
	const char *GetURL() { return "https://github.com/FestClub/rm-css"; }
	const char *GetLicense() { return "GPL-3.0"; }
	const char *GetVersion() { return "1.0.0"; }
	const char *GetDate() { return __DATE__; }
	const char *GetLogTag() { return "RM"; }
};

RmCss g_Plugin;
PLUGIN_EXPOSE(RmCss, g_Plugin);

bool RmCss::Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	PLUGIN_SAVEVARS();
	GET_V_IFACE_ANY(GetServerFactory, server, IServerGameDLL, INTERFACEVERSION_SERVERGAMEDLL);
	GET_V_IFACE_CURRENT(GetEngineFactory, engine, IVEngineServer, INTERFACEVERSION_VENGINESERVER);
	SH_ADD_HOOK_MEMFUNC(IVEngineServer, GetMapEntitiesString, engine, this, &RmCss::Hook_GetMapEntitiesString, false);
	SH_ADD_HOOK_MEMFUNC(IServerGameDLL, LevelShutdown, server, this, &RmCss::Hook_LevelShutdown, false);
	SH_ADD_HOOK_MEMFUNC(IServerGameDLL, LevelInit, server, this, &RmCss::Hook_LevelInit, false);
	return true;
}

bool RmCss::Unload(char *error, size_t maxlen)
{
	SH_REMOVE_HOOK_MEMFUNC(IVEngineServer, GetMapEntitiesString, engine, this, &RmCss::Hook_GetMapEntitiesString, false);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameDLL, LevelShutdown, server, this, &RmCss::Hook_LevelShutdown, false);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameDLL, LevelInit, server, this, &RmCss::Hook_LevelInit, false);
	return true;
}

// CS:S CleanUpMap respawns map entities from this string on every round restart.
const char *RmCss::Hook_GetMapEntitiesString()
{
	if (g_lump.empty())
		RETURN_META_VALUE(MRES_IGNORED, nullptr);
	RETURN_META_VALUE(MRES_SUPERCEDE, g_lump.c_str());
}

bool RmCss::Hook_LevelInit(const char *map, const char *ents, const char *old, const char *lm, bool load, bool bg)
{
	std::string dir = std::string(g_SMAPI->GetBaseDir()) + "/addons/rm_css/";
	const char *base = strrchr(map, '/'); // basename only: no path tricks from the map name
	std::vector<Rule> rules;
	if (!ents)
		RETURN_META_VALUE(MRES_IGNORED, true);
	LoadRules(dir + "rm.cfg", rules);
	LoadRules(dir + "maps/" + (base ? base + 1 : map) + ".cfg", rules);
	if (rules.empty())
		RETURN_META_VALUE(MRES_IGNORED, true);

	int removed;
	std::string filtered = FilterLump(ents, rules, removed); // ents may alias g_lump: build before assigning
	g_lump = std::move(filtered);
	META_CONPRINTF("[RM] %s: %d rules, %d entities removed\n", map, (int)rules.size(), removed);
	RETURN_META_VALUE_NEWPARAMS(MRES_HANDLED, true, &IServerGameDLL::LevelInit, (map, g_lump.c_str(), old, lm, load, bg));
}

// Defined here instead of linking tier1 (same reason as patch-vphys).
InterfaceReg *InterfaceReg::s_pInterfaceRegs = nullptr;
InterfaceReg::InterfaceReg(InstantiateInterfaceFn fn, const char *pName) : m_CreateFn(fn), m_pName(pName)
{
	m_pNext = s_pInterfaceRegs;
	s_pInterfaceRegs = this;
}

extern "C" __attribute__((visibility("default"))) void *CreateInterface(const char *name, int *rc)
{
	for (InterfaceReg *r = InterfaceReg::s_pInterfaceRegs; r; r = r->m_pNext)
		if (!strcmp(r->m_pName, name)) { if (rc) *rc = 0; return r->m_CreateFn(); }
	if (rc) *rc = 1;
	return nullptr;
}
