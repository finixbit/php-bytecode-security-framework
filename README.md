# php-bytecode-security-framework

A high-level Python API for converting PHP files into a PHP Bytecode Pydantic model. It incorporates additional functions to assist in the analysis of PHP bytecode. The implementation involves a Python module written in `C` to convert PHP source files to Zend bytecode (op_arrays) Python objects.  

## Why `php-bytecode-security-framework`

Please visit <https://finixbit.github.io/site/posts/autonomous-Hacking-of-PHP-Web-Applications-at-the-Bytecode-Level>

## Requirements

[Docker](https://www.docker.com/) - Build, test, and deploy applications quickly. 

## Setting up
```sh
git clone https://github.com/finixbit/php-bytecode-security-framework
cd php-bytecode-security-framework

docker run --rm -it --entrypoint /bin/bash -v ${PWD}/:/app debian:buster-20220801
cd /app

./setup_scripts/install_deps.sh
./setup_scripts/install_framework.sh
```

## Example Test cases
Before running any of the test cases below, you need to set the docker container by following the [setup above](#setting-up)

### Testing with Vulnerable Web Applications
All tests were done with `detection_script/simple_code_injection.py` which only models generic PHP global variables (SOURCES) like `$_GET, $_POST, $_REQUEST` and traces data down to potentially vulnerable calls (SINKS) like `echo, concat, include, etc`.

| Name        | Vulns  | Report  | Github
| ----------- |:-------------:| -----:| -----:|
| Damn Vulnerable Web Application (DVWA) | `21` | [Report Link](https://github.com/finixbit/php-bytecode-security-framework/blob/master/vulnerabilities/vulnerable_web_apps.md#testing-with-dvwa) | [Github Repo](https://github.com/digininja/DVWA) |
| OWASP Vulnerable Web Application Project | `10` | [Report Link](https://github.com/finixbit/php-bytecode-security-framework/blob/master/vulnerabilities/vulnerable_web_apps.md#testing-with-owasp-vuln-web-app) | [Github Repo](https://github.com/OWASP/Vulnerable-Web-Application) |
| Simple SQL Injection Training App | `15` | [Report Link](https://github.com/finixbit/php-bytecode-security-framework/blob/master/vulnerabilities/vulnerable_web_apps.md#testing-with-sqlinjection-training-app) | [Github Repo](https://github.com/appsecco/sqlinjection-training-app) |
| Vulnerable Web application made with PHP/SQL | `6` | [Report Link](https://github.com/finixbit/php-bytecode-security-framework/blob/master/vulnerabilities/vulnerable_web_apps.md#testing-with-oste-vulnerable-web-application) | [Github Repo](https://github.com/OSTEsayed/OSTE-Vulnerable-Web-Application) |
| InsecureTrust_Bank - Educational repo demonstrating web app vulnerabilities | `5` | [Report Link](https://github.com/finixbit/php-bytecode-security-framework/blob/master/vulnerabilities/vulnerable_web_apps.md#testing-with-insecuretrust_bank) | [Github Repo](https://github.com/Hritikpatel/InsecureTrust_Bank) |


## Projects

### php_bytecode_api (`src/php_bytecode_api`)
A high-level Python API for `src/php_to_bytecode_converter`, intended for the conversion of PHP files into a defined PHP Bytecode Pydantic model. This includes additional functions to facilitate the analysis of PHP bytecode. 

```python
import php_bytecode_api
functions: list[FunctionModel] = php_bytecode_api.convert_and_lift(path_to_php_file)
``` 

See [Pydantic Models](https://github.com/finixbit/php-bytecode-security-framework/blob/master/src/php_bytecode_api/models.py) below for more details on the data structure `FunctionModel` extracted from the PHP Bytecode.  

### php_to_bytecode_converter (`src/php_to_bytecode_converter`)
A Python module is written in `C` to convert PHP source files to Zend bytecode (op_arrays), CFG, DFG, and SSA Python objects. 

```python
import php_to_bytecode_converter
functions: list[dict] = php_to_bytecode_converter.convert(path_to_php_file)
``` 

Below is the data structure extracted from each function found in a PHP file:
```yaml
- filename: str
- class_name: str
- function_name: str
- num_args: int
- required_num_args: int
- number_of_instructions: int
- extra = {
    type: int
    number_of_cv_variables: int 
    number_of_tmp_variables: int
    last_live_range: int
    last_try_catch: int 
    arg_flags: list [int, int, int]
    last_literal: int
    literals: [
    	{
    		type: str
    		value: str
    	}
    ]
  }
- instructions = [
	{
		op1: {
			type:  str
			value: str
			variable_number: int
		}
		op2: {
			type:  str
			value: str
			variable_number: int
		}
		result: {
			type:  str
			value: str
			variable_number: int
		}
		num: int
		extended_value: int
		lineno: int
		opcode: int
		opcode_name: str
		opcode_flags: int
		op1_type: int
		op2_type: int
		result_type: int
	}
]
- cfg = {
	blocks_count: int
	edges_count: int
	blocks: [
		{
			start: int
			len: int
			successors_count: int
			predecessors_count: int
			predecessor_offset: int
			idom: int
			loop_header: int
			level: int
			children: int
			next_child: int
			successors_storage: [int, int]
			successors: [int, int]
		}
	]
	predecessors: [int, int]
}
- dfg = [
	{
		block_index: int
		var_def: [
			{
				var_num: int
				var_name: str
			}
		]
		var_use: [
			{
				var_num: int
				var_name: str
			}
		]
		var_in: [
			{
				var_num: int
				var_name: str
			}
		]
		var_out: [
			{
				var_num: int
				var_name: str
			}
		]
		var_tmp: [
			{
				var_num: int
				var_name: str
			}
		]
	}
]
- ssa = {
	number_of_sccs: int
	number_of_ssa_variables: int
	ssa_variables: [
		{
			ssa_var_num: int
			var_num: int
			var_name: str
			definition: int
			definition_phi: {
				pi: int
				variable_index: int
				variable: {
					var_num: int
					var_name: str
				}
				ssa_variable_index: int
				current_block_index: int
				visited: int
				has_range_constraint: int
				constraint: {
					range_range_min: int
					range_range_max: int
					range_range_underflow: int
					range_range_overflow: int
					range_min_var: int
					range_max_var: int
					range_min_ssa_var: int
					range_max_ssa_var: int
				}
				sources: [...int]
			}
			no_val: int
			use_chain: int
			escape_state: int
			strongly_connected_component: int
			strongly_connected_component_entry: int

		}
	]
	ssa_instructions: [
		{
			op1_use: int
			op2_use: int
			result_use: int
			op1_def: int
			op2_def: int
			result_def: int
			op1_use_chain: int
			op2_use_chain: int
			res_use_chain: int
		}
	]
	ssa_blocks: [
		{
			block_index; int
			phis; [
				{
					pi: int
					variable_index: int
					variable: {
						var_num: int
						var_name: str
					}
					ssa_variable_index: int
					current_block_index: int
					visited: int
					has_range_constraint: int
					constraint: {
						range_range_min: int
						range_range_max: int
						range_range_underflow: int
						range_range_overflow: int
						range_min_var: int
						range_max_var: int
						range_min_ssa_var: int
						range_max_ssa_var: int
					}
					sources: [...int]
				}
			]
		}
	]

}
```

### detectors
Directory to hold all scripts
- `detectors/template_script.py` - template script to get started.  
- `detectors/code_injection.py` - Models generic PHP global variable (SOURCES) like `$_GET, $_POST, $_REQUEST` and traces data down to potentially vulnerable calls (SINKS) like `echo, concat, include, etc`.  

### targets
Directory to store target source files. Follow the [Example Test Cases](#example-test-cases) above to set up a target.

