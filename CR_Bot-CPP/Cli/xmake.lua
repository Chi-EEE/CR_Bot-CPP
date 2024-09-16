add_rules("mode.debug", "mode.release")

add_requires("boost", {configs = {iostreams = true}})
add_requires("spdlog", {configs = {fmt_external = true}})

target("CR_Bot.Cli", function()
    set_kind("binary")
    set_languages("c++17")
    add_packages("boost")
    add_packages("fmt")
    add_packages("spdlog")

    if is_plat("windows") then
        add_defines("WIN32")
        add_defines("NOMINMAX")
        add_cxflags("/utf-8")
    end

    add_files("src/**.cpp")

    add_defines("BOOST_ASIO_DISABLE_CONCEPTS")

    add_configfiles("(constants/**.toml)", {onlycopy = true})
    add_configfiles("(config.toml)", {onlycopy = true})
    add_configfiles("(assets/**.png)", {onlycopy = true})
    set_configdir("$(buildir)/$(plat)/$(arch)/$(mode)")

    add_deps("CR_Bot.Common", {public = true})
end)