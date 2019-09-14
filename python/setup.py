# -*- coding: utf-8 -*-

import os
from distutils.core import setup, Extension
from Cython.Build import cythonize

setup(
    ext_modules=cythonize(Extension('cybagoa',
                                    sources=['src/cybagoa.pyx', '../src/bag.cpp',
                                             '../src/bagoa.cpp'],
                                    language='c++',
                                    include_dirs=[os.environ['OA_INCLUDE_DIR'],
                                                  '../include/'],
                                    libraries=['oaCommon', 'oaBase', 'oaPlugIn',
                                               'oaDM', 'oaTech', 'oaDesign', 'dl'],
                                    library_dirs=[os.environ['OA_LINK_DIR']],
                                    extra_compile_args=["-std=c++11"],
                                    extra_link_args=["-std=c++11"],
                                    )
                          ),
)
