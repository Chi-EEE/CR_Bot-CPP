add_rules("mode.debug", "mode.release")

add_requires("opencv", "ffmpeg 7.0.2", {configs = {debug = false}})

add_syslinks("kernel32", "shell32")

target("av", function() 
    set_kind("$(kind)")
    set_languages("c++17")

    add_packages("opencv", "ffmpeg", {public = true})
    
    add_headerfiles("include/(av/**.h)", "include/(av/**.hpp)")
    add_files("src/**.cpp")
    add_includedirs("include", {public = true})
end)