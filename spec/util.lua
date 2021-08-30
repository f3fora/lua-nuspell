-- testing utility
-- Thanks to
-- https://github.com/openresty/lua-cjson
local M = {}

local unpack = unpack or table.unpack

local maxn = table.maxn
    or function(t)
        local max = 0
        for k, _ in pairs(t) do
            if type(k) == 'number' and k > max then
                max = k
            end
        end
        return max
    end

local _one_of_mt = {}

local function is_one_of(t)
    return type(t) == 'table' and getmetatable(t) == _one_of_mt
end

local function is_array(table)
    local max = 0
    local count = 0
    for k, _ in pairs(table) do
        if type(k) == 'number' then
            if k > max then
                max = k
            end
            count = count + 1
        else
            return -1
        end
    end
    if max > count * 2 then
        return -1
    end

    return max
end

local serialise_value

local function serialise_table(value, indent, depth)
    local spacing, spacing2, indent2
    if indent then
        spacing = '\n' .. indent
        spacing2 = spacing .. '  '
        indent2 = indent .. '  '
    else
        spacing, spacing2, indent2 = ' ', ' ', false
    end
    depth = depth + 1
    if depth > 50 then
        return 'Cannot serialise any further: too many nested tables'
    end

    local max = is_array(value)

    local comma = false
    local prefix = '{'
    if is_one_of(value) then
        prefix = 'ONE_OF{'
    end
    local fragment = { prefix .. spacing2 }
    if max > 0 then
        -- Serialise array
        for i = 1, max do
            if comma then
                table.insert(fragment, ',' .. spacing2)
            end
            table.insert(fragment, serialise_value(value[i], indent2, depth))
            comma = true
        end
    elseif max < 0 then
        -- Serialise table
        for k, v in pairs(value) do
            if comma then
                table.insert(fragment, ',' .. spacing2)
            end
            table.insert(
                fragment,
                ('[%s] = %s'):format(serialise_value(k, indent2, depth), serialise_value(v, indent2, depth))
            )
            comma = true
        end
    end
    table.insert(fragment, spacing .. '}')

    return table.concat(fragment)
end

function serialise_value(value, indent, depth)
    if indent == nil then
        indent = ''
    end
    if depth == nil then
        depth = 0
    end

    if type(value) == 'string' then
        return ('%q'):format(value)
    elseif type(value) == 'nil' or type(value) == 'number' or type(value) == 'boolean' then
        return tostring(value)
    elseif type(value) == 'table' then
        return serialise_table(value, indent, depth)
    else
        return '"<' .. type(value) .. '>"'
    end
end

local function compare_values(val1, val2)
    if is_one_of(val2) then
        for _, option in ipairs(val2) do
            if compare_values(val1, option) then
                return true
            end
        end
        return false
    end

    local type1 = type(val1)
    local type2 = type(val2)
    if type1 ~= type2 then
        return false
    end

    -- Check for NaN
    if type1 == 'number' and val1 ~= val1 and val2 ~= val2 then
        return true
    end

    if type1 ~= 'table' then
        return val1 == val2
    end

    -- check_keys stores all the keys that must be checked in val2
    local check_keys = {}
    for k, _ in pairs(val1) do
        check_keys[k] = true
    end

    for k, _ in pairs(val2) do
        if not check_keys[k] then
            return false
        end

        if not compare_values(val1[k], val2[k]) then
            return false
        end

        check_keys[k] = nil
    end
    for _, _ in pairs(check_keys) do
        -- Not the same if any keys from val1 were not found in val2
        return false
    end
    return true
end

local test_count_pass = 0
local test_count_total = 0

function M.run_test_summary()
    return test_count_pass, test_count_total
end

function M.run_test(testname, func, input, should_work, output)
    local function status_line(name, status, value)
        local statusmap = { [true] = ':success', [false] = ':error' }
        if status ~= nil then
            name = name .. statusmap[status]
        end
        print(('[%s] %s'):format(name, serialise_value(value, false)))
    end

    local result = {}
    local tmp = { pcall(func, unpack(input)) }
    local success = tmp[1]
    for i = 2, maxn(tmp) do
        result[i - 1] = tmp[i]
    end

    local correct = false
    if success == should_work and compare_values(result, output) then
        correct = true
        test_count_pass = test_count_pass + 1
    end
    test_count_total = test_count_total + 1

    local teststatus = { [true] = 'PASS', [false] = 'FAIL' }
    print(('==> Test [%d] %s: %s'):format(test_count_total, testname, teststatus[correct]))

    status_line('Input', nil, input)
    if not correct then
        status_line('Expected', should_work, output)
    end
    status_line('Received', success, result)
    print()

    return correct, result
end

function M.run_test_group(tests)
    local function run_helper(name, func, input)
        if type(name) == 'string' and #name > 0 then
            print('==> ' .. name)
        end
        -- Not a protected call, these functions should never generate errors.
        func(unpack(input or {}))
        print()
    end

    for _, v in ipairs(tests) do
        -- Run the helper if "should_work" is missing
        if v[4] == nil then
            run_helper(unpack(v))
        else
            M.run_test(unpack(v))
        end
    end
end

-- Export functions
return M
