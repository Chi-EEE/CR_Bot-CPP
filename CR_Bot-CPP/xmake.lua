add_rules("mode.debug", "mode.release")

add_requires("platform-tools 35.0.0")
add_requires("onnxruntime 1.19.2")
add_requires("openh264")
add_requires("boost")
add_requires("toml++")
add_requires("fmt")
-- add_requires("sfml", "imgui-sfml v2.6")

add_syslinks("kernel32")

target("CR_Bot-CPP", function()
    set_kind("binary")
    set_languages("c++17")
    add_packages("platform-tools")
    add_packages("onnxruntime")
    add_packages("openh264")
    add_packages("boost")
    add_packages("toml++")
    add_packages("fmt")

    if is_plat("windows") then
        add_defines("WIN32")
        add_defines("NOMINMAX")
    end
    -- add_packages("sfml", "imgui-sfml")

    add_files("src/*.cpp")
    add_headerfiles("src/*.h")

    add_configfiles("config.toml")
    set_configdir("$(buildir)/$(plat)/$(arch)/$(mode)")

    -- set_runargs("c:/Users/admin/Documents/GitHub/ClashRoyaleBuildABot/clashroyalebuildabot/models/units_M_480x352.onnx")
end)
