#include "types.h"
#include "debug.h"
#include "alloc-inl.h"
#include "config.h"

#include <stdio.h>
#include "cjson/cJSON.h"
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

typedef struct _range {
  u64 min;
  u64 max;
} _range;


/* Domain of the argument:
   Fixed data: e.g. "1-3,5,7,8-10"
*/
typedef struct _domain {
  u64    *discrete; /* Discrete[0]: number of discrete values used  */
  s32     d_size;   /* The real size malloced for discrete values   */
  _range *range;    /* The continuous values' range ([min, max])    */
  s32     r_size;   /* Total number of ranges bound to the argument */
  u64     total_n;  /* Total choices can the argument to choose     */
} _domain;

/* Structure of argument entry */
struct argument_entry {
  u8      type;      /* Record every argument's type     */
  s32     len;       /* Record length of init data       */
  cJSON  *value;     /* The pointer to struct in cJSON   */
  u8      skip_fuzz; /* If this argument skip fuzzing    */
  s8      name[64];  /* argument's alias                 */
  _domain domain;    /* The constraint of the argument   */
  u8     real_type;  /* argument's refined type          */
  u8     real_val_str[500];
};

struct input_data {
  u8 *data; /* Constructed data                 */
  u32 len;  /* Constructed data length          */
};

/* 
   Structure of api entry which include 
   the number of arguments and their's types etc.
*/
struct api_entry {
  s32                    argument_num;   /* Total number of arguments    */
  cJSON *                monitor_json;   /* Global monitor of the json   */
  struct argument_entry *args_info;	     /* Global argument_entry array  */
  s32                    cur_arg_id;     /* Current arg_id while fuzzing */
};

/* Some arguments for parsing json text */
//s32    argument_num = 0;                 /* Total number of arguments        */
//s32    pre_argument_num = 0;             /* Arg-num of previous sort file    */
//cJSON *monitor_json = NULL;              /* Global monitor of the json       */
//struct argument_entry *args_info = NULL; /* Global argument_entry array      */
//u8 *queue_interest_fn = NULL;            /* Temp filename                    */
//s32 current_arg_id = 0;                  /* Current arg_id while fuzzing     */

/* Type of arguments */
enum {
  /* 00 */ FIXED_DATA,
  /* 01 */ UNFIXED_DATA,
  /* 02 */ STRUCT_DATA,
};

/* Real type of arguments */
enum {
  /* 00 */ UNSIGNED_CHAR,  /* 8  bits */
  /* 01 */ SIGNED_CHAR,    /* 8  bits */
  /* 02 */ UNSIGNED_SHORT, /* 16 bits */
  /* 03 */ SIGNED_SHORT,   /* 16 bits */
  /* 04 */ UNSIGNED_INT,   /* 32 bits */
  /* 05 */ SIGNED_INT,     /* 32 bits */
  /* 06 */ UNSIGNED_LL,    /* 64 bits */
  /* 07 */ SIGNED_LL,      /* 64 bits */
  /* 08 */ FLOAT,          /* 32 bits */
  /* 09 */ DOUBLE,         /* 64 bits */
  /* 10 */ STRING,         /* unfixed */
};

static char *real_type_name[10] = {
    "unsigned char", "signed char", "unsigned short",     "signed short",
    "unsigned int",  "signed int",  "unsigned long long", "signed long long",
    "float",         "double"
};
/*
    static char *format_strs[10] = {"%u", "%d",   "%u",   "%d",   "%u",
                         "%d", "%llu", "%lld", "%.7f", "%.15lf"};
*/
static u8 typename_to_size[10] = {8, 8, 16, 16, 32, 32, 64, 64, 32, 64};

/* Read json text from file */
u8 *read_json_file(u8 *fname);

/* Check the format of json text */
u8 check_format(const char *monitor);

/* A helper function to parse structure data. */
s32 parse_struct(cJSON *args, struct api_entry *api, s32 cnt, s32 g_skip);

/* Parse json text and record some information about arguments */
void parse_json(const char *monitor, struct api_entry *api);

/* Construct input for harness. The input will be written in .cur_input by
 * default */
struct input_data *construct_input(struct api_entry *api);

/* Clean some memory created by api_entry */
void clean_entries(struct api_entry *api);

/* Parse domain string */
void parse_domain(const char *str, _domain *domain, u8 type, s32 len);

u64 fetch_right_val(struct api_entry* api, u8* t, s32 i);