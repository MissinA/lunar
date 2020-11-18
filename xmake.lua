-- add rules: debug/release
add_rules("mode.debug", "mode.release")

target("lunar")
    set_languages("cxx20")
    set_kind("shared")
    add_defines("TRAY_WINAPI")
    add_linkdirs("./script/microsoft.web.webview2.0.9.488/build/native/x86", "E:/env/luajit")
    add_includedirs("./script", "E:/env/luajit/include")
    add_links("WebView2Loader.dll", "lua51", "user32", "shell32")
    add_files("lunar.cc")

target("tray_test")
    set_kind("binary")
    add_links("user32", "shell32")
    add_files("traytest.c")

target("tray")
    set_languages("cxx20")
    set_targetdir("C:/Users/chen/Desktop")
    set_kind("shared")
    add_links("user32", "shell32")
    add_files("./extention/main.cc")