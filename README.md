# lua-nuspell

`lua-nuspell` is a set of Lua 5.x bindings for [Nuspell](https://nuspell.github.io/) spellchecking C++ library.

## About Nuspell

> Nuspell is a fast and safe spelling checker software program. It is designed for languages with rich morphology and complex word compounding.Nuspell is written in modern C++ and it supports Hunspell dictionaries.
> 
> Main features of Nuspell spelling checker:
> 
> - Provides software library and command-line tool.
> - Suggests high-quality spelling corrections.
> - Backward compatibility with Hunspell dictionary file format.
> - Up to 3.5 times faster than Hunspell.
> - Full Unicode support backed by ICU.
> - Twofold affix stripping (for agglutinative languages, like Azeri, Basque, Estonian, Finnish, Hungarian, Turkish, etc.).
> - Supports complex compounds (for example, Hungarian, German and Dutch).
> - Supports advanced features, for example: special casing rules (Turkish dotted i or German sharp s), conditional affixes, circumfixes, fogemorphemes, forbidden words, pseudoroots and homonyms.
> - Free and open source software. Licensed under GNU LGPL v3 or later.

## Status

[![Test](https://github.com/f3fora/lua-nuspell/actions/workflows/test.yaml/badge.svg)](https://github.com/f3fora/lua-nuspell/actions)

Tested only on Linux. Feedback wanted!

## Requirements

- nuspell

## Installation 

### from [LuaRocks](https://luarocks.org/modules/f3fora/lua-nuspell)
```
luarocks install lua-nuspell
```

### from RockSpec

Clone and build.
```
luarocks make --local
```

### from Source
Clone and compile for example for Lua 5.1.
```
g++ -fPIC -shared -lnuspell -std=c++17 -I/usr/include/lua5.1/ -o nuspell.so src/nuspell.cxx
```

## Documentation

Documentation is available with 
```
luarocks doc lua-nuspell
```

Otherwise, it can be generated with [LDoc](https://github.com/lunarmodules/LDoc) by running
```
ldoc .
```
and the result can be viewed by opening `doc/index.html` in a web browser.

### Minimal Example

```lua
local nuspell = require('nuspell')
local unpack = table.unpack or unpack
local word = 'Hello'
local lang = 'en_US'

local dirs = nuspell.search_default_dirs_for_dicts()
local dict = nuspell.Dictionary.load_from_path(nuspell.find_dictionary(dirs, lang))
print(dict:spell(word))
print(unpack(dict:suggest(word)))
```

## License

`lua-nuspell` is licensed under the LGPL version 3 or later, see [LICENSE](LICENSE) for more
information.
