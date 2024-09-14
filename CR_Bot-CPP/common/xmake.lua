add_rules("mode.debug", "mode.release")

add_requires("platform-tools 35.0.0")
add_requires("onnxruntime 1.19.2")
add_requires("boost", {configs = {iostreams = true}})
add_requires("ffmpeg 7.0.2")
add_requires("opencv")
add_requires("toml++")
add_requires("tobiaslocker_base64")
add_requires("tl_expected")

add_requires("fmt")
add_requires("spdlog", {configs = {fmt_external = true}})

target("common", function()
    set_kind("headeronly")
    set_languages("c++17")

    add_packages("platform-tools", {public = true})
    add_packages("onnxruntime", {public = true})
    add_packages("boost", {public = true})
    add_packages("ffmpeg", "opencv", {public = true})
    add_packages("toml++", {public = true})
    add_packages("tobiaslocker_base64", {public = true})
    add_packages("tl_expected", {public = true})
    add_packages("fmt", "spdlog", {public = true})

    add_headerfiles("include/(**.hpp)")
    add_includedirs("include", {public = true})

    add_deps("av", {public = true})
end)
