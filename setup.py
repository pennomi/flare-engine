from distutils.core import setup
import os
from Cython.Build import cythonize
from Cython.Distutils import Extension


SOURCE_DIR = os.path.join(os.getcwd(), 'src')
INCLUDE_DIRS = [s[0] for s in os.walk(SOURCE_DIR)]
ALL_FILES = []
[ALL_FILES.extend(os.path.join(d[0], f) for f in d[2]) for d in os.walk(SOURCE_DIR)]
SOURCE_FILES = [f for f in ALL_FILES if f.endswith('.cpp') or f.endswith('.c')]

setup(
    name="flare",
    ext_modules=cythonize([
        Extension(
            "flare", ["bindings/flare.pyx"] + SOURCE_FILES,
            include_dirs=['/usr/include/SDL2', 'src'],
            libraries=["SDL2", "SDL2_ttf", "SDL2_mixer", "SDL2_image", ],
            language="c++",
        )
    ])
)
