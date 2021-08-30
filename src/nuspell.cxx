/***
 * lua-nuspell is a set of Lua 5.x bindings for Nuspell spellchecking C++ library.
 *
 * @module nuspell
 */
#include <nuspell/finder.hxx>
#include <nuspell/dictionary.hxx>

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#define MT "Dictionary"

static inline auto return_array_of_pairs(lua_State *L,
										 std::vector<std::pair<std::string, std::string>> &pairs)
{
	// {#pairs}
	lua_createtable(L, pairs.size(), 0);
	for (std::size_t i = 0; i < pairs.size(); i++) {
		// {#pairs}, {2}
		lua_createtable(L, 2, 0);
		// {#pairs}, {2}, pairs[i].first
		lua_pushstring(L, pairs[i].first.c_str());
		// {#pairs}, {2} <- pairs[i].first
		lua_rawseti(L, -2, 1);
		// {#pairs}, {2}, pairs[i].second
		lua_pushstring(L, pairs[i].second.c_str());
		// {#pairs}, {2} <- pairs[i].second
		lua_rawseti(L, -2, 2);
		// {#pairs} <- {2}
		lua_rawseti(L, -2, i + 1);
	}
}

static inline auto return_array(lua_State *L, std::vector<std::string> &elements)
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
	auto elements = std::vector<std::string>();
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
		elements.push_back(luaL_checkstring(L, -1));
		// {#paths_size}
		lua_pop(L, 1);
	}
	return elements;
}

static inline auto read_array_of_pairs(lua_State *L)
{
	auto pairs = std::vector<std::pair<std::string, std::string>>();
	auto pair = std::pair<std::string, std::string>();
	// check first argument is a table
	luaL_checktype(L, 1, LUA_TTABLE);
	// {#pairs}
	lua_settop(L, 1);
#if LUA_VERSION_NUM >= 502
	auto pairs_size = lua_rawlen(L, 1);
#else
	auto pairs_size = lua_objlen(L, 1);
#endif
	for (std::size_t i = 1; i <= pairs_size; i++) {
		// {#pairs_size} -> {2}
		lua_rawgeti(L, -1, i);
		// {#pairs_size}, {2} -> first
		lua_rawgeti(L, -1, 1);
		// {#pairs_size}, {2}, first
		pair.first = luaL_checkstring(L, -1);
		// {#pairs_size}, {2}
		lua_pop(L, 1);
		// {#pairs_size}, {2} -> second
		lua_rawgeti(L, -1, 2);
		// {#pairs_size}, {2}, second
		pair.second = luaL_checkstring(L, -1);
		// {#pairs_size}, {2}
		lua_pop(L, 1);
		// {#pairs_size}
		lua_pop(L, 1);
		pairs.push_back(pair);
	}
	return pairs;
}

/***
 * Get the paths of the default directories to be searched for dictionaries.
 *
 * @function get_default_dir_paths
 * @return `dir_paths` table of directory paths
 */
static auto l_get_default_dir_paths(lua_State *L)
{
	auto path = std::vector<std::string>();
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
 * @return `dir_paths` table of directory paths
 */
static auto l_get_libreoffice_dir_paths(lua_State *L)
{
	auto path = std::vector<std::string>();
	nuspell::append_libreoffice_dir_paths(path);
	return_array(L, path);
	return 1;
}

/***
 * Search the directories for dictionaries.
 *
 * This function searches the directory for files that represent a dictionary and for each one found it appends the pair of dictionary name and filepath to dictionary, both without the filename extension (.aff or .dic).
 *
 * For example for the files /dict/dir/en_US.dic and /dict/dir/en_US.aff the following pair will be appended ("en_US", "/dict/dir/en_US").
 *
 * @function search_dirs_for_dicts
 * @param dir_paths table of directory paths
 * @return `dict_list` table of the found dictionaries
 */
static auto l_search_dirs_for_dicts(lua_State *L)
{
	auto dir_paths = read_array(L);
	auto dict_list = std::vector<std::pair<std::string, std::string>>();
	nuspell::search_dirs_for_dicts(dir_paths, dict_list);
	return_array_of_pairs(L, dict_list);
	return 1;
}

/***
 * Search the default directories for dictionaries.
 *
 * @see get_default_dir_paths
 * @see search_dirs_for_dicts
 *
 * @function search_default_dirs_for_dicts
 * @return `dict_list` table of the found dictionaries
 */
static auto l_search_default_dirs_for_dicts(lua_State *L)
{
	auto dict_list = std::vector<std::pair<std::string, std::string>>();
	nuspell::search_default_dirs_for_dicts(dict_list);
	return_array_of_pairs(L, dict_list);
	return 1;
}

/***
 * Find dictionary path given the name.
 *
 * Find the first dictionary whose name matches p dict_name.
 *
 * @function find_dictionary
 * @param dict_list table of the found dictionaries
 * @param dict_name dictionary name
 * @return `dict_path` path of the found dictionary or nil if not found
 */
static auto l_find_dictionary(lua_State *L)
{
	const std::string dict_name = luaL_checkstring(L, 2);
	auto dict_list = read_array_of_pairs(L);
	auto finded = nuspell::find_dictionary(dict_list, dict_name);
	if (end(dict_list) == finded) {
		lua_pushnil(L);
	} else {
		lua_pushstring(L, finded->second.c_str());
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
/***
 * Create a dictionary from files
 *
 * @static
 * @function load_from_path
 * @param file_path_without_extension path *without* extensions (without .dic or
 * .aff)
 * @return `Dictionary`
 * @raise `Dictionary_Loading_Error` on error
 */
static auto l_Dictionary_load_from_path(lua_State *L)
{
	const std::string file_path_without_extensions = luaL_checkstring(L, 1);
	auto dictionary = (nuspell::Dictionary **)lua_newuserdata(L, sizeof(nuspell::Dictionary *));
	*dictionary = new nuspell::Dictionary();
	try {
		**dictionary = nuspell::Dictionary::load_from_path(file_path_without_extensions);
	} catch (nuspell::Dictionary_Loading_Error e) {
		delete *dictionary;
		return luaL_error(L, e.what());
	}
	luaL_getmetatable(L, MT);
	lua_setmetatable(L, -2);
	return 1;
}

/***
 * Checks if a given word is correct
 *
 * @function spell
 * @param word any word
 * @return `is_correct` true if correct, false otherwise
 */
static auto l_Dictionary_spell(lua_State *L)
{
	auto Dictionary = CheckDictionary(L, 1);
	const std::string word = luaL_checkstring(L, 2);
	lua_pushboolean(L, Dictionary->spell(word));
	return 1;
}

/***
 * Suggests correct words for a given incorrect word
 *
 * @function suggest
 * @param word incorrect word
 * @return `suggestions` table populated with the suggestions
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
		{ "find_dictionary", l_find_dictionary },
		{ NULL, NULL }
	};

#if LUA_VERSION_NUM >= 502
	luaL_newlib(L, Finder_Regs);
#else
	luaL_register(L, "nuspell", Finder_Regs);
#endif

	static const luaL_Reg Dictionary_Regs[] = { { "load_from_path", l_Dictionary_load_from_path },
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

	return 1;
}
