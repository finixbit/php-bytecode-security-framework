from distutils.core import setup, Extension
import os


def get_php_include_dirs():
    dirs = []
    try:
        dirs = os.popen("php-config --includes").readline().strip().split(" ")
    except Exception as e:
        print("Error getting php include directories")
        raise e
    return [d[2:] for d in dirs]


phpmodule = Extension(
    "php_to_bytecode_converter",
    define_macros=[("MAJOR_VERSION", "1"), ("MINOR_VERSION", "0")],
    include_dirs=get_php_include_dirs()
    + ["/app/src/php_to_bytecode_converter/src/includes"],
    libraries=["php7"],
    library_dirs=["/usr/local/lib"],
    sources=[
        "src/main.c",
        "src/libs/php_embed_ex.c",
        "src/libs/php_cfg.c",
        "src/libs/php_dfg.c",
        "src/libs/php_ssa.c",
    ],
    extra_compile_args=["-Wmaybe-uninitialized", "-Wreturn-type", "-ggdb"],
)


def main():
    setup(
        name="php_to_bytecode_converter",
        version="0.0.1",
        description="A Python module is written in `C` to convert PHP source files to Zend bytecode (op_arrays), CFG, DFG, and SSA Python objects.",  # noqa: E501
        author="finixbit",
        author_email="samuelasirifi1@gmail.com",
        ext_modules=[phpmodule],
    )


if __name__ == "__main__":
    main()
