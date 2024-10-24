add_rules("mode.debug", "mode.release")

add_requires("opencv", "ffmpeg", {configs = {debug = false}})

add_syslinks("kernel32", "shell32")

target("av", function() 
    set_kind("$(kind)")
    set_languages("c++17")
    set_group("other")

    add_packages("opencv", "ffmpeg", {public = true})
    
    add_headerfiles("include/(av/**.h)", "include/(av/**.hpp)")
    add_files("src/**.cpp")
    add_includedirs("include", {public = true})
end)