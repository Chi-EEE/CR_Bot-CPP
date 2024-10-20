add_rules("mode.debug", "mode.release")

add_requires("boost", {configs = {iostreams = true}})
add_requires("spdlog", {configs = {fmt_external = true}})
add_requires("fmt")
add_requires("tobiaslocker_base64")

target("emulator", function() 
    set_kind("$(kind)")
    set_languages("c++17")
    set_group("other")

    add_packages("boost", "spdlog", "fmt", "tobiaslocker_base64", {public = true})
    
    add_headerfiles("include/(emulator/**.h)")
    add_files("src/**.cpp")
    add_includedirs("include", {public = true})

    add_deps("av", {public = true})
end)