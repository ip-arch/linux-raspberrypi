from setuptools import setup, Extension

setup(
    name="pygpio",
    version="0.1",
    ext_modules=[
        Extension("pygpio", sources=["pygpio.c"]),
    ],
)
