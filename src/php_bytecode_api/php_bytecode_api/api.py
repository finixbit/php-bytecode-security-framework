from php_bytecode_api.models import FunctionModel
import php_to_bytecode_converter


def convert_and_lift(file_path: str) -> [FunctionModel]:
    functions = php_to_bytecode_converter.convert(file_path)
    if not functions:
        print(f"Unable to convert file path {file_path} to PHP Bytecodes")
        return
    try:
        source_lines = open(file_path, "rb").readlines()
        for fn in functions:
            fn.update({"source_lines": source_lines})
            fn_obj = FunctionModel(**fn)
            fn_obj.update_all()
            yield fn_obj
    except Exception as e:
        print(str(e))
