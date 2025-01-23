from glob import glob
from setuptools import setup
import os
import subprocess
import platform
import distutils.cmd

from pybind11.setup_helpers import Pybind11Extension, build_ext as build_ext_orig


__package__ = "pyimax"
__version__ = "0.0.1"

cppfiles = []
cppfiles.extend(sorted(glob("src/*.cpp")))

cfiles = []
cfiles.extend(sorted(glob("src/*.c")))

imax_files = []
imax_files.extend(sorted(glob("src/imax/*.c")))

ext_modules = [
    Pybind11Extension(
        __package__,
        cppfiles,
        define_macros=[("VERSION_INFO", __version__)],
    ),
]

class CustomBuildExt(build_ext_orig):
    def build_extensions(self):
        # 現在のコンパイルコマンド（リスト）を取得
        orig_compiler_so = self.compiler.compiler_so[:]  # リストのコピー

        # 環境変数 CC で C コンパイラが指定されていればそれを、なければ "gcc" を使用
        gcc_cmd = os.environ.get("CC", "gcc")
        self.compiler.compiler_so[0] = gcc_cmd

        # C のソースファイルをコンパイル（このとき、self.compiler で gcc が使われます）
        imax_args = ["-DEMAX7", "-DFPDDMA"]
        if platform.system() == "Linux" and (platform.machine() == "aarch64" or platform.machine() == "arm64"):
            imax_args.append("-DARMZYNQ")
        c_objects = self.compiler.compile(cfiles, extra_postargs=["-fcommon"] + imax_args)

        # 元のコンパイルコマンドに戻す
        self.compiler.compiler_so = orig_compiler_so

        cwd = os.getcwd()
        conv_mark_cmd = cwd + "/include/conv-mark/conv-mark" 
        conv_c2d_cmd =  cwd + "/include/conv-c2d/conv-c2d-zynq"

        new_imax_files = []
        imax_build_dir = cwd + "/build/imax/"
        if not os.path.exists(imax_build_dir):
            os.makedirs(imax_build_dir, exist_ok=True)
        for imax_file in imax_files:
            if "-DARMZYNQ" in imax_args:
                imax_file_name = imax_build_dir + imax_file.split('/')[-1]
                imax_output_file_name = imax_build_dir + imax_file.split('/')[-1].split('.')[0] + '-emax7.c'
                with open(imax_file_name + '-mark.c', 'w') as fp:
                    subprocess.run([conv_mark_cmd, imax_file], stdout=fp)
                with open(imax_file_name + '-cppo.c', 'w') as fp:
                    subprocess.run(['cpp', '-P', '-DEMAX7', '-DFPDDMA', '-DNO_EMAX7LIB_BODY', "-DARMZYNQ", imax_file_name + '-mark.c'], stdout=fp)
                subprocess.run([conv_c2d_cmd, imax_file_name + '-cppo.c'], cwd=imax_build_dir)
                with open(imax_output_file_name, 'r') as fp:
                    print(fp.read())
                new_imax_files.append(imax_output_file_name)
            else:
                new_imax_files.append(imax_file)

        imax_objects = self.compiler.compile(new_imax_files, extra_postargs=["-lm", "-lpthread", "-fcommon"] + imax_args)

        for ext in self.extensions:
            if ext.name == __package__:
                if ext.extra_objects is None:
                    ext.extra_objects = []
                ext.extra_objects.extend(c_objects)
                ext.extra_objects.extend(imax_objects)
        super().build_extensions()

class CleanCommand(distutils.cmd.Command):
    user_options = []

    def initialize_options(self):
        pass

    def finalize_options(self):
        pass

    def run(self):
        subprocess.run(["rm", "-rf", "build"])
        subprocess.run(["rm", "-rf", "dist"])
        subprocess.run(["rm", "-rf", "pyimax.egg-info"])
        subprocess.run(["find", ".", "-name", "*.o", "-exec", "rm", "{}", ";"])
        subprocess.run(["find", ".", "-name", "*.so", "-exec", "rm", "{}", ";"])
        subprocess.run(["find", ".", "-name", "*.pyc", "-exec", "rm", "{}", ";"])
        subprocess.run(["find", ".", "-name", "__pycache__", "-exec", "rm", "-rf", "{}", ";"])


setup(
    name=__package__,
    version=__version__,
    description="IMAX Python Wrapper",
    long_description="IMAX Python Wrapper",
    author="Dohyun Kim",
    ext_modules=ext_modules,
    cmdclass={"build_ext": CustomBuildExt, "clean": CleanCommand},
    install_requires=["pybind11>=2.10"],
    python_requires=">=3.9",
)
