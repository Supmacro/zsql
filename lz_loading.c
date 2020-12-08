
#include "lz_loading.h"

#include <stdio.h>
#include <stdlib.h>


static void luaz_set_paths(lua_State *L)
{
    /*
     * Push the value in the global variable 'package' on 
     * the stack and return the type of the value
     * */
    lua_getglobal(L, "package");

    /* Returns the index of the top element of the stack. 
     * Because the index is numbered from 1, 
     * the result is equal to the number of elements 
     * on the stack; 
     * in particular, 0 means the stack is empty */
    int top = lua_gettop(L);

    /* This macro is equivalent to lua_pushlstring, 
     * except that it can only be used when s is a literal. 
     * It will automatically give the length of the string */
    lua_pushliteral(L, "./?.lua;");
    lua_pushliteral(L, "./?/init.lua;");

    const char *home = getenv("HOME");
    if(home)
    {
        lua_pushstring(L, home);
        lua_pushstring(L, "/.luarocks/share/lua/5.1/?.lua;");
        lua_pushstring(L, home);
        lua_pushstring(L, "/.luarocks/share/lua/?.lua;");
    }

    lua_pushliteral(L, "/usr/local/share/lua/5.1/?.lua;");
    lua_pushliteral(L, "/usr/share/lua/5.1/?.lua;");

    /* Connect the n values at the top of the stack, 
     * then pop these values from the stack, 
     * and put the result on the top of the stack */
    lua_concat(L, lua_gettop(L) - top);

    const char *lua_path = getenv("LUA_PATH");
    if(lua_path)
    {
        const char *def = lua_tostring(L, -1); 
        lua_path = luaL_gsub(L, lua_path, ";;", ";\1;");
        luaL_gsub(L, lua_path, "\1", def);
        lua_remove(L, -2);
        lua_remove(L, -2);
    }

    /* Do an operation equivalent to package[path] = v, 
     * And v is the value at the top of the stack,
     * the v value will be popped from the stack*/
    lua_setfield(L, top, "path");

    lua_pushliteral(L, "./?.so;");
    if(home)
    {
        lua_pushstring(L, home);
        lua_pushliteral(L, "/.luarocks/lib/lua/5.1/?.so;");
        lua_pushstring(L, home);
        lua_pushliteral(L, "/.luarocks/lib/lua/?.so;");
    }

    lua_pushliteral(L, "/usr/local/lib/lua/5.1/?.so;");
    lua_pushliteral(L, "/usr/lib/lua/5.1/?.so;");

    lua_concat(L, lua_gettop(L) - top);
    const char *cpath = getenv("LUA_CPATH");
    if(cpath)
    {
        const char *def = lua_tostring(L, -1);
        cpath = luaL_gsub(L, cpath, ";;", ";\1;");
        luaL_gsub(L, cpath, "\1", def);
        lua_remove(L, -2);
        lua_remove(L, -2);
    }

    lua_setfield(L, top, "cpath");
    lua_pop(L, 1); /* package.path, package.cpath */

}


static void luaz_load_number(lua_State *L, const char *name, lua_Number n)
{
    lua_pushstring(L, name);
    lua_pushnumber(L, n);
    lua_settable(L, -3);
}


static void luaz_load_string(lua_State *L, const char *name, const char *s)
{
    lua_pushstring(L, name);
    lua_pushstring(L, s);
    lua_settable(L, -3);
}


int luaz_close_state(lua_State *state)
{
    if(state)
        lua_close(state);

    return 0;
}


lua_State *luaz_new_state(void)
{
    lua_State *L ;
    
    L = luaL_newstate();
    if(!L)
        return NULL;

    luaL_openlibs(L);
    luaz_set_paths(L);

    /* Create an empty table and push it onto the stack. 
     * It is equivalent to lua_createtable(L, 0, 0) */
    //lua_newtable(L);
    
    int rc;

    if((rc = luaL_loadfile(L, "config")) != 0)
    {
        lua_getglobal(L, "require");
        lua_pushstring(L, "config");
        
        if(lua_pcall(L, 1, 1, 0))
        {
            const char * err = lua_tostring(L, -1);
            if(err)
            {
                fprintf(stdout, "error: %s\n", err);
                PRT_TAIL_CHR;
                exit(-1);
            }
    
            exit(-1);
        }

    }else if(lua_pcall(L, 0, 0, 0))
    {
        const char *err = lua_tostring(L, -1);
        if(err)
        {
            fprintf(stdout, "error: %s\n", err);
            PRT_TAIL_CHR;
            exit(-1);
        }

        exit(-1);
    }

    int top = lua_gettop(L); 
    return L; 
}






