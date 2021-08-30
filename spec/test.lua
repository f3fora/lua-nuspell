#!/usr/bin/env lua

local nuspell = require('nuspell')
local util = require('util')

local function dictionary(word, lang)
    local dirs = nuspell.search_default_dirs_for_dicts()
    local dict = nuspell.Dictionary.load_from_path(nuspell.find_dictionary(dirs, lang))
    return dict:spell(word), dict:suggest(word)
end

local tests = {
    --[[
    function my_function(input)
        return output
    end

    should_work -> output == expected
    {
        'title',
        my_function,
        { input },
        should_work,
        { expected },
    }
    --]]
    {
        'get_libreoffice_dir_paths',
        nuspell.get_libreoffice_dir_paths,
        {},
        true,
        { {} },
    },
    {
        'search_dirs_for_dicts',
        nuspell.search_dirs_for_dicts,
        { nuspell.get_default_dir_paths() },
        true,
        { nuspell.search_default_dirs_for_dicts() },
    },
    {
        'find_dictionary',
        nuspell.find_dictionary,
        { { { 'a', 'b' }, { 'c', 'd' }, { 'a', 'e' } }, 'a' },
        true,
        { 'b' },
    },
    {
        'load_from_path',
        nuspell.Dictionary.load_from_path,
        { 'missing_path' },
        false,
        { 'Aff file missing_path.aff not found' },
    },
    {
        'Dictionary',
        dictionary,
        { 'true', 'en_US' },
        true,
        { true, { 'rue', 'trues', 'truer', 'truce', 'trued', 'tree', 'grue', 'trug', 't rue' } },
    },
}

print('==> Testing lua-nuspell\n')

util.run_test_group(tests)

local pass, total = util.run_test_summary()

if pass == total then
    print('==> Summary: all tests succeeded')
else
    print(('==> Summary: %d/%d tests failed'):format(total - pass, total))
    os.exit(1)
end
