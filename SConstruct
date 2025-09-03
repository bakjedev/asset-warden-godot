#!/usr/bin/env python
import os
import sys

env = SConscript("godot-cpp/SConstruct")

# Create variant build directory
env.VariantDir('build/src', 'src', duplicate=0)

env.Append(CPPPATH=["src/"])
sources = Glob("build/src/*.cpp")  # Use the variant directory

library = env.SharedLibrary(
    "demo/bin/libbakjext{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
    source=sources,
)

Default(library)