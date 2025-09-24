#!/usr/bin/env python
import os
import sys
import shutil

env = SConscript("godot-cpp/SConstruct")

env.Append(CPPPATH=["src/"])

env.VariantDir('build/src', 'src', duplicate=0)

sources = Glob("build/src/*.cpp")

folder = "demo/addons/bakje-extension"

for filename in os.listdir(folder):
	file_path = os.path.join(folder, filename)
	if os.path.isfile(file_path):
		os.remove(file_path)

env.Append(LINKFLAGS=["/ignore:4099"])

library = env.SharedLibrary(
		"{}/libbakjext{}{}"
			.format(folder, env["suffix"], env["SHLIBSUFFIX"]),
		source=sources,
)

Default(library)