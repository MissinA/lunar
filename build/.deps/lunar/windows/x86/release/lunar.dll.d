{
    files = {
        [[build\.objs\lunar\windows\x86\release\lunar.cc.obj]]
    },
    values = {
        [[C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.27.29110\bin\HostX86\x86\link.exe]],
        {
            "-nologo",
            "-machine:x86",
            [[-libpath:script\microsoft.web.webview2.0.9.488\build\native\x86]],
            [[-libpath:E:\env\luajit]],
            "/opt:ref",
            "/opt:icf",
            "/ltcg",
            "WebView2Loader.dll.lib",
            "lua51.lib",
            "user32.lib",
            "shell32.lib"
        }
    }
}