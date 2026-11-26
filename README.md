# rm-css

Metamod:Source plugin (CS:S, 32-bit) that removes unwanted entities from a map **before it loads**.
A simpler Stripper: the config is just a list of patterns. Replaces the SourceMod `rm` plugin
(`remove_entities.cfg` + `remove-fog.sp`) and needs no SourceMod and no map editing.

## Install

Download `rm-css-vX.Y.Z.zip` from the [Releases](https://github.com/FestClub/rm-css/releases) and extract it into `<server>/cstrike/`
(it contains the `addons/` folder). Or build it yourself:

```sh
./deps.sh                       # once: fetches the SDK headers into third_party/
./build.sh                      # runs the test, then builds dist/
cp -r dist/addons/. <server>/cstrike/addons/
```

Installs `addons/metamod/rm_css.vdf`, `addons/rm_css/bin/rm_css.so` and `addons/rm_css/rm.cfg`.
Restart the server, then check with `meta list` (shows `rm-css`).

Build needs `git`, `clang++` and the 32-bit toolchain (`g++-multilib`). CI (`.github/workflows/build.yml`) builds on every push, caches the toolchain and SDK headers, and attaches the zip to a release when you push a `v*` tag.

## Config

- `addons/rm_css/rm.cfg`: global rules.
- `addons/rm_css/maps/<map>.cfg`: rules for one map only.
- Re-read on every map load, so edit and change map. No reload command.

One rule per line; `#` or `//` start a comment. `*` is a wildcard, matching is case-insensitive.

| Rule | Removes entities whose... |
|---|---|
| `pattern` | classname **or** model matches |
| `key=pattern` | keyvalue `key` matches |

```ini
prop_*                                  # all props
models/props_junk/garbage_milkcarton*   # a specific model (milk carton trash)
effect_name=*_fog_*                     # fog particles
targetname=trash_*
```

The default `rm.cfg` reproduces the old plugin: `prop_*`, `physics_prop*`, `phys_*`, `env_fog_controller`,
`func_smokevolume`, `func_dustmotes`, `effect_name=*_fog_*`. **`prop_*` removes every prop** (doors included);
use narrower rules if a map breaks.

## How it works

- Hooks `IServerGameDLL::LevelInit` and passes the filtered entity lump to the engine.
- Hooks `IVEngineServer::GetMapEntitiesString` so `CleanUpMap` (round restart) also uses the filtered lump.
- `LevelShutdown` drops the lump. `worldspawn` is never removed, even by `*`.
- No per-frame hooks. Filtering costs ~1.5 ms per map load (dust2, default rules).

## Limits

- **Static props** (`prop_static` compiled into the `.bsp`) are not entities and cannot be removed. Needs a map edit.
- Only keyvalues present in the map file can be matched (not runtime values).
- To find a model name in game, use `sm_identifymodel` from the SourceMod plugin.

## License

GPL-3.0, see [LICENSE](LICENSE). https://github.com/FestClub/rm-css

## Files

- `src/rm_css.cpp`: the plugin
- `src/filter.hpp`: glob + lump filter (no engine deps)
- `src/test.cpp`: native test, run by `build.sh`
- `deps.sh`: fetches the pinned Metamod:Source and HL2SDK-CSS headers into `third_party/` (git-ignored)
- `config/rm.cfg`: default config shipped in the release
- `.github/workflows/build.yml`: build, cache, release
