package("platform-tools")
    set_kind("binary")
    set_homepage("https://developer.android.com/tools/releases/platform-tools")
    set_description("Contains ADB, fastboot, and other platform tools.")
    if is_host("windows") then
        add_urls("https://dl.google.com/android/repository/platform-tools_r$(version)-windows.zip")
    elseif is_host("linux") then
        add_urls("https://dl.google.com/android/repository/platform-tools_r$(version)-linux.zip")
    elseif is_host("macosx") then
        add_urls("https://dl.google.com/android/repository/platform-tools_r$(version)-darwin.zip")
    end

    on_install("windows", "macosx", "linux", function (package)
        os.cp("*", package:installdir("bin"))
        package:addenv("PATH", "bin")
    end)

    on_test(function (package)
        os.vrun("adb --version")
    end)
