/*
   +----------------------------------------------------------------------+
   | Zend Engine, SSA - Static Single Assignment Form                     |
   +----------------------------------------------------------------------+
   | Copyright (c) 1998-2018 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Dmitry Stogov <dmitry@php.net>                              |
   +----------------------------------------------------------------------+
*/

#ifndef ZEND_SSA_EX_H
#define ZEND_SSA_EX_H

#include <stdint.h>
#include "php.h"
#include "zend_compile.h"
#include "zend_worklist_ex.h"
#include "php_cfg.h"

#define MAY_BE_UNDEF                (1 << IS_UNDEF)
#define MAY_BE_NULL		            (1 << IS_NULL)
#define MAY_BE_FALSE	            (1 << IS_FALSE)
#define MAY_BE_TRUE		            (1 << IS_TRUE)
#define MAY_BE_LONG		            (1 << IS_LONG)
#define MAY_BE_DOUBLE	            (1 << IS_DOUBLE)
#define MAY_BE_STRING	            (1 << IS_STRING)
#define MAY_BE_ARRAY	            (1 << IS_ARRAY)
#define MAY_BE_OBJECT	            (1 << IS_OBJECT)
#define MAY_BE_RESOURCE	            (1 << IS_RESOURCE)
#define MAY_BE_ANY                  (MAY_BE_NULL|MAY_BE_FALSE|MAY_BE_TRUE|MAY_BE_LONG|MAY_BE_DOUBLE|MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE)
#define MAY_BE_REF                  (1 << IS_REFERENCE) /* may be reference */

#define MAY_BE_ARRAY_SHIFT          (IS_REFERENCE)

#define MAY_BE_ARRAY_OF_NULL		(MAY_BE_NULL     << MAY_BE_ARRAY_SHIFT)
#define MAY_BE_ARRAY_OF_FALSE		(MAY_BE_FALSE    << MAY_BE_ARRAY_SHIFT)
#define MAY_BE_ARRAY_OF_TRUE		(MAY_BE_TRUE     << MAY_BE_ARRAY_SHIFT)
#define MAY_BE_ARRAY_OF_LONG		(MAY_BE_LONG     << MAY_BE_ARRAY_SHIFT)
#define MAY_BE_ARRAY_OF_DOUBLE		(MAY_BE_DOUBLE   << MAY_BE_ARRAY_SHIFT)
#define MAY_BE_ARRAY_OF_STRING		(MAY_BE_STRING   << MAY_BE_ARRAY_SHIFT)
#define MAY_BE_ARRAY_OF_ARRAY		(MAY_BE_ARRAY    << MAY_BE_ARRAY_SHIFT)
#define MAY_BE_ARRAY_OF_OBJECT		(MAY_BE_OBJECT   << MAY_BE_ARRAY_SHIFT)
#define MAY_BE_ARRAY_OF_RESOURCE	(MAY_BE_RESOURCE << MAY_BE_ARRAY_SHIFT)
#define MAY_BE_ARRAY_OF_ANY			(MAY_BE_ANY      << MAY_BE_ARRAY_SHIFT)
#define MAY_BE_ARRAY_OF_REF			(MAY_BE_REF      << MAY_BE_ARRAY_SHIFT)

#define MAY_BE_ARRAY_KEY_LONG       (1<<21)
#define MAY_BE_ARRAY_KEY_STRING     (1<<22)
#define MAY_BE_ARRAY_KEY_ANY        (MAY_BE_ARRAY_KEY_LONG | MAY_BE_ARRAY_KEY_STRING)

#define MAY_BE_ERROR                (1<<23)
#define MAY_BE_CLASS                (1<<24)

//TODO: remome MAY_BE_RC1, MAY_BE_RCN???
#define MAY_BE_RC1                  (1<<27) /* may be non-reference with refcount == 1 */
#define MAY_BE_RCN                  (1<<28) /* may be non-reference with refcount > 1  */

#define MAY_BE_IN_REG               (1<<25) /* value allocated in CPU register */

typedef struct _zend_script {
    zend_string   *filename;
    zend_op_array  main_op_array;
    HashTable      function_table;
    HashTable      class_table;
    uint32_t       first_early_binding_opline; /* the linked list of delayed declarations */
} zend_script;

typedef struct _zend_optimizer_ctx {
    zend_arena             *arena;
    zend_script            *script;
    HashTable              *constants;
    zend_long               optimization_level;
    zend_long               debug_level;
} zend_optimizer_ctx;

typedef struct _zend_ssa_range {
	zend_long              min;
	zend_long              max;
	zend_bool              underflow;
	zend_bool              overflow;
} zend_ssa_range;

typedef enum _zend_ssa_negative_lat {
	NEG_NONE      = 0,
	NEG_INIT      = 1,
	NEG_INVARIANT = 2,
	NEG_USE_LT    = 3,
	NEG_USE_GT    = 4,
	NEG_UNKNOWN   = 5
} zend_ssa_negative_lat;

/* Special kind of SSA Phi function used in eSSA */
typedef struct _zend_ssa_range_constraint {
	zend_ssa_range         range;       /* simple range constraint */
	int                    min_var;
	int                    max_var;
	int                    min_ssa_var; /* ((min_var>0) ? MIN(ssa_var) : 0) + range.min */
	int                    max_ssa_var; /* ((max_var>0) ? MAX(ssa_var) : 0) + range.max */
	zend_ssa_negative_lat  negative;
} zend_ssa_range_constraint;

typedef struct _zend_ssa_type_constraint {
	uint32_t               type_mask;   /* Type mask to intersect with */
	zend_class_entry      *ce;          /* Class entry for instanceof constraints */
} zend_ssa_type_constraint;

typedef union _zend_ssa_pi_constraint {
	zend_ssa_range_constraint range;
	zend_ssa_type_constraint type;
} zend_ssa_pi_constraint;

/* SSA Phi - ssa_var = Phi(source0, source1, ...sourceN) */
typedef struct _zend_ssa_phi zend_ssa_phi;
struct _zend_ssa_phi {
	zend_ssa_phi          *next;          /* next Phi in the same BB */
	int                    pi;            /* if >= 0 this is actually a e-SSA Pi */
	zend_ssa_pi_constraint constraint;    /* e-SSA Pi constraint */
	int                    var;           /* Original CV, VAR or TMP variable index */
	int                    ssa_var;       /* SSA variable index */
	int                    block;         /* current BB index */
	int                    visited : 1;   /* flag to avoid recursive processing */
	int                    has_range_constraint : 1;
	zend_ssa_phi         **use_chains;
	zend_ssa_phi          *sym_use_chain;
	int                   *sources;       /* Array of SSA IDs that produce this var.
									         As many as this block has
									         predecessors.  */
};

typedef struct _zend_ssa_block {
	zend_ssa_phi          *phis;
} zend_ssa_block;

typedef struct _zend_ssa_op {
	int                    op1_use;
	int                    op2_use;
	int                    result_use;
	int                    op1_def;
	int                    op2_def;
	int                    result_def;
	int                    op1_use_chain;
	int                    op2_use_chain;
	int                    res_use_chain;
} zend_ssa_op;

typedef enum _zend_ssa_alias_kind {
	NO_ALIAS,
	SYMTABLE_ALIAS,
	PHP_ERRORMSG_ALIAS,
	HTTP_RESPONSE_HEADER_ALIAS
} zend_ssa_alias_kind;

typedef enum _zend_ssa_escape_state {
	ESCAPE_STATE_UNKNOWN,
	ESCAPE_STATE_NO_ESCAPE,
	ESCAPE_STATE_FUNCTION_ESCAPE,
	ESCAPE_STATE_GLOBAL_ESCAPE
} zend_ssa_escape_state;

typedef struct _zend_ssa_var {
	int                    var;            /* original var number; op.var for CVs and following numbers for VARs and TMP_VARs */
	int                    scc;            /* strongly connected component */
	int                    definition;     /* opcode that defines this value */
	zend_ssa_phi          *definition_phi; /* phi that defines this value */
	int                    use_chain;      /* uses of this value, linked through opN_use_chain */
	zend_ssa_phi          *phi_use_chain;  /* uses of this value in Phi, linked through use_chain */
	zend_ssa_phi          *sym_use_chain;  /* uses of this value in Pi constraints */
	unsigned int           no_val : 1;     /* value doesn't mater (used as op1 in ZEND_ASSIGN) */
	unsigned int           scc_entry : 1;
	unsigned int           alias : 2;  /* value may be changed indirectly */
	unsigned int           escape_state : 2;
} zend_ssa_var;

typedef struct _zend_ssa_var_info {
	uint32_t               type; /* inferred type (see zend_inference.h) */
	zend_ssa_range         range;
	zend_class_entry      *ce;
	unsigned int           has_range : 1;
	unsigned int           is_instanceof : 1; /* 0 - class == "ce", 1 - may be child of "ce" */
	unsigned int           recursive : 1;
	unsigned int           use_as_double : 1;
} zend_ssa_var_info;

typedef struct _zend_ssa {
	zend_cfg               cfg;            /* control flow graph             */
	int                    rt_constants;   /* run-time or compile-time       */
	int                    vars_count;     /* number of SSA variables        */
	zend_ssa_block        *blocks;         /* array of SSA blocks            */
	zend_ssa_op           *ops;            /* array of SSA instructions      */
	zend_ssa_var          *vars;           /* use/def chain of SSA variables */
	int                    sccs;           /* number of SCCs                 */
	zend_ssa_var_info     *var_info;
} zend_ssa;

BEGIN_EXTERN_C()

int zend_build_ssa(zend_arena **arena, const zend_script *script, const zend_op_array *op_array, uint32_t build_flags, zend_ssa *ssa);
int zend_ssa_compute_use_def_chains(zend_arena **arena, const zend_op_array *op_array, zend_ssa *ssa);


#define NUM_PHI_SOURCES(phi) \
	((phi)->pi >= 0 ? 1 : (ssa->cfg.blocks[(phi)->block].predecessors_count))

/* FOREACH_USE and FOREACH_PHI_USE explicitly support "continue"
 * and changing the use chain of the current element */
#define FOREACH_USE(var, use) do { \
	int _var_num = (var) - ssa->vars, next; \
	for (use = (var)->use_chain; use >= 0; use = next) { \
		next = zend_ssa_next_use(ssa->ops, _var_num, use);
#define FOREACH_USE_END() \
	} \
} while (0)

#define FOREACH_PHI_USE(var, phi) do { \
	int _var_num = (var) - ssa->vars; \
	zend_ssa_phi *next_phi; \
	for (phi = (var)->phi_use_chain; phi; phi = next_phi) { \
		next_phi = zend_ssa_next_use_phi(ssa, _var_num, phi);
#define FOREACH_PHI_USE_END() \
	} \
} while (0)

#define FOREACH_PHI_SOURCE(phi, source) do { \
	zend_ssa_phi *_phi = (phi); \
	int _i, _end = NUM_PHI_SOURCES(phi); \
	for (_i = 0; _i < _end; _i++) { \
		ZEND_ASSERT(_phi->sources[_i] >= 0); \
		source = _phi->sources[_i];
#define FOREACH_PHI_SOURCE_END() \
	} \
} while (0)

#define FOREACH_PHI(phi) do { \
	int _i; \
	for (_i = 0; _i < ssa->cfg.blocks_count; _i++) { \
		phi = ssa->blocks[_i].phis; \
		for (; phi; phi = phi->next) {
#define FOREACH_PHI_END() \
		} \
	} \
} while (0)

#define FOREACH_BLOCK(block) do { \
	int _i; \
	for (_i = 0; _i < ssa->cfg.blocks_count; _i++) { \
		(block) = &ssa->cfg.blocks[_i]; \
		if (!((block)->flags & ZEND_BB_REACHABLE)) { \
			continue; \
		}
#define FOREACH_BLOCK_END() \
	} \
} while (0)

/* Does not support "break" */
#define FOREACH_INSTR_NUM(i) do { \
	zend_basic_block *_block; \
	FOREACH_BLOCK(_block) { \
		uint32_t _end = _block->start + _block->len; \
		for ((i) = _block->start; (i) < _end; (i)++) {
#define FOREACH_INSTR_NUM_END() \
		} \
	} FOREACH_BLOCK_END(); \
} while (0)

#endif /* ZEND_SSA_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */
