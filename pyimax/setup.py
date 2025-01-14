from glob import glob
from setuptools import setup

from pybind11.setup_helpers import Pybind11Extension, build_ext


__package__ = "pyimax"
__version__ = "0.0.1"

cppfiles = []
cppfiles.extend(sorted(glob("src/*.cpp")))

ext_modules = [
    Pybind11Extension(
        __package__,
        cppfiles,
        define_macros=[("VERSION_INFO", __version__)],
    ),
]

setup(
    name=__package__,
    version=__version__,
    description="IMAX Python Wrapper",
    long_description="IMAX Python Wrapper",
    author="Dohyun Kim",
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
    install_requires=["pybind11>=2.10"],
    python_requires=">=3.9",
)
