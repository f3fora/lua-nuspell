/***
 * lua-nuspell is a set of Lua 5.x bindings for Nuspell spellchecking C++ library.
 *
 * @module nuspell
 */
#include <nuspell/finder.hxx>
#include <nuspell/dictionary.hxx>
#include <iostream>

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#ifndef VERSION
#define VERSION "scm"
#endif

#ifndef PROJECT_NAME
#define PROJECT_NAME "nuspell"
#endif

#define MT "Dictionary"

template <typename vector> static inline auto return_array(lua_State *L, vector &elements)
{
	// {#elements}
	lua_createtable(L, elements.size(), 0);

	for (std::size_t i = 0; i < elements.size(); i++) {
		// {#elements}, elements[i]
		lua_pushstring(L, elements[i].c_str());
		// {#elements} <- elements[i]
		lua_rawseti(L, -2, i + 1);
	}
}

static inline auto read_array(lua_State *L)
{
	auto elements = std::vector<std::filesystem::path>();
	// check first argument is a table
	luaL_checktype(L, 1, LUA_TTABLE);
	// {#elements}
	lua_settop(L, 1);
#if LUA_VERSION_NUM >= 502
	auto elements_size = lua_rawlen(L, 1);
#else
	auto elements_size = lua_objlen(L, 1);
#endif
	for (std::size_t i = 1; i <= elements_size; i++) {
		// {#paths_size} -> e
		lua_rawgeti(L, -1, i);
		// {#paths_size}, e
		elements.push_back(std::filesystem::path(luaL_checkstring(L, -1)));
		// {#paths_size}
		lua_pop(L, 1);
	}
	return elements;
}

/***
 * Get the paths of the default directories to be searched for dictionaries.
 *
 * @function get_default_dir_paths
 * @return table of directory paths
 */
static auto l_get_default_dir_paths(lua_State *L)
{
	auto path = std::vector<std::filesystem::path>();
	nuspell::append_default_dir_paths(path);
	return_array(L, path);
	return 1;
}

/***
 * Get the paths of the LibreOffice's directories to be searched for dictionaries.
 *
 * @warning This function shall not be called from LibreOffice or modules that may end up being used by LibreOffice. It is mainly intended to be used by the CLI tool.
 *
 * @function get_libreoffice_dir_paths
 * @return table of directory paths
 */
static auto l_get_libreoffice_dir_paths(lua_State *L)
{
	auto path = std::vector<std::filesystem::path>();
	nuspell::append_libreoffice_dir_paths(path);
	return_array(L, path);
	return 1;
}

/***
 * Search the directories for dictionaries.
 *
 * This function searches the directories for files that represent dictionaries and for each found dictionary it appends the path of the .aff file to @p dict_list. One dictionary consts of two files, .aff and .dic, and both need to exist, but only the .aff is added.
 *
 *
 * @function search_dirs_for_dicts
 * @param dir_paths table of directory paths
 * @return table of the found dictionaries
 */
static auto l_search_dirs_for_dicts(lua_State *L)
{
	auto dir_paths = read_array(L);
	auto dict_list = std::vector<std::filesystem::path>();
	nuspell::search_dirs_for_dicts(dir_paths, dict_list);
	return_array(L, dict_list);
	return 1;
}

/***
 * Search the default directories for dictionaries.
 * 
 * This is just a convenience that call two other functions.
 * @see get_default_dir_paths
 * @see search_dirs_for_dicts
 *
 * @function search_default_dirs_for_dicts
 * @return table of the found dictionaries
 */
static auto l_search_default_dirs_for_dicts(lua_State *L)
{
	auto dict_list = nuspell::search_default_dirs_for_dicts();
	return_array(L, dict_list);
	return 1;
}

/***
 * Serach the directories for only one dictionary.
 *
 * @function search_dirs_for_one_dict
 * @param dir_paths list of directories
 * @param dict_name_stem dictionary name, filename without extension (stem)
 * @return path to the .aff file of the dictionary or empty object if not found
 */
static auto l_search_dirs_for_one_dict(lua_State *L)
{
	const std::string dict_name = luaL_checkstring(L, 2);
	auto dict_list = read_array(L);
	auto finded = nuspell::search_dirs_for_one_dict(dict_list, dict_name);
	std::cout << dict_name << dict_list[0] << finded << std::endl;
	if (finded.empty()) {
		lua_pushnil(L);
	} else {
		lua_pushstring(L, finded.c_str());
	}
	return 1;
}

/***
 * Dictionary object
 * 
 * @type Dictionary
 */

static inline auto CheckDictionary(lua_State *L, int n)
{
	return *(nuspell::Dictionary **)luaL_checkudata(L, n, MT);
}

static auto l_Dictionary_destructor(lua_State *L)
{
	auto Dictionary = CheckDictionary(L, 1);
	delete Dictionary;
	return 0;
}
/**
 * Load the dictionary from file.
 *
 * @function load_aff_dic
 * @param aff_path path to .aff file. The path of .dic is inffered from this.
 * @return Dictionary
 * @raise `Dictionary_Loading_Error` on error
 */
static auto l_Dictionary_load_aff_dic(lua_State *L)
{
	const auto path = std::filesystem::path(luaL_checkstring(L, 1));
	auto dictionary = (nuspell::Dictionary **)lua_newuserdata(L, sizeof(nuspell::Dictionary *));
	*dictionary = new nuspell::Dictionary();
	try {
		(**dictionary).load_aff_dic(path);
	} catch (nuspell::Dictionary_Loading_Error e) {
		delete *dictionary;
		return luaL_error(L, e.what());
	}
	luaL_getmetatable(L, MT);
	lua_setmetatable(L, -2);
	return 1;
}

/***
 * Checks if a given word is correct.
 *
 * @function spell
 * @param word any word
 * @return true if correct, false otherwise
 */
static auto l_Dictionary_spell(lua_State *L)
{
	auto Dictionary = CheckDictionary(L, 1);
	const std::string word = luaL_checkstring(L, 2);
	lua_pushboolean(L, Dictionary->spell(word));
	return 1;
}

/***
 * Suggests correct words for a given incorrect word.
 *
 * @function suggest
 * @param word incorrect word
 * @return table populated with the suggestions
 */
static auto l_Dictionary_suggest(lua_State *L)
{
	auto Dictionary = CheckDictionary(L, 1);
	const std::string word = luaL_checkstring(L, 2);
	auto suggestions = std::vector<std::string>();
	Dictionary->suggest(word, suggestions);
	return_array(L, suggestions);
	return 1;
}

extern "C" auto luaopen_nuspell(lua_State *L) -> int
{
	static const luaL_Reg Finder_Regs[] = {
		{ "get_default_dir_paths", l_get_default_dir_paths },
		{ "get_libreoffice_dir_paths", l_get_libreoffice_dir_paths },
		{ "search_dirs_for_dicts", l_search_dirs_for_dicts },
		{ "search_default_dirs_for_dicts", l_search_default_dirs_for_dicts },
		{ "search_dirs_for_one_dict", l_search_dirs_for_one_dict },
		{ NULL, NULL }
	};

#if LUA_VERSION_NUM >= 502
	luaL_newlib(L, Finder_Regs);
#else
	luaL_register(L, PROJECT_NAME, Finder_Regs);
#endif

	static const luaL_Reg Dictionary_Regs[] = { { "load_aff_dic", l_Dictionary_load_aff_dic },
												{ "spell", l_Dictionary_spell },
												{ "suggest", l_Dictionary_suggest },
												{ "__gc", l_Dictionary_destructor },
												{ NULL, NULL } };

	// ...{} <- Dictionary_Regs
	luaL_newmetatable(L, MT);

#if LUA_VERSION_NUM >= 502
	luaL_setfuncs(L, Dictionary_Regs, 0);
#else
	luaL_register(L, NULL, Dictionary_Regs);
#endif

	// ...{}, {}
	lua_pushvalue(L, -1);
	// ...{}
	lua_setfield(L, -1, "__index");

	lua_setfield(L, -2, MT);

	lua_pushstring(L, VERSION);
	lua_setfield(L, -2, "_VERSION");
	lua_pushstring(L, PROJECT_NAME);
	lua_setfield(L, -2, "_NAME");

	return 1;
}
