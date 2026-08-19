import subprocess
import sys

from setuptools import Distribution, find_packages, setup
from setuptools.command.build_py import build_py as _build_py


class CustomBuildPy(_build_py):
    def run(self):
        subprocess.check_call([sys.executable, "local_build.py"])
        super().run()


class BinaryDistribution(Distribution):
    def has_ext_modules(self):
        return True


setup(
    packages=find_packages(include=["docling_nlp", "docling_nlp.*"]),
    distclass=BinaryDistribution,
    cmdclass={"build_py": CustomBuildPy},
    zip_safe=False,
    include_package_data=True,
    package_data={
        "docling_nlp": [
            "*.so",
            "*.pyd",
            "*.dll",
            "resources/**/*",
        ],
    },
)
