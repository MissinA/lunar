#include <lua.hpp>
#include <stdio.h>
#include <string.h>
#include "webview.h"

#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__) || defined(linux) || defined(__linux)
#define TRAY_APPINDICATOR 1
#elif defined(__APPLE__) || defined(__MACH__)
#define TRAY_APPKIT 1
#endif

const char *BINDTABLE = "JsBindFn";
lua_State *GL = NULL; // global

typedef struct WebviewLua
{
    webview_t wb;
    int hints;
    int width;
    int height;
} WebviewLua;

typedef struct BindArg
{
    char name[256];
    webview_t wb;
} BindArg;

static inline webview_t get_w(lua_State *L)
{
    WebviewLua *webview = (WebviewLua *)lua_touserdata(L, 1);
    return webview->wb;
}

static inline WebviewLua *get_lua_webview(lua_State *L)
{
    WebviewLua *webview = (WebviewLua *)lua_touserdata(L, 1);
    return webview;
}

void get_bindfn(const char *fn_name)
{
    lua_pushstring(GL, BINDTABLE);
    lua_gettable(GL, LUA_REGISTRYINDEX);
    lua_pushstring(GL, fn_name);
    lua_gettable(GL, -2);
}

/* bind func */
void _webview_binding_cb(const char *id, const char *req, void *arg)
{
    BindArg *argptr = (BindArg *)arg;
    const char *cb_name = argptr->name;
    get_bindfn(cb_name);
    lua_pushstring(GL, req);
    lua_pcall(GL, 1, 1, 0);
    const char *result = lua_tostring(GL, -1);
    // printf("%s ------- %p ----- %s\n", id, argptr->wb, result);
    webview_return(argptr->wb, id, 0, result);
}

static int lua_webview_bind(lua_State *L)
{
    webview_t w = get_w(L);
    const char *cb_name = lua_tostring(L, 2);
    lua_pushstring(L, BINDTABLE);
    lua_gettable(L, LUA_REGISTRYINDEX);

    BindArg *arg = (BindArg *)(malloc(sizeof(BindArg)));
    strcpy(arg->name, cb_name);
    arg->wb = w;

    if (lua_isfunction(L, 3))
    {
        lua_pushstring(L, cb_name);
        lua_pushvalue(L, 3);
        lua_settable(L, -3);
        webview_bind(w, cb_name, &_webview_binding_cb, arg);
    }

    return 0;
}

static int lweb_new(lua_State *L)
{
    GL = L;
    WebviewLua *lua_webview = (WebviewLua *)lua_newuserdata(L, sizeof(WebviewLua));
    //set metatable
    luaL_getmetatable(L, "Webview.wb");
    lua_setmetatable(L, -2);
    const char *title = luaL_checkstring(L, 1);
    bool borderless = lua_toboolean(L, 2);
    bool debug = lua_toboolean(L, 3);
    lua_webview->wb = webview_create(debug, nullptr);
    webview_set_title(lua_webview->wb, title);
    if (borderless)
    {
        #if defined(_WIN32)
            webview_set_size(lua_webview->wb, 500, 500, 4);
            lua_webview->hints = 4;
        #endif
    }
    // webview_navigate(lua_webview->wb, url);
    return 1;
};

static int lua_webview_set_title(lua_State *L)
{
    webview_t w = get_w(L);
    const char *title = luaL_checkstring(L, 2);
    webview_set_title(w, title);
    return 0;
}

static int lua_webview_run(lua_State *L)
{
    webview_t w = get_w(L);
    webview_run(w);
    return 0;
};

static int lua_webview_navigate(lua_State *L)
{
    webview_t w = get_w(L);
    const char *url = luaL_checkstring(L, 2);
    webview_navigate(w, url);
    return 0;
};

static int lua_webview_return(lua_State *L)
{
    webview_t w = get_w(L);
    const char *seq = luaL_checkstring(L, 2);
    const char *result = luaL_optstring(L, 3, NULL);
    webview_return(w, seq, 0, result);
    return 0;
}

static int lua_webview_size(lua_State *L)
{
    webview_t w = get_w(L);
    WebviewLua *lua_webview_struct = get_lua_webview(L);
    int width = luaL_checkint(L, 2);
    int height = luaL_checkint(L, 3);
    // const char *shits[] = {"none", "min", "max", "fixed"};
    int hints = luaL_optinteger(L, 4, lua_webview_struct->hints);
    lua_webview_struct->width = width;
    lua_webview_struct->height = height;

    webview_set_size(w, width, height, hints);

    lua_pushvalue(L, 1);
    return 1;
}

static int lua_webview_getwin(lua_State *L)
{
    webview_t w = get_w(L);
    void *hwnd = webview_get_window(w);
    lua_pushinteger(L, int(hwnd));
    return 1;
}

static int lua_webview_terminate(lua_State *L)
{
    webview_t w = get_w(L);
    webview_terminate(w);
    return 0;
}

static int lua_webview_eval(lua_State *L)
{
    webview_t w = get_w(L);
    const char *code = lua_tostring(L, 2);
    webview_eval(w, code);
    return 0;
}

static int lua_webview_destroy(lua_State *L)
{
    webview_t w = get_w(L);
    webview_destroy(w);
}

static int lua_webview_move(lua_State *L)
{
    webview_t w = get_w(L);
    void *hwnd = webview_get_window(w);
    SendMessageA((HWND)hwnd, 161, 2, 0);
    return 0;
}

// tray extention begin
static void hello_cb(struct tray_menu *item)
{
    printf("cb");
}

static void quit_cb(struct tray_menu *item)
{
    webview_t w = get_w(GL);
    webview_destroy(w);
}

// tray extention end

luaL_Reg lua_webviewlib[] = {
    {"new", lweb_new},
    {NULL, NULL}};

luaL_Reg lua_webview_method[] = {
    {"run", lua_webview_run},
    {"setTitle", lua_webview_set_title},
    {"navigate", lua_webview_navigate},
    {"bind", lua_webview_bind},
    {"returns", lua_webview_return},
    {"size", lua_webview_size},
    {"hwnd", lua_webview_getwin},
    {"exit", lua_webview_terminate},
    {"eval", lua_webview_eval},
    {"close", lua_webview_destroy},
    {"move", lua_webview_move},
    {NULL, NULL}};

static void lua_webview_init(lua_State *L)
{
    // init the cb table
    lua_pushstring(L, BINDTABLE);
    lua_newtable(L);
    lua_settable(L, LUA_REGISTRYINDEX);
    // create the mt_table
    luaL_newmetatable(L, "Webview.wb");
    //setmetatabel(Webview.wb,{__index = Webview.wb})
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_settable(L, -3);
    //fill the method
    luaL_openlib(L, NULL, lua_webview_method, 0);
}

extern "C" __declspec(dllexport) int luaopen_lunar(lua_State *L)
{
    lua_webview_init(L);
    luaL_openlib(L, "lunar", lua_webviewlib, 0);
    return 1;
}
