add_rules("mode.debug", "mode.release")

add_requires("sol2", {configs = {includes_lua = false}})
add_requires("luajit")

target("CR_Bot.Lua", function()
    set_kind("binary")
    set_default(false)
    set_languages("c++17")
    
    add_packages("sol2", "luajit", {public = true})

    add_files("src/*.cpp")

    if is_plat("windows") then
        add_defines("WIN32")
        add_defines("NOMINMAX")
        add_cxflags("/utf-8")
    end

    add_deps("CR_Bot.Common", {public = true})
end)