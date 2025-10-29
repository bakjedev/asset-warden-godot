#!/usr/bin/env python
import os
import sys
import shutil

env = SConscript("godot-cpp/SConstruct")

env.Append(CPPPATH=["src/"])

env.VariantDir('build/src', 'src', duplicate=0)

sources = Glob("build/src/*.cpp") + \
          Glob("build/src/core/*.cpp") + \
          Glob("build/src/editor/*.cpp") + \
          Glob("build/src/debugger/*.cpp")


if env["target"] in ["editor", "template_debug"]:
    try:
        doc_data = env.GodotCPPDocData("build/src/gen/doc_data.gen.cpp", source=Glob("doc_classes/*.xml"))
        sources.append(doc_data)
    except AttributeError:
        print("Not including class reference as we're targeting a pre-4.3 baseline.")

folder = "demo/addons/bakje-extension"

if not os.path.isdir(folder):
	os.makedirs(folder, exist_ok=True)

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