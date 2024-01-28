#!/usr/bin/env python3

import importlib
import os
import setuptools


def discover_files_by_extension(dirname: str, ext: str) -> list:
	ret = list()
	for i in os.scandir(dirname):
		if os.path.splitext(i.path)[1] == ext:
			ret.append(i.path)
	return ret


def get_numpy_include_dir() -> str:
	numpy = importlib.import_module("numpy")
	return numpy.get_include()


# c++ extension module
ext__iebpr = setuptools.Extension(
	"iebpr._iebpr",
	sources=discover_files_by_extension("src", ".cpp"),
	include_dirs=[
		get_numpy_include_dir(),
	],
	define_macros=[],
	library_dirs=[],
	libraries=[],
	extra_compile_args=["-std=c++11", "-g0", "-O3", "-Wall",
		"-Wno-missing-braces"],
	extra_link_args=[],
	# py_limited_api=True,
)

setuptools.setup(
    name="iebpr",
    packages=["iebpr", "iebpr.agent_template"],
	package_dir={
		"iebpr": "iebpr",
	},
	include_package_data=True,
    ext_modules=[ext__iebpr],
)
