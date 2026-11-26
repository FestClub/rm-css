// Pure logic (no engine deps) so it can be tested natively.
#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <cstring>
#include <cctype>
#include <strings.h>

// Case-insensitive glob, '*' = any run of chars.
inline bool Glob(const char *p, const char *s)
{
	if (!*p) return !*s;
	if (*p == '*')
	{
		while (p[1] == '*') p++;
		for (;; s++)
		{
			if (Glob(p + 1, s)) return true;
			if (!*s) return false;
		}
	}
	return *s && tolower((unsigned char)*p) == tolower((unsigned char)*s) && Glob(p + 1, s + 1);
}

// A rule matches an entity when "key" (empty = classname or model) has a value matching "pat".
struct Rule { std::string key, pat; };

inline std::string Trim(const std::string &s)
{
	size_t a = s.find_first_not_of(" \t\r\n"), b = s.find_last_not_of(" \t\r\n");
	return a == std::string::npos ? "" : s.substr(a, b - a + 1);
}

// Lines: "pattern" or "key=pattern". '#' and '//' start comments.
inline void LoadRules(const std::string &path, std::vector<Rule> &out)
{
	std::ifstream f(path);
	std::string line;
	while (std::getline(f, line))
	{
		line = Trim(line);
		if (line.empty() || line[0] == '#' || line.compare(0, 2, "//") == 0) continue;
		size_t eq = line.find('=');
		if (eq == std::string::npos) out.push_back({"", line});
		else out.push_back({Trim(line.substr(0, eq)), Trim(line.substr(eq + 1))});
	}
}

inline bool Matches(const std::vector<Rule> &rules, const std::vector<std::pair<std::string, std::string>> &kv)
{
	for (const auto &p : kv) // a catch-all rule must never delete the world
		if (!strcasecmp(p.first.c_str(), "classname") && !strcasecmp(p.second.c_str(), "worldspawn")) return false;
	for (const Rule &r : rules)
		for (const auto &p : kv)
			if ((r.key.empty() ? (!strcasecmp(p.first.c_str(), "classname") || !strcasecmp(p.first.c_str(), "model"))
			                   : !strcasecmp(p.first.c_str(), r.key.c_str())) &&
			    Glob(r.pat.c_str(), p.second.c_str()))
				return true;
	return false;
}

// Returns the entity lump without the entities matching any rule.
inline std::string FilterLump(const char *src, const std::vector<Rule> &rules, int &removed)
{
	std::string out;
	const char *p = src;
	removed = 0;
	while (const char *b = strchr(p, '{'))
	{
		out.append(p, b - p);
		std::vector<std::pair<std::string, std::string>> kv;
		std::string tok[2];
		int n = 0;
		const char *e = b + 1;
		while (*e && *e != '}')
		{
			if (*e == '"')
			{
				const char *q = strchr(e + 1, '"');
				if (!q) { e = q = e + strlen(e); break; }
				if (!n) { tok[0].assign(e + 1, q - e - 1); n = 1; } // key
				else { kv.push_back({tok[0], std::string(e + 1, q - e - 1)}); n = 0; } // value
				e = q;
			}
			e++;
		}
		if (!*e) { out.append(b); return out; } // unterminated: keep the rest untouched
		if (Matches(rules, kv)) removed++;
		else out.append(b, e + 1 - b);
		p = e + 1;
	}
	out.append(p);
	return out;
}
