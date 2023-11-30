#!/usr/bin/env bash

# 
# install php_to_bytecode_converter
# 
cd /app/src/php_to_bytecode_converter
python3 setup.py install

# 
# install php_bytecode_api
# 
cd /app/src/php_bytecode_api
python3 setup.py install

cd /app

echo "done ..."