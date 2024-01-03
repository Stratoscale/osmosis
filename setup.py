import os
from setuptools import setup


def read(fname):
    return open(os.path.join(os.path.dirname(__file__), fname)).read()

setup(
    name="osmosis",
    version="1.1.0-beta1",
    author="Shlomo Matichin",
    author_email="shlomomatichin@gmail.com",
    description=(
        "Osmosis support tools in python"),
    keywords="Osmosis rootfs initrd boot",
    url="http://packages.python.org/osmosis",
    packages=["osmosis", "osmosis.policy"],
    package_dir={
        "osmosis": "py/osmosis",
        "osmosis.policy": "py/osmosis/policy"
    },
    long_description=read("README.md"),
    classifiers=[
        "Development Status :: 4 - Beta",
        "Topic :: Utilities",
    ],
)
