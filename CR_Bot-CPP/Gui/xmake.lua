add_rules("mode.debug", "mode.release")

target("CR_Bot.Gui", function()
    set_kind("binary")
    set_default(false)
    set_languages("c++17")
    
    if is_plat("windows") then
        add_defines("WIN32")
        add_defines("NOMINMAX")
        add_cxflags("/utf-8")
    end
    
    add_deps("CR_Bot.Common", {public = true})
end)