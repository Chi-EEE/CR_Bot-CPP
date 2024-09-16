add_rules("mode.debug", "mode.release")

add_requires("doctest")

for _, file in ipairs(os.files("*.cpp")) do
    local name = path.basename(file)
    target(name, function()
        set_kind("binary")
        set_default(false)
        set_languages("c++17")
        set_group("tests")
        add_packages("doctest")
        
        if is_plat("windows") then
            add_defines("WIN32")
            add_defines("NOMINMAX")
            add_cxflags("/utf-8")
        end
        
        add_files(name .. ".cpp")

        add_deps("CR_Bot.Common", {public = true})
    end)
end