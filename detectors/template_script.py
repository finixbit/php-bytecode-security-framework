from php_bytecode_api import convert_and_lift
from php_bytecode_api import helper_functions
from php_bytecode_api import models
import logging

logging.getLogger().setLevel(logging.INFO)

rootdir = "/app/targets"


def do_something(function: models.FunctionModel):
    pass


def main():
    for file_path in helper_functions.recursively_get_directory_php_files(rootdir):
        functions = convert_and_lift(file_path)
        if not functions:
            logging.error(f"Loading ... {file_path} -- No Functions Found")
        else:
            for function in functions:
                logging.info(
                    f"Analyzing function ... {file_path}:{function.function_name}"
                )
                do_something(function)


if __name__ == "__main__":
    main()
