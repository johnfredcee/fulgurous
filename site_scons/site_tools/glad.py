from SCons.Script import *

import os
import os.path
import glob
import platform


def exists(env):
    return True

def generate(env, **kwargs):
    vars = env["BUILD_OPTIONS"]
    vars.Add(PathVariable("GLAD_INCLUDE_PATH",
                          "Where the glad generated headers live",
                          "/usr/include"))
    gladenv = Environment(variables = vars, tools = [])
    env.Append(GLAD_INCLUDE_PATH = gladenv["GLAD_INCLUDE_PATH"])
    result = FindFile("glad.h", env["GLAD_INCLUDE_PATH"]  + "/glad")
    if (result):
        if (not("CPPPATH" in env) or (not(env["GLAD_INCLUDE_PATH"] in env["CPPPATH"]))):
            env.Append(CPPPATH =  env["GLAD_INCLUDE_PATH"])
        else:
            pass
    else:
        print("Failed to find glad at " + ( env["GLAD_INCLUDE_PATH"] ))
        Exit(1)
    return env
    
