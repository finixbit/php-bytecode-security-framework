import setuptools
from setuptools import find_packages, setup
import os

_THIS_DIR = os.path.dirname(os.path.realpath(__file__))

PYTHON_REQUIRES = ">=3.8"


if int(setuptools.__version__.split(".", 1)[0]) < 18:
    raise AssertionError("setuptools >= 18 must be installed")


def main():
    with open(os.path.join(_THIS_DIR, "requirements.txt")) as f:
        requirements = f.readlines()

    setup(
        name="php_bytecode_api",
        version="0.0.1",
        author="finixbit",
        author_email="samuelasirifi1@gmail.com",
        description="A high-level Python API for `src/php_to_bytecode_converter`, intended for the conversion of PHP files into a defined PHP Bytecode Pydantic model. This includes additional functions to facilitate the analysis of PHP bytecode. ",  # noqa: E501
        packages=find_packages(),
        include_package_data=True,
        install_requires=requirements,
        python_requires=PYTHON_REQUIRES,
    )


if __name__ == "__main__":
    main()
