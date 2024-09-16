add_rules("mode.debug", "mode.release")

includes(
    "common/xmake.lua", 
    "gui/xmake.lua", 
    "lua/xmake.lua", 
    "cli/xmake.lua", 
    "tests/xmake.lua",
    "other/xmake.lua"
)

add_configfiles("(constants/**.toml)", {onlycopy = true})
add_configfiles("(config.toml)", {onlycopy = true})
add_configfiles("(assets/**.png)", {onlycopy = true})
set_configdir("$(buildir)/$(plat)/$(arch)/$(mode)")