from glob import glob
from setuptools import setup
import os
import subprocess

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
        # ここでは self.compiler は Pybind11Extension 用（通常 g++）のコンパイラオブジェクトです。
        # そのため、Cソースを gcc でコンパイルするため、一時的に self.compiler.compiler_so の実行コマンドを上書きします。

        # 現在のコンパイルコマンド（リスト）を取得
        orig_compiler_so = self.compiler.compiler_so[:]  # リストのコピー

        # 環境変数 CC で C コンパイラが指定されていればそれを、なければ "gcc" を使用
        gcc_cmd = os.environ.get("CC", "gcc")
        # 元の compiler_so は、通常 g++ が使われているはずなので、
        # 先頭要素のみを gcc に変更し、他はそのままとする（例：['g++', '-pthread', ...] → ['gcc', ...]）
        self.compiler.compiler_so[0] = gcc_cmd

        # C のソースファイルをコンパイル（このとき、self.compiler で gcc が使われます）
        c_objects = self.compiler.compile(cfiles, extra_postargs=["-fcommon"])

        # 元のコンパイルコマンドに戻す
        self.compiler.compiler_so = orig_compiler_so

        conv_mark_cmd = "./include/conv-mark/conv-mark" 
        conv_c2d_cmd =  "./include/conv-c2d/conv-c2d-cent"

        new_imax_files = []
        for imax_file in imax_files:
            with open(imax_file + '-mark.c', 'w') as fp:
                subprocess.run([conv_mark_cmd, imax_file], stdout=fp)
            with open(imax_file + '-cppo.c', 'w') as fp:
                subprocess.run(['cpp', '-P', '-DEMAX7', '-DFPDDMA', '-DNO_EMAX7LIB_BODY', imax_file + '-mark.c'], stdout=fp)
            subprocess.run([conv_c2d_cmd, imax_file + '-cppo.c'])
            with open(imax_file.split('.')[0] + '-emax7.c', 'r') as fp:
                print(fp.read())
            new_imax_files.append(imax_file.split('.')[0] + '-emax7.c')

        imax_objects = self.compiler.compile(new_imax_files, extra_postargs=["-lm", "-lpthread", "-fcommon"])

        for ext in self.extensions:
            if ext.name == __package__:
                if ext.extra_objects is None:
                    ext.extra_objects = []
                ext.extra_objects.extend(c_objects)
                ext.extra_objects.extend(imax_objects)
        super().build_extensions()


setup(
    name=__package__,
    version=__version__,
    description="IMAX Python Wrapper",
    long_description="IMAX Python Wrapper",
    author="Dohyun Kim",
    ext_modules=ext_modules,
    cmdclass={"build_ext": CustomBuildExt},
    install_requires=["pybind11>=2.10"],
    python_requires=">=3.9",
)
