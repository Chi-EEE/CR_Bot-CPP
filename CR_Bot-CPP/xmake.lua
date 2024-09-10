add_rules("mode.debug", "mode.release")

add_requires("platform-tools 35.0.0")
add_requires("onnxruntime 1.19.2")
add_requires("openh264")
add_requires("ffmpeg 7.0.2", {configs = {debug = false}})
add_requires("opencv", {configs = {debug = false}})
add_requires("libjpeg", "libpng")
add_requires("boost", {configs = {iostreams = true}})
add_requires("toml++")
add_requires("fmt")
add_requires("tobiaslocker_base64")
-- add_requires("sfml", "imgui-sfml v2.6")

add_syslinks("kernel32", "shell32")

target("CR_Bot-CPP", function()
    set_kind("binary")
    set_languages("c++17")
    add_packages("platform-tools")
    add_packages("onnxruntime")
    add_packages("openh264", "ffmpeg", "opencv")
    add_packages("libjpeg", "libpng")
    add_packages("boost")
    add_packages("toml++")
    add_packages("fmt")
    add_packages("tobiaslocker_base64")

    add_rules("utils.bin2c", {extensions = {".bash"}})
    add_files("embed/screen_record.bash")

    if is_plat("windows") then
        add_defines("WIN32")
        add_defines("NOMINMAX")
    end
    -- add_packages("sfml", "imgui-sfml")

    add_files("src/**.cpp")
    add_headerfiles("src/**.h", "src/**.hpp")

    add_defines("BOOST_ASIO_DISABLE_CONCEPTS")

    add_configfiles("config.toml")
    set_configdir("$(buildir)/$(plat)/$(arch)/$(mode)")

    -- set_runargs("c:/Users/admin/Documents/GitHub/ClashRoyaleBuildABot/clashroyalebuildabot/models/units_M_480x352.onnx")
end)
