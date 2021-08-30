#!/bin/bash
set -eo pipefail

do_tests() {
	echo
	cd spec
	echo 'Testing lua-nuspell'
	./test.lua
	cd ..
}

echo "===== Setting LuaRocks PATH ====="
eval "$(luarocks path)"

if lua -e 'require("nuspell")' 2>/dev/null; then
	cat <<EOT
Please ensure you do not have the lua-nuspell module installed before
running these tests.
EOT
	exit 1
fi

echo "===== Testing LuaRocks build ====="
luarocks make --local
do_tests
luarocks remove --local lua-nuspell
