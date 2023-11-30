#include <stdio.h>
#include <Python.h>
#include <ctype.h>
#include <stdbool.h>
#include "zend.h"
#include "php_embed_ex.h"
#include "php_cfg.h"
#include "php_dfg.h"
#include "php_ssa.h"

void zend_accel_move_user_functions (HashTable *src, HashTable *dst);

static bool generate_function_object (const char *class_name,
                                      const char *function_name,
                                      const zend_op_array *op_array,
                                      zend_file_handle *file_handle,
                                      PyObject *fn_pobject);

static bool generate_basic_info (const zend_op_array *op_array,
                                 PyObject *fn_pobject);

static bool generate_extra_info (const zend_op_array *op_array,
                                 PyObject *fn_pobject);

static bool generate_instructions_info (const zend_op_array *op_array,
                                        PyObject *fn_pobject);

static bool generate_cfg_info (const zend_op_array *op_array,
                               zend_file_handle *file_handle,
                               PyObject *fn_pobject);

static bool generate_dfg_info (const zend_op_array *op_array, zend_cfg *cfg,
                               PyObject *fn_pobject);

static bool generate_ssa_info (const zend_op_array *op_array, zend_cfg *cfg,
                               zend_file_handle *file_handle,
                               PyObject *fn_pobject);

static PyObject *zval_to_python_object (zval *val);

static PyObject *
method_convert (PyObject *self, PyObject *args)
{
    char *filename = NULL;
    PyObject *zend_functions_pyobject = PyList_New (0);

    /* Parse arguments */
    if (!PyArg_ParseTuple (args, "s", &filename))
        {
            return zend_functions_pyobject;
        }

    zend_script script;
    zend_op_array *op_array = NULL;
    int php_embed_argc = 0;
    char **php_embed_argv = NULL;

    PHP_EMBED_START_BLOCK (php_embed_argc, php_embed_argv);

    zend_file_handle file_handle;
    file_handle.filename = filename;
    file_handle.free_filename = 0;
    file_handle.type = ZEND_HANDLE_FILENAME;
    file_handle.opened_path = NULL;

    // initialize function_table & class_table
    zend_hash_init (&script.function_table, 128, NULL, ZEND_FUNCTION_DTOR, 0);
    zend_hash_init (&script.class_table, 16, NULL, ZEND_CLASS_DTOR, 0);

    op_array = zend_compile_file (&file_handle, ZEND_INCLUDE TSRMLS_CC);
    if (!op_array)
        {
            printf ("Error parsing php script file: %s\n", filename);
            goto cleanup_e;
        }

    PyObject *fn_pobject = PyDict_New ();
    bool status = generate_function_object ("", "main", op_array, &file_handle,
                                            fn_pobject);
    if (status == true)
        {
            PyList_Append (zend_functions_pyobject, fn_pobject);
        }

    zend_accel_move_user_functions (CG (function_table),
                                    &script.function_table);
    zend_op_array *iter_op_array;
    ZEND_HASH_FOREACH_PTR (&script.function_table, iter_op_array)
    {
        PyObject *fn_pobject = PyDict_New ();
        bool status = generate_function_object (
            "", ZSTR_VAL (iter_op_array->function_name), iter_op_array,
            &file_handle, fn_pobject);
        if (status == true)
            {
                PyList_Append (zend_functions_pyobject, fn_pobject);
            }
    }
    ZEND_HASH_FOREACH_END ();

    EG (class_table) = CG (class_table);
    zend_class_entry *ce;
    zend_string *name;
    ZEND_HASH_FOREACH_PTR (EG (class_table), ce)
    {
        if (ce->type == ZEND_USER_CLASS)
            {
                ZEND_HASH_FOREACH_STR_KEY_PTR (&ce->function_table, name,
                                               iter_op_array)
                {
                    if (iter_op_array->scope == ce)
                        {
                            PyObject *fn_pobject = PyDict_New ();
                            bool status = generate_function_object (
                                ZSTR_VAL (ce->name),
                                ZSTR_VAL (iter_op_array->function_name),
                                iter_op_array, &file_handle, fn_pobject);
                            if (status == true)
                                {
                                    PyList_Append (zend_functions_pyobject,
                                                   fn_pobject);
                                }
                        }
                }
                ZEND_HASH_FOREACH_END ();
            }
    }
    ZEND_HASH_FOREACH_END ();

cleanup_e:
    zend_hash_destroy (&script.function_table);
    zend_hash_destroy (&script.class_table);

    if (op_array)
        {
            destroy_op_array (op_array TSRMLS_CC);
            efree (op_array);
        }

    PHP_EMBED_END_BLOCK ();
    return zend_functions_pyobject;
}

void
zend_accel_move_user_functions (HashTable *src, HashTable *dst)
{
    Bucket *p;
    dtor_func_t orig_dtor = src->pDestructor;

    src->pDestructor = NULL;
    zend_hash_extend (dst, dst->nNumUsed + src->nNumUsed, 0);
    ZEND_HASH_REVERSE_FOREACH_BUCKET (src, p)
    {
        zend_function *function = Z_PTR (p->val);

        if (EXPECTED (function->type == ZEND_USER_FUNCTION))
            {
                _zend_hash_append_ptr (dst, p->key, function);
                zend_hash_del_bucket (src, p);
            }
        else
            {
                break;
            }
    }
    ZEND_HASH_FOREACH_END ();
    src->pDestructor = orig_dtor;
}

static bool
generate_function_object (const char *class_name, const char *function_name,
                          const zend_op_array *op_array,
                          zend_file_handle *file_handle, PyObject *fn_pobject)
{
    bool status = true;

    PyDict_SetItem (fn_pobject, Py_BuildValue ("s", "class_name"),
                    Py_BuildValue ("s", class_name));

    PyDict_SetItem (fn_pobject, Py_BuildValue ("s", "function_name"),
                    Py_BuildValue ("s", function_name));

    status = generate_basic_info (op_array, fn_pobject);
    if (status == false)
        {
            return false;
        }

    status = generate_extra_info (op_array, fn_pobject);
    if (status == false)
        {
            return false;
        }

    status = generate_instructions_info (op_array, fn_pobject);
    if (status == false)
        {
            return false;
        }

    // printf("\n");
    status = generate_cfg_info (op_array, file_handle, fn_pobject);
    if (status == false)
        {
            return false;
        }
    return status;
}

static bool
generate_basic_info (const zend_op_array *op_array, PyObject *fn_pobject)
{
    // op_array->num_args
    PyDict_SetItem (fn_pobject, Py_BuildValue ("s", "num_args"),
                    Py_BuildValue ("i", op_array->num_args));

    // op_array->required_num_args
    PyDict_SetItem (fn_pobject, Py_BuildValue ("s", "required_num_args"),
                    Py_BuildValue ("i", op_array->required_num_args));

    // op_array->last
    PyDict_SetItem (fn_pobject, Py_BuildValue ("s", "number_of_instructions"),
                    Py_BuildValue ("i", op_array->last));

    // op_array->filename
    if (op_array->filename)
        PyDict_SetItem (fn_pobject, Py_BuildValue ("s", "filename"),
                        Py_BuildValue ("s", ZSTR_VAL (op_array->filename)));
    else
        PyDict_SetItem (fn_pobject, Py_BuildValue ("s", "filename"),
                        Py_BuildValue ("s", ""));
    return true;
}

static bool
generate_extra_info (const zend_op_array *op_array, PyObject *fn_pobject)
{
    PyObject *fn_extra_pobject = PyDict_New ();

    // op_array->type
    PyDict_SetItem (fn_extra_pobject, Py_BuildValue ("s", "type"),
                    Py_BuildValue ("i", op_array->type));

    // op_array->cache_size
    PyDict_SetItem (fn_extra_pobject, Py_BuildValue ("s", "cache_size"),
                    Py_BuildValue ("i", op_array->cache_size));

    // op_array->last_var
    PyDict_SetItem (fn_extra_pobject,
                    Py_BuildValue ("s", "number_of_cv_variables"),
                    Py_BuildValue ("i", op_array->last_var));

    // op_array->T
    PyDict_SetItem (fn_extra_pobject,
                    Py_BuildValue ("s", "number_of_tmp_variables"),
                    Py_BuildValue ("i", op_array->T));

    // op_array->last_live_range
    PyDict_SetItem (fn_extra_pobject, Py_BuildValue ("s", "last_live_range"),
                    Py_BuildValue ("i", op_array->last_live_range));

    // op_array->last_try_catch
    PyDict_SetItem (fn_extra_pobject, Py_BuildValue ("s", "last_try_catch"),
                    Py_BuildValue ("i", op_array->last_try_catch));

    // op_array->arg_flags
    PyObject *arg_flags = PyList_New (0);
    PyList_Append (arg_flags, Py_BuildValue ("i", op_array->arg_flags[0]));

    PyList_Append (arg_flags, Py_BuildValue ("i", op_array->arg_flags[1]));

    PyList_Append (arg_flags, Py_BuildValue ("i", op_array->arg_flags[2]));

    PyDict_SetItem (fn_extra_pobject, Py_BuildValue ("s", "arg_flags"),
                    arg_flags);

    // op_array->last_literal
    PyDict_SetItem (fn_extra_pobject, Py_BuildValue ("s", "last_literal"),
                    Py_BuildValue ("i", op_array->last_literal));

    // op_array->literals
    PyObject *literals_array_pyobject = PyList_New (0);
    if (op_array->literals)
        {
            int j;
            for (j = 0; j < op_array->last_literal; j++)
                {
                    zval *literal_val = &op_array->literals[j];
                    PyList_Append (literals_array_pyobject,
                                   zval_to_python_object (literal_val));
                }
        }
    PyDict_SetItem (fn_extra_pobject, Py_BuildValue ("s", "literals"),
                    literals_array_pyobject);

    PyDict_SetItem (fn_pobject, Py_BuildValue ("s", "extra"), fn_extra_pobject);
    return true;
}

static int
zval_string_is_ascii (zval *val)
{
    int status = 1;
    for (int i = 0; i < Z_STRLEN_P (val); ++i)
        {
            if ((!isascii (Z_STRVAL_P (val)[i])
                 && (!isprint (Z_STRVAL_P (val)[i]))))
                {
                    status = 0;
                    break;
                }
        }
    return status;
}

static PyObject *
zval_to_python_object (zval *val)
{
    PyObject *zval_pyobject = PyDict_New ();
    char *decode = NULL;

    PyDict_SetItem (zval_pyobject, Py_BuildValue ("s", "type"),
                    Py_BuildValue ("s", "unknown"));

    PyDict_SetItem (zval_pyobject, Py_BuildValue ("s", "value"),
                    Py_BuildValue ("s", ""));

    // types defined in Zend/zend_types.h
    switch (Z_TYPE_P (val))
        {
        case IS_STRING:
            PyDict_SetItem (zval_pyobject, Py_BuildValue ("s", "type"),
                            Py_BuildValue ("s", "string"));

            // printf("==== %ld, %s %d\n",
            //        Z_STRLEN_P(val),
            //        Z_STRVAL_P(val),
            //        zval_string_is_ascii(val));

            if (zval_string_is_ascii (val))
                {
                    PyDict_SetItem (zval_pyobject, Py_BuildValue ("s", "value"),
                                    Py_BuildValue ("s", Z_STRVAL_P (val)));
                }
            else
                {
                    PyDict_SetItem (zval_pyobject, Py_BuildValue ("s", "value"),
                                    Py_BuildValue ("s", ""));
                }

            break;
        case IS_DOUBLE:
            PyDict_SetItem (zval_pyobject, Py_BuildValue ("s", "type"),
                            Py_BuildValue ("s", "integer"));
            PyDict_SetItem (zval_pyobject, Py_BuildValue ("s", "value"),
                            Py_BuildValue ("i", Z_DVAL_P (val)));
            break;
        case IS_LONG:
            PyDict_SetItem (zval_pyobject, Py_BuildValue ("s", "type"),
                            Py_BuildValue ("s", "integer"));
            PyDict_SetItem (zval_pyobject, Py_BuildValue ("s", "value"),
                            Py_BuildValue ("i", Z_LVAL_P (val)));
            break;
        case IS_TRUE:
            PyDict_SetItem (zval_pyobject, Py_BuildValue ("s", "type"),
                            Py_BuildValue ("s", "boolean"));
            PyDict_SetItem (zval_pyobject, Py_BuildValue ("s", "value"),
                            Py_BuildValue ("s", "true"));
            break;
        case IS_FALSE:
            PyDict_SetItem (zval_pyobject, Py_BuildValue ("s", "type"),
                            Py_BuildValue ("s", "boolean"));
            PyDict_SetItem (zval_pyobject, Py_BuildValue ("s", "value"),
                            Py_BuildValue ("s", "false"));
            break;
        case IS_UNDEF:
            PyDict_SetItem (zval_pyobject, Py_BuildValue ("s", "type"),
                            Py_BuildValue ("s", "undef"));
            break;
        case IS_RESOURCE:
            PyDict_SetItem (zval_pyobject, Py_BuildValue ("s", "type"),
                            Py_BuildValue ("s", "resource"));

            spprintf (&decode, 0, "Rsrc #%d", Z_RES_HANDLE_P (val));

            PyDict_SetItem (zval_pyobject, Py_BuildValue ("s", "value"),
                            Py_BuildValue ("s", decode));
            break;
        case IS_REFERENCE:
            PyDict_SetItem (zval_pyobject, Py_BuildValue ("s", "type"),
                            Py_BuildValue ("s", "reference"));
            break;
        case IS_OBJECT:
            PyDict_SetItem (zval_pyobject, Py_BuildValue ("s", "type"),
                            Py_BuildValue ("s", "object"));

            zend_string *str = Z_OBJCE_P (val)->name;
            spprintf (&decode, 0, "%s", ZSTR_VAL (str));

            PyDict_SetItem (zval_pyobject, Py_BuildValue ("s", "value"),
                            Py_BuildValue ("s", decode));
            break;
        case IS_ARRAY:
            PyDict_SetItem (zval_pyobject, Py_BuildValue ("s", "type"),
                            Py_BuildValue ("s", "array"));

            spprintf (&decode, 0, "array(%d)",
                      zend_hash_num_elements (Z_ARR_P (val)));

            PyDict_SetItem (zval_pyobject, Py_BuildValue ("s", "value"),
                            Py_BuildValue ("s", decode));
            break;
        case IS_NULL:
            PyDict_SetItem (zval_pyobject, Py_BuildValue ("s", "type"),
                            Py_BuildValue ("s", "null"));
            PyDict_SetItem (zval_pyobject, Py_BuildValue ("s", "value"),
                            Py_BuildValue ("s", estrdup ("null")));
            break;
        default:
            spprintf (&decode, 0, "unknown type: %d", Z_TYPE_P (val));

            PyDict_SetItem (zval_pyobject, Py_BuildValue ("s", "value"),
                            Py_BuildValue ("s", decode));
            break;
        }
    return zval_pyobject;
}

static PyObject *
get_znode_operand (const zend_op_array *op_array, const zend_op *opline,
                   znode_op op, zend_uchar op_type, uint32_t flags)
{
    /***************************************
        typedef union _znode_op {
            uint32_t      constant;
            uint32_t      var;
            uint32_t      num;
            uint32_t      opline_num;
        #if ZEND_USE_ABS_JMP_ADDR
            zend_op       *jmp_addr;
        #else
            uint32_t      jmp_offset;
        #endif
        #if ZEND_USE_ABS_CONST_ADDR
            zval          *zv;
        #endif
        } znode_op;
    ***************************************/

    PyObject *znode_pyobject = PyDict_New ();

    PyDict_SetItem (znode_pyobject, Py_BuildValue ("s", "type"),
                    Py_BuildValue ("s", "UNKNOWN"));

    PyDict_SetItem (znode_pyobject, Py_BuildValue ("s", "value"),
                    Py_BuildValue ("s", ""));

    PyDict_SetItem (znode_pyobject, Py_BuildValue ("s", "variable_number"),
                    Py_BuildValue ("i", EX_VAR_TO_NUM (op.var)));

    if (op_type == IS_UNUSED)
        {
            char *decode = NULL;

            if (ZEND_VM_OP_JMP_ADDR == (flags & ZEND_VM_OP_MASK))
                {
                    // spprintf(&decode, 0, "J%td", OP_JMP_ADDR(opline, op) -
                    // op_array->opcodes);
                }
            else if (ZEND_VM_OP_NUM == (flags & ZEND_VM_OP_MASK))
                {
                    spprintf (&decode, 0, "%" PRIu32, op.num);
                }
            else if (ZEND_VM_OP_TRY_CATCH == (flags & ZEND_VM_OP_MASK))
                {
                    if (op.num != (uint32_t)-1)
                        {
                            spprintf (&decode, 0, "try-catch(%" PRIu32 ")",
                                      op.num);
                        }
                }
            else if (ZEND_VM_OP_THIS == (flags & ZEND_VM_OP_MASK))
                {
                    decode = estrdup ("THIS");
                }
            else if (ZEND_VM_OP_NEXT == (flags & ZEND_VM_OP_MASK))
                {
                    decode = estrdup ("NEXT");
                }
            else if (ZEND_VM_OP_CLASS_FETCH == (flags & ZEND_VM_OP_MASK))
                {
                }
            else if (ZEND_VM_OP_CONSTRUCTOR == (flags & ZEND_VM_OP_MASK))
                {
                    decode = estrdup ("CONSTRUCTOR");
                }

            PyDict_SetItem (znode_pyobject, Py_BuildValue ("s", "type"),
                            Py_BuildValue ("s", "IS_UNUSED"));

            PyDict_SetItem (znode_pyobject, Py_BuildValue ("s", "value"),
                            Py_BuildValue ("s", decode));
        }

    if (op_type == IS_CONST)
        {
            PyDict_SetItem (znode_pyobject, Py_BuildValue ("s", "type"),
                            Py_BuildValue ("s", "IS_CONST"));

            PyDict_SetItem (znode_pyobject, Py_BuildValue ("s", "value"),
                            Py_BuildValue ("s", ""));

            zval *const_literal = RT_CONSTANT (opline, op);
            if (const_literal)
                {
                    PyObject *const_literal_pyobject
                        = zval_to_python_object (const_literal);

                    PyDict_SetItem (
                        znode_pyobject, Py_BuildValue ("s", "value"),
                        PyDict_GetItem (const_literal_pyobject,
                                        Py_BuildValue ("s", "value")));
                }
        }

    if (op_type == IS_TMP_VAR)
        {
            char *decode = NULL;
            spprintf (&decode, 0, "T%u", EX_VAR_TO_NUM (op.var));

            PyDict_SetItem (znode_pyobject, Py_BuildValue ("s", "type"),
                            Py_BuildValue ("s", "IS_TMP_VAR"));

            PyDict_SetItem (znode_pyobject, Py_BuildValue ("s", "value"),
                            Py_BuildValue ("s", decode));
        }

    if (op_type == IS_VAR)
        {
            char *decode = NULL;
            spprintf (&decode, 0, "@%u", EX_VAR_TO_NUM (op.var));

            PyDict_SetItem (znode_pyobject, Py_BuildValue ("s", "type"),
                            Py_BuildValue ("s", "IS_VAR"));

            PyDict_SetItem (znode_pyobject, Py_BuildValue ("s", "value"),
                            Py_BuildValue ("s", decode));
        }

    if (op_type == IS_CV)
        {
            char *decode = NULL;
            zend_string *var = op_array->vars[EX_VAR_TO_NUM (op.var)];

            spprintf (&decode, 0, "$%.*s%c",
                      ZSTR_LEN (var) <= 19 ? (int)ZSTR_LEN (var) : 18,
                      ZSTR_VAL (var), ZSTR_LEN (var) <= 19 ? 0 : '+');

            PyDict_SetItem (znode_pyobject, Py_BuildValue ("s", "type"),
                            Py_BuildValue ("s", "IS_CV"));

            PyDict_SetItem (znode_pyobject, Py_BuildValue ("s", "value"),
                            Py_BuildValue ("s", decode));
        }

    return znode_pyobject;
}

static bool
generate_instructions_info (const zend_op_array *op_array, PyObject *fn_pobject)
{
    PyObject *insns_pyobject = PyList_New (0);
    for (int i = 0; i < op_array->last; i++)
        {
            zend_op *op = &op_array->opcodes[i];
            uint32_t flags = zend_get_opcode_flags (op->opcode);

            /***************************************************************************
            // php-7.3.31
            struct _zend_op {
                const void *handler;
                znode_op op1;               // input operand
                znode_op op2;               // input operand
                znode_op result;            // one output operand result
                uint32_t extended_value;
                uint32_t lineno;
                zend_uchar opcode;          // determining the instruction type
                zend_uchar op1_type;
                zend_uchar op2_type;
                zend_uchar result_type;
            };
            *****************************************************************************/
            PyObject *dict = PyDict_New ();

            // op->op1
            PyDict_SetItem (dict, Py_BuildValue ("s", "op1"),
                            get_znode_operand (op_array, op, op->op1,
                                               op->op1_type,
                                               ZEND_VM_OP1_FLAGS (flags)));

            // op->op2
            PyDict_SetItem (dict, Py_BuildValue ("s", "op2"),
                            get_znode_operand (op_array, op, op->op2,
                                               op->op2_type,
                                               ZEND_VM_OP2_FLAGS (flags)));

            // op->result
            switch (op->opcode)
                {
                case ZEND_CATCH:
                    if (op->extended_value & ZEND_LAST_CATCH)
                        {
                            PyObject *znode_pyobject = PyDict_New ();
                            PyDict_SetItem (dict, Py_BuildValue ("s", "op2"),
                                            znode_pyobject);
                        }
                    PyDict_SetItem (dict, Py_BuildValue ("s", "result"),
                                    get_znode_operand (op_array, op, op->result,
                                                       op->result_type, -1));
                    break;
                default:
                    PyDict_SetItem (dict, Py_BuildValue ("s", "result"),
                                    get_znode_operand (op_array, op, op->result,
                                                       op->result_type, -1));
                    break;
                }

            // num
            PyDict_SetItem (dict, Py_BuildValue ("s", "num"),
                            Py_BuildValue ("i", i));

            // op->extended_value
            PyDict_SetItem (dict, Py_BuildValue ("s", "extended_value"),
                            Py_BuildValue ("i", op->extended_value));

            // op->lineno
            PyDict_SetItem (dict, Py_BuildValue ("s", "lineno"),
                            Py_BuildValue ("i", op->lineno));

            // op->opcode
            PyDict_SetItem (dict, Py_BuildValue ("s", "opcode"),
                            Py_BuildValue ("i", op->opcode));

            // op->opcode_name
            const char *opcode_name = zend_get_opcode_name (op->opcode);
            PyDict_SetItem (dict, Py_BuildValue ("s", "opcode_name"),
                            Py_BuildValue ("s", ""));

            if (opcode_name)
                {
                    PyDict_SetItem (dict, Py_BuildValue ("s", "opcode_name"),
                                    Py_BuildValue ("s", opcode_name));
                }

            // op->opcode_flags
            PyDict_SetItem (
                dict, Py_BuildValue ("s", "opcode_flags"),
                Py_BuildValue ("i", zend_get_opcode_flags (op->opcode)));

            // op->op1_type
            PyDict_SetItem (dict, Py_BuildValue ("s", "op1_type"),
                            Py_BuildValue ("i", op->op1_type));

            // op->op2_type
            PyDict_SetItem (dict, Py_BuildValue ("s", "op2_type"),
                            Py_BuildValue ("i", op->op2_type));

            // op->result_type
            PyDict_SetItem (dict, Py_BuildValue ("s", "result_type"),
                            Py_BuildValue ("i", op->result_type));

            PyList_Append (insns_pyobject, dict);
        }

    PyDict_SetItem (fn_pobject, Py_BuildValue ("s", "instructions"),
                    insns_pyobject);
    return true;
}

static bool
generate_cfg_info (const zend_op_array *op_array, zend_file_handle *file_handle,
                   PyObject *fn_pobject)
{
    PyObject *cfg_pyobject = PyDict_New ();

    zend_cfg cfg;
    zend_arena *arena = zend_arena_create (64 * 1024);
    void *checkpoint = zend_arena_checkpoint (arena);

    if (zend_build_cfg (&arena, op_array, ZEND_CFG_SPLIT_AT_LIVE_RANGES, &cfg)
        != SUCCESS)
        {
            zend_arena_release (&arena, checkpoint);
            printf ("Error building cfg\n");
            return false;
        }

    if (zend_cfg_build_predecessors (&arena, &cfg) != SUCCESS)
        {
            printf ("Error building cfg predecessors\n");
            return false;
        }

    PyDict_SetItem (cfg_pyobject, Py_BuildValue ("s", "blocks_count"),
                    Py_BuildValue ("i", cfg.blocks_count));

    PyDict_SetItem (cfg_pyobject, Py_BuildValue ("s", "edges_count"),
                    Py_BuildValue ("i", cfg.edges_count));

    PyObject *cfg_blocks_pyobject = PyList_New (0);
    if (cfg.blocks)
        {
            int k;
            for (k = 0; k < cfg.blocks_count; k++)
                {
                    PyObject *block_pyobject = PyDict_New ();
                    zend_basic_block *block = &cfg.blocks[k];

                    // first opcode number
                    PyDict_SetItem (block_pyobject,
                                    Py_BuildValue ("s", "start"),
                                    Py_BuildValue ("i", block->start));

                    // number of opcodes
                    PyDict_SetItem (block_pyobject, Py_BuildValue ("s", "len"),
                                    Py_BuildValue ("i", block->len));

                    // number of successors
                    PyDict_SetItem (
                        block_pyobject, Py_BuildValue ("s", "successors_count"),
                        Py_BuildValue ("i", block->successors_count));

                    // number of predecessors
                    PyDict_SetItem (
                        block_pyobject,
                        Py_BuildValue ("s", "predecessors_count"),
                        Py_BuildValue ("i", block->predecessors_count));

                    // offset of 1-st predecessor
                    PyDict_SetItem (
                        block_pyobject,
                        Py_BuildValue ("s", "predecessor_offset"),
                        Py_BuildValue ("i", block->predecessor_offset));

                    // immediate dominator block
                    PyDict_SetItem (block_pyobject, Py_BuildValue ("s", "idom"),
                                    Py_BuildValue ("i", block->idom));

                    // closest loop header, or -1
                    PyDict_SetItem (block_pyobject,
                                    Py_BuildValue ("s", "loop_header"),
                                    Py_BuildValue ("i", block->loop_header));

                    // steps away from the entry in the dom. tree
                    PyDict_SetItem (block_pyobject,
                                    Py_BuildValue ("s", "level"),
                                    Py_BuildValue ("i", block->level));

                    // list of dominated blocks
                    PyDict_SetItem (block_pyobject,
                                    Py_BuildValue ("s", "children"),
                                    Py_BuildValue ("i", block->children));

                    // next dominated block
                    PyDict_SetItem (block_pyobject,
                                    Py_BuildValue ("s", "next_child"),
                                    Py_BuildValue ("i", block->next_child));

                    // up to 2 successor blocks
                    PyObject *sstorage_pyobject = PyList_New (0);
                    PyList_Append (
                        sstorage_pyobject,
                        Py_BuildValue ("i", block->successors_storage[0]));
                    PyList_Append (
                        sstorage_pyobject,
                        Py_BuildValue ("i", block->successors_storage[1]));
                    PyDict_SetItem (block_pyobject,
                                    Py_BuildValue ("s", "successors_storage"),
                                    sstorage_pyobject);

                    // successor block indices
                    PyObject *successor_block_indices_pyobject = PyList_New (0);
                    if (block->successors_count > 0)
                        {
                            for (int i = 0; i < block->successors_count; i++)
                                {
                                    PyList_Append (
                                        successor_block_indices_pyobject,
                                        Py_BuildValue ("i",
                                                       block->successors[i]));
                                }
                        }
                    PyDict_SetItem (block_pyobject,
                                    Py_BuildValue ("s", "successors"),
                                    successor_block_indices_pyobject);
                    PyList_Append (cfg_blocks_pyobject, block_pyobject);
                }
        }
    PyDict_SetItem (cfg_pyobject, Py_BuildValue ("s", "blocks"),
                    cfg_blocks_pyobject);

    PyObject *predecessors_pyobject = PyList_New (0);
    if (cfg.predecessors)
        {
            int k;
            for (k = 0; k < cfg.edges_count; k++)
                {
                    int predecessor = cfg.predecessors[k];
                    PyList_Append (predecessors_pyobject,
                                   Py_BuildValue ("i", predecessor));
                }
        }
    PyDict_SetItem (cfg_pyobject, Py_BuildValue ("s", "predecessors"),
                    predecessors_pyobject);

    PyDict_SetItem (fn_pobject, Py_BuildValue ("s", "cfg"), cfg_pyobject);

    bool dfg_status = generate_dfg_info (op_array, &cfg, fn_pobject);
    if (dfg_status == false)
        {
            return false;
        }

    bool ssa_status
        = generate_ssa_info (op_array, &cfg, file_handle, fn_pobject);
    if (ssa_status == false)
        {
            return false;
        }

    zend_arena_release (&arena, checkpoint);
    zend_arena_destroy (arena);
    return true;
}

static void
zend_dump_variable (const zend_op_array *op_array, int var_num,
                    PyObject *var_pyobject)
{
    PyDict_SetItem (var_pyobject, Py_BuildValue ("s", "var_num"),
                    Py_BuildValue ("i", var_num));

    if (var_num < op_array->last_var)
        {
            PyDict_SetItem (var_pyobject, Py_BuildValue ("s", "var_name"),
                            Py_BuildValue ("s", op_array->vars[var_num]->val));
        }
    else
        {
            PyDict_SetItem (var_pyobject, Py_BuildValue ("s", "var_name"),
                            Py_BuildValue ("s", ""));
        }
}

static PyObject *
zend_dump_var_set (const zend_op_array *op_array, const char *name,
                   zend_bitset set)
{
    int first = 1;
    uint32_t i;
    PyObject *var_set_pyobject_vars = PyList_New (0);

    for (i = 0; i < op_array->last_var + op_array->T; i++)
        {
            if (zend_bitset_in (set, i))
                {
                    if (first)
                        {
                            first = 0;
                        }
                    PyObject *var_value = PyDict_New ();
                    zend_dump_variable (op_array, i, var_value);
                    PyList_Append (var_set_pyobject_vars, var_value);
                }
        }
    return var_set_pyobject_vars;
}

static bool
generate_dfg_info (const zend_op_array *op_array, zend_cfg *cfg,
                   PyObject *fn_pobject)
{
    zend_dfg dfg;
    int blocks_count = cfg->blocks_count;
    uint32_t set_size;
    uint32_t build_flags = 0;
    ALLOCA_FLAG (dfg_use_heap)

    dfg.vars = op_array->last_var + op_array->T;
    dfg.size = set_size = zend_bitset_len (dfg.vars);
    dfg.tmp
        = do_alloca ((set_size * sizeof (zend_ulong)) * (blocks_count * 4 + 1),
                     dfg_use_heap);
    memset (dfg.tmp, 0,
            (set_size * sizeof (zend_ulong)) * (blocks_count * 4 + 1));
    dfg.def = dfg.tmp + set_size;
    dfg.use = dfg.def + set_size * blocks_count;
    dfg.in = dfg.use + set_size * blocks_count;
    dfg.out = dfg.in + set_size * blocks_count;

    if (zend_build_dfg (op_array, cfg, &dfg, build_flags) != SUCCESS)
        {
            free_alloca (dfg.tmp, dfg_use_heap);
            return false;
        }

    PyObject *dfg_pyobject = PyList_New (0);
    if (cfg->blocks)
        {
            for (int j = 0; j < cfg->blocks_count; j++)
                {

                    PyObject *var_set_value_def = zend_dump_var_set (
                        op_array, "def", DFG_BITSET (dfg.def, dfg.size, j));
                    PyObject *var_set_value_use = zend_dump_var_set (
                        op_array, "use", DFG_BITSET (dfg.use, dfg.size, j));
                    PyObject *var_set_value_in = zend_dump_var_set (
                        op_array, "in ", DFG_BITSET (dfg.in, dfg.size, j));
                    PyObject *var_set_value_out = zend_dump_var_set (
                        op_array, "out", DFG_BITSET (dfg.out, dfg.size, j));
                    PyObject *var_set_value_tmp = zend_dump_var_set (
                        op_array, "tmp", DFG_BITSET (dfg.tmp, dfg.size, j));

                    PyObject *var_set_values_pyobject = PyDict_New ();

                    PyDict_SetItem (var_set_values_pyobject,
                                    Py_BuildValue ("s", "block_index"),
                                    Py_BuildValue ("i", j));

                    PyDict_SetItem (var_set_values_pyobject,
                                    Py_BuildValue ("s", "var_def"),
                                    var_set_value_def);

                    PyDict_SetItem (var_set_values_pyobject,
                                    Py_BuildValue ("s", "var_use"),
                                    var_set_value_use);

                    PyDict_SetItem (var_set_values_pyobject,
                                    Py_BuildValue ("s", "var_in"),
                                    var_set_value_in);

                    PyDict_SetItem (var_set_values_pyobject,
                                    Py_BuildValue ("s", "var_out"),
                                    var_set_value_out);

                    PyDict_SetItem (var_set_values_pyobject,
                                    Py_BuildValue ("s", "var_tmp"),
                                    var_set_value_tmp);

                    PyList_Append (dfg_pyobject, var_set_values_pyobject);
                }
        }

    PyDict_SetItem (fn_pobject, Py_BuildValue ("s", "dfg"), dfg_pyobject);
    return true;
}

static void
zend_ssa_pi_constraint_to_pyobject (zend_ssa_pi_constraint *constraint,
                                    PyObject *constraint_pyobject)
{
    PyDict_SetItem (constraint_pyobject, Py_BuildValue ("s", "range_range_min"),
                    Py_BuildValue ("i", constraint->range.range.min));

    PyDict_SetItem (constraint_pyobject, Py_BuildValue ("s", "range_range_max"),
                    Py_BuildValue ("i", constraint->range.range.max));

    PyDict_SetItem (constraint_pyobject,
                    Py_BuildValue ("s", "range_range_underflow"),
                    Py_BuildValue ("i", constraint->range.range.underflow));

    PyDict_SetItem (constraint_pyobject,
                    Py_BuildValue ("s", "range_range_overflow"),
                    Py_BuildValue ("i", constraint->range.range.overflow));

    PyDict_SetItem (constraint_pyobject, Py_BuildValue ("s", "range_min_var"),
                    Py_BuildValue ("i", constraint->range.min_var));

    PyDict_SetItem (constraint_pyobject, Py_BuildValue ("s", "range_max_var"),
                    Py_BuildValue ("i", constraint->range.max_var));

    // ((min_var>0) ? MIN(ssa_var) : 0) + range.min
    PyDict_SetItem (constraint_pyobject,
                    Py_BuildValue ("s", "range_min_ssa_var"),
                    Py_BuildValue ("i", constraint->range.min_ssa_var));

    // ((max_var>0) ? MAX(ssa_var) : 0) + range.max
    PyDict_SetItem (constraint_pyobject,
                    Py_BuildValue ("s", "range_max_ssa_var"),
                    Py_BuildValue ("i", constraint->range.max_ssa_var));

    // zend_ssa_negative_lat check = POS_INF;
    switch (constraint->range.negative)
        {
        case NEG_NONE:
            PyDict_SetItem (constraint_pyobject,
                            Py_BuildValue ("s", "range_negative"),
                            Py_BuildValue ("s", "NEG_NONE"));
            break;
        case NEG_INIT:
            PyDict_SetItem (constraint_pyobject,
                            Py_BuildValue ("s", "range_negative"),
                            Py_BuildValue ("s", "NEG_INIT"));
            break;
        case NEG_INVARIANT:
            PyDict_SetItem (constraint_pyobject,
                            Py_BuildValue ("s", "range_negative"),
                            Py_BuildValue ("s", "NEG_INVARIANT"));
            break;
        case NEG_USE_LT:
            PyDict_SetItem (constraint_pyobject,
                            Py_BuildValue ("s", "range_negative"),
                            Py_BuildValue ("s", "NEG_USE_LT"));
            break;
        case NEG_USE_GT:
            PyDict_SetItem (constraint_pyobject,
                            Py_BuildValue ("s", "range_negative"),
                            Py_BuildValue ("s", "NEG_USE_GT"));
            break;
        default:
            PyDict_SetItem (constraint_pyobject,
                            Py_BuildValue ("s", "range_negative"),
                            Py_BuildValue ("s", "NEG_UNKNOWN"));
            break;
        }
}

static void
zend_ssa_phi_to_pyobject (const zend_op_array *op_array, const zend_ssa *ssa,
                          zend_ssa_phi *phi, PyObject *phi_pyobject)
{
    // if >= 0 this is actually a e-SSA Pi
    PyDict_SetItem (phi_pyobject, Py_BuildValue ("s", "pi"),
                    Py_BuildValue ("i", phi->pi));

    // Original CV, VAR or TMP variable index
    PyDict_SetItem (phi_pyobject, Py_BuildValue ("s", "variable_index"),
                    Py_BuildValue ("i", phi->var));

    PyObject *var_pyobject = PyDict_New ();
    if (phi->var >= 0)
        {
            zend_dump_variable (op_array, phi->var, var_pyobject);
        }
    PyDict_SetItem (phi_pyobject, Py_BuildValue ("s", "variable"),
                    var_pyobject);

    // SSA variable index
    PyDict_SetItem (phi_pyobject, Py_BuildValue ("s", "ssa_variable_index"),
                    Py_BuildValue ("i", phi->ssa_var));

    // current BB index
    PyDict_SetItem (phi_pyobject, Py_BuildValue ("s", "current_block_index"),
                    Py_BuildValue ("i", phi->block));

    // flag to avoid recursive processing
    PyDict_SetItem (phi_pyobject, Py_BuildValue ("s", "visited"),
                    Py_BuildValue ("i", phi->visited));

    PyDict_SetItem (phi_pyobject, Py_BuildValue ("s", "has_range_constraint"),
                    Py_BuildValue ("i", phi->has_range_constraint));

    PyObject *constraint_pyobject = PyDict_New ();
    if (phi->has_range_constraint)
        {
            zend_ssa_pi_constraint_to_pyobject (&phi->constraint,
                                                constraint_pyobject);
        }
    PyDict_SetItem (phi_pyobject, Py_BuildValue ("s", "constraint"),
                    constraint_pyobject);

    PyObject *sources_pyobject = PyList_New (0);
    if (phi->sources)
        {
            int current_block_index = phi->block;
            for (int i = 0;
                 i < ssa->cfg.blocks[current_block_index].predecessors_count;
                 ++i)
                {
                    PyList_Append (sources_pyobject,
                                   Py_BuildValue ("i", phi->sources[i]));
                }
        }
    PyDict_SetItem (phi_pyobject, Py_BuildValue ("s", "sources"),
                    sources_pyobject);
}

static void
zend_dump_ssa_variable (const zend_op_array *op_array, const zend_ssa *ssa,
                        int ssa_var_num, int var_num, PyObject *var_pyobject)
{
    PyDict_SetItem (var_pyobject, Py_BuildValue ("s", "ssa_var_num"),
                    Py_BuildValue ("i", -1));

    if (ssa_var_num >= 0)
        PyDict_SetItem (var_pyobject, Py_BuildValue ("s", "ssa_var_num"),
                        Py_BuildValue ("i", ssa_var_num));

    zend_dump_variable (op_array, var_num, var_pyobject);

    // opcode that defines this value
    PyDict_SetItem (var_pyobject, Py_BuildValue ("s", "definition"),
                    Py_BuildValue ("i", ssa->vars[ssa_var_num].definition));

    // phi that defines this value
    PyObject *definition_phi_pyobject = PyDict_New ();
    if (ssa->vars[ssa_var_num].definition_phi)
        {
            zend_ssa_phi *phi = ssa->vars[ssa_var_num].definition_phi;
            zend_ssa_phi_to_pyobject (op_array, ssa, phi,
                                      definition_phi_pyobject);
        }
    PyDict_SetItem (var_pyobject, Py_BuildValue ("s", "definition_phi"),
                    definition_phi_pyobject);

    // value doesn't mater (used as op1 in ZEND_ASSIGN)
    PyDict_SetItem (var_pyobject, Py_BuildValue ("s", "no_val"),
                    Py_BuildValue ("i", ssa->vars[ssa_var_num].no_val));

    // uses of this value, linked through opN_use_chain
    PyDict_SetItem (var_pyobject, Py_BuildValue ("s", "use_chain"),
                    Py_BuildValue ("i", ssa->vars[ssa_var_num].use_chain));

    PyDict_SetItem (var_pyobject, Py_BuildValue ("s", "escape_state"),
                    Py_BuildValue ("i", ssa->vars[ssa_var_num].escape_state));
}

static PyObject *
zend_dump_ssa_variables (const zend_op_array *op_array, const zend_ssa *ssa)
{
    PyObject *ssa_variables_pyobject = PyList_New (0);

    if (ssa->vars)
        {
            for (int ssa_var_num = 0; ssa_var_num < ssa->vars_count;
                 ssa_var_num++)
                {
                    PyObject *var_pyobject = PyDict_New ();
                    zend_dump_ssa_variable (op_array, ssa, ssa_var_num,
                                            ssa->vars[ssa_var_num].var,
                                            var_pyobject);

                    // set scc
                    PyDict_SetItem (
                        var_pyobject,
                        Py_BuildValue ("s", "strongly_connected_component"),
                        Py_BuildValue ("i", ssa->vars[ssa_var_num].scc));

                    // set default to false
                    PyDict_SetItem (
                        var_pyobject,
                        Py_BuildValue ("s",
                                       "strongly_connected_component_entry"),
                        Py_BuildValue ("i", 1));

                    if (ssa->vars[ssa_var_num].scc >= 0)
                        {
                            if (ssa->vars[ssa_var_num].scc_entry)
                                {
                                    // set to true
                                    PyDict_SetItem (
                                        var_pyobject,
                                        Py_BuildValue ("s",
                                                       "strongly_connected_"
                                                       "component_entry"),
                                        Py_BuildValue ("i", 0));
                                }
                        }

                    PyList_Append (ssa_variables_pyobject, var_pyobject);
                }
        }
    return ssa_variables_pyobject;
}

static PyObject *
zend_dump_ssa_instructions (const zend_op_array *op_array, const zend_ssa *ssa)
{
    PyObject *ssa_instructions_pyobject = PyList_New (0);

    /*
    op1_use -> op1_def
    op2_use -> op2_def
    result_use -> result_def

    takes *_use
    *_def: maps to def if greater than -1. creates a def of that variable stored
    in _def

    */

    if (ssa->ops)
        {
            for (int i = op_array->last - 1; i >= 0; i--)
                {
                    zend_ssa_op *op = ssa->ops + i;
                    PyObject *op_pyobject = PyDict_New ();

                    PyDict_SetItem (op_pyobject, Py_BuildValue ("s", "op1_use"),
                                    Py_BuildValue ("i", op->op1_use));

                    PyDict_SetItem (op_pyobject, Py_BuildValue ("s", "op2_use"),
                                    Py_BuildValue ("i", op->op2_use));

                    PyDict_SetItem (op_pyobject,
                                    Py_BuildValue ("s", "result_use"),
                                    Py_BuildValue ("i", op->result_use));

                    PyDict_SetItem (op_pyobject, Py_BuildValue ("s", "op1_def"),
                                    Py_BuildValue ("i", op->op1_def));

                    PyDict_SetItem (op_pyobject, Py_BuildValue ("s", "op2_def"),
                                    Py_BuildValue ("i", op->op2_def));

                    PyDict_SetItem (op_pyobject,
                                    Py_BuildValue ("s", "result_def"),
                                    Py_BuildValue ("i", op->result_def));

                    PyDict_SetItem (op_pyobject,
                                    Py_BuildValue ("s", "op1_use_chain"),
                                    Py_BuildValue ("i", op->op1_use_chain));

                    PyDict_SetItem (op_pyobject,
                                    Py_BuildValue ("s", "op2_use_chain"),
                                    Py_BuildValue ("i", op->op2_use_chain));

                    PyDict_SetItem (op_pyobject,
                                    Py_BuildValue ("s", "res_use_chain"),
                                    Py_BuildValue ("i", op->res_use_chain));

                    PyList_Append (ssa_instructions_pyobject, op_pyobject);
                }
        }
    return ssa_instructions_pyobject;
}

static PyObject *
zend_dump_ssa_blocks (const zend_op_array *op_array, const zend_ssa *ssa)
{
    PyObject *ssa_blocks_pyobject = PyList_New (0);

    if (ssa->blocks)
        {
            for (int i = 0; i < ssa->cfg.blocks_count; i++)
                {
                    PyObject *block_pyobject = PyDict_New ();

                    PyDict_SetItem (block_pyobject,
                                    Py_BuildValue ("s", "block_index"),
                                    Py_BuildValue ("i", i));

                    PyObject *phis_pyobject = PyList_New (0);
                    zend_ssa_phi *phi = ssa->blocks[i].phis;
                    while (phi)
                        {
                            PyObject *phi_pyobject = PyDict_New ();
                            zend_ssa_phi_to_pyobject (op_array, ssa, phi,
                                                      phi_pyobject);

                            PyDict_SetItem (block_pyobject,
                                            Py_BuildValue ("s", "phis"),
                                            phi_pyobject);

                            PyList_Append (phis_pyobject, phi_pyobject);
                            phi = phi->next;
                        }

                    PyDict_SetItem (block_pyobject, Py_BuildValue ("s", "phis"),
                                    phis_pyobject);

                    PyList_Append (ssa_blocks_pyobject, block_pyobject);
                }
        }
    return ssa_blocks_pyobject;
}

static PyObject *
zend_dump_zend_ssa (const zend_op_array *op_array, const zend_ssa *ssa)
{
    PyObject *ssa_pyobject = PyDict_New ();

    // number of SCCs
    PyDict_SetItem (ssa_pyobject, Py_BuildValue ("s", "number_of_sccs"),
                    Py_BuildValue ("i", ssa->sccs));

    // number of SSA variables
    PyDict_SetItem (ssa_pyobject,
                    Py_BuildValue ("s", "number_of_ssa_variables"),
                    Py_BuildValue ("i", ssa->vars_count));

    // dump ssa vars (ssa->vars)
    PyDict_SetItem (ssa_pyobject, Py_BuildValue ("s", "ssa_variables"),
                    zend_dump_ssa_variables (op_array, ssa));

    // dump array of SSA instructions (ssa->ops)
    PyDict_SetItem (ssa_pyobject, Py_BuildValue ("s", "ssa_instructions"),
                    zend_dump_ssa_instructions (op_array, ssa));

    // dump array of SSA blocks (ssa->blocks)
    PyDict_SetItem (ssa_pyobject, Py_BuildValue ("s", "ssa_blocks"),
                    zend_dump_ssa_blocks (op_array, ssa));

    return ssa_pyobject;
}

static bool
generate_ssa_info (const zend_op_array *op_array, zend_cfg *cfg,
                   zend_file_handle *file_handle, PyObject *fn_pobject)
{
    zend_ssa ssa;
    zend_script script;
    uint32_t build_flags = 0;

    script.first_early_binding_opline
        = (op_array->fn_flags & ZEND_ACC_EARLY_BINDING)
              ? zend_build_delayed_early_binding_list (op_array)
              : (uint32_t)-1;
    script.main_op_array = *op_array;

    if (file_handle->opened_path)
        {
            script.filename = zend_string_copy (file_handle->opened_path);
        }
    else
        {
            script.filename = zend_string_init (
                file_handle->filename, strlen (file_handle->filename), 0);
        }
    zend_string_hash_val (script.filename);

    /* Build SSA */
    memset (&ssa, 0, sizeof (zend_ssa));

    zend_arena *arena = zend_arena_create (64 * 1024);
    void *checkpoint = zend_arena_checkpoint (arena);

    if (zend_build_cfg (&arena, op_array, ZEND_CFG_SPLIT_AT_LIVE_RANGES,
                        &ssa.cfg)
        != SUCCESS)
        {
            printf ("Error building ssa cfg\n");
            zend_arena_release (&arena, checkpoint);
            return false;
        }

    if (zend_cfg_build_predecessors (&arena, &ssa.cfg) != SUCCESS)
        {
            printf ("Error building ssa cfg predecessors\n");
            return false;
        }

    if (zend_cfg_compute_dominators_tree (op_array, &ssa.cfg) != SUCCESS)
        {
            printf ("Error building ssa cfg dominators tree\n");
            zend_arena_release (&arena, checkpoint);
            return false;
        }

    if (zend_build_ssa (&arena, &script, op_array, build_flags, &ssa)
        != SUCCESS)
        {
            printf ("Error building ssa\n");
            zend_arena_release (&arena, checkpoint);
            return false;
        }

    if (zend_ssa_compute_use_def_chains (&arena, op_array, &ssa) != SUCCESS)
        {
            printf ("Error building ssa cfg use_def_chains\n");
            zend_arena_release (&arena, checkpoint);
            return false;
        }

    PyObject *ssa_pyobject = zend_dump_zend_ssa (op_array, &ssa);
    PyDict_SetItem (fn_pobject, Py_BuildValue ("s", "ssa"), ssa_pyobject);

    zend_arena_release (&arena, checkpoint);
    zend_arena_destroy (arena);
    return true;
}

static PyMethodDef OpArrayMethods[]
    = { { "convert", method_convert, METH_VARARGS,
          "Compile and Converts PHP code/script to Bytecode, CFG, DFG, SSA" },
        { NULL, NULL, 0, NULL } };


static struct PyModuleDef OpArrayDefinition
    = { PyModuleDef_HEAD_INIT, "convert",
        "Compile and Convert PHP code/script to Bytecode, CFG, DFG, SSA", -1,
        OpArrayMethods };

PyMODINIT_FUNC
PyInit_php_to_bytecode_converter (void)
{
    Py_Initialize ();
    return PyModule_Create (&OpArrayDefinition);
}