add_rules("mode.debug", "mode.release")

add_requires("boost", {configs = {iostreams = true}})
add_requires("spdlog", {configs = {fmt_external = true}})
-- add_requires("sfml", "imgui-sfml v2.6")

includes("common/xmake.lua", "av/xmake.lua")

target("CR_Bot-CPP", function()
    set_kind("binary")
    set_languages("c++17")
    add_packages("boost")
    add_packages("fmt")
    add_packages("spdlog")

    add_rules("utils.bin2c", {extensions = {".bash"}})
    add_files("embed/screen_record.bash")

    if is_plat("windows") then
        add_defines("WIN32")
        add_defines("NOMINMAX")
        add_cxflags("/utf-8")
    end

    add_files("src/**.cpp")

    add_defines("BOOST_ASIO_DISABLE_CONCEPTS")

    add_configfiles("(constants/**.toml)")
    add_configfiles("(config.toml)")
    set_configdir("$(buildir)/$(plat)/$(arch)/$(mode)")

    add_deps("common", {public = true})
end)

add_requires("doctest")
for _, file in ipairs(os.files("tests/*.cpp")) do
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
        
        add_files("tests/" .. name .. ".cpp")

        add_deps("common", {public = true})
    end)
end