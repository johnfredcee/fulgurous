from SCons.Script import *

import os
import os.path
import glob
import platform


def exists(env):
	return True

def generate(env, **kwargs):
	vars = env["BUILD_OPTIONS"]
	env.Append(LIBS =  [ "kernel32.lib", "user32.lib", "gdi32.lib", "winspool.lib", "shell32.lib", "ole32.lib", "oleaut32.lib", "uuid.lib", "comdlg32.lib", "advapi32.lib" ])
	env.Append(LINKFLAGS = [ "/SUBSYSTEM:CONSOLE" ])
	env.Append(CFLAGS = ["/DWIN32", "/D_WINDOWS", "/W3"])
	env.Append(CCFLAGS = ["/DWIN32", "/D_WINDOWS", "/W3"])
	env.Append(CFLAGS = ["/D_DEBUG", "/MDd", "/Zi", "/Ob0", "/Od", "/RTC1"])     # debug flags
	env.Append(CCFLAGS = ["/D_DEBUG", "/MDd", "/Zi", "/Ob0", "/Od", "/RTC1"])    # debug flags
	env.Append(LINKFLAGS = [ "/MACHINE:X64"])
	env.Append(LINKFLAGS = [ "/DEBUG", "/INCREMENTAL"])     # debug flags
	env.Append(CXXFLAGS =["/EHsc", "/std:c++17"])
	return env
	
