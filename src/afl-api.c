#include "afl-api.h"

#define VALID_INPUT_PROB 90

u8 *read_json_file(u8 *fname) {
  
  struct stat st;

  if (lstat(fname, &st) || access(fname, R_OK))
    PFATAL("Unable to access '%s'", fname);

  u32 len = st.st_size;

  u8 *json_text = malloc(len + 1);

  s32 fd = open(fname, O_RDONLY);

  if (fd < 0) PFATAL("Unable to open '%s'", fname);

  read(fd, json_text, len);

  close(fd);

  json_text[len] = 0;

  return json_text;
}

/* Generate a random number (from 0 to limit - 1). This may
   have slight bias. */
static inline u32 UR(u32 limit) {
  static rand_cnt = 0;
  s32 fd = open("/dev/urandom", O_RDONLY);
  if (fd < 0) PFATAL("Unable to open /dev/urandom");

  if (unlikely(!rand_cnt--)) {
    u32 seed[2];

    ck_read(fd, &seed, sizeof(seed), "/dev/urandom");

    srandom(seed[0]);
    rand_cnt = (RESEED_RNG / 2) + (seed[1] % RESEED_RNG);
  }
  close(fd);
  return random() % limit;
}

u8 check_format(const char *monitor) {
  cJSON *num = NULL;
  cJSON *argument = NULL;
  cJSON *arguments = NULL;
  cJSON *monitor_json = cJSON_Parse(monitor);
  if (!monitor_json) {
    const char *error_ptr = cJSON_GetErrorPtr();
    if (error_ptr)
      PFATAL("There is something wrong with json file. Error before: %s",
             error_ptr);
    PFATAL("There is something wrong happened while parsing json");
  }
  num = cJSON_GetObjectItemCaseSensitive(monitor_json, "num");
  if (!cJSON_IsNumber(num) || !num->valueint) {
    PFATAL("Cannot resolve 'num' in json file");
  }
  int arg_num = num->valueint;
  arguments = cJSON_GetObjectItemCaseSensitive(monitor_json, "arguments");
  int cnt = 0;
  cJSON_ArrayForEach(argument, arguments) {
    cnt++;
    cJSON *type = cJSON_GetObjectItemCaseSensitive(argument, "type");
    cJSON *value = cJSON_GetObjectItemCaseSensitive(argument, "value");
    cJSON *context = cJSON_GetObjectItemCaseSensitive(argument, "context");
    cJSON *skip = cJSON_GetObjectItemCaseSensitive(argument, "skip");
    cJSON *domain = cJSON_GetObjectItemCaseSensitive(argument, "domain");
    if (skip && !cJSON_IsBool(skip)) {
      PFATAL("Cannot resolve %dth argument of 'skip' in json file", cnt);
    }
    char *typeValue = type->valuestring;
    if (!cJSON_IsString(type) || typeValue == NULL ||
        (strcmp(typeValue, "unfixed") &&
         strstr(typeValue, "fixed_") != typeValue) &&
            strcmp(typeValue, "struct")) {
      PFATAL("Cannot resolve %dth argument of 'type' in json file", cnt);
    }
    if (!cJSON_IsBool(context)) {
      PFATAL("Cannot resolve %dth argument of 'context' in json file", cnt);
    }

    if (domain && !cJSON_IsString(domain))
      FATAL("%dth argument 'domain' must be parsing string", cnt);

    if (!strcmp(typeValue, "struct")) {
      /* do something.... */

    } else if (strstr(typeValue, "fixed_") == typeValue &&
               strlen(typeValue) < 9) {
      char *t = strchr(typeValue, '_') + 1;
      int   type_len = 0;
      while (*t != '\0') {
        type_len = type_len * 10 + *t - '0';
        t++;
      }
      if (type_len < 8 || (type_len & (type_len - 1)) != 0) {
        PFATAL("Cannot resolve %dth argument of 'type=fixed_%d' in json file",
               cnt, type_len);
      }
      char *type_value = value->valuestring;
      /*
      while (*type_value != '\0') {
        if (isdigit(*type_value) == 0) {
          PFATAL("Cannot resolve %dth argument of 'value=%s' in json file", cnt,
                 value->valuestring);
        }

        type_value++;
      }
      */
    } else if (!strcmp(typeValue, "unfixed")) {
      if (!cJSON_IsString(value)) {
        PFATAL("Cannot resolve %dth argument of 'value' in json file", cnt);
      }
      if (strlen(typeValue) == 0) {
        PFATAL("The value of unfixed data should be not empty string");
      }
    } else {
      PFATAL("Cannot resolve %dth argument of 'type=%s' in json file", cnt,
             typeValue);
    }
  }
  if (cnt != arg_num) {
    PFATAL("The num=%d is not equal to arguments's length=%d", arg_num, cnt);
  }

  cJSON_Delete(monitor_json);
  return 0;
}

s32 parse_struct(cJSON *args, struct api_entry *api, s32 cnt, s32 g_skip) {
  s32    i = 0;
  cJSON *arg = NULL;

  /* some duplication with the function parseJSON(), whatever~ */
  cJSON_ArrayForEach(arg, args) {
    if (i != 0) {
      api->args_info = (struct argument_entry *)ck_realloc(
          api->args_info,
          sizeof(struct argument_entry) * (++api->argument_num));
    }
    cJSON *value = cJSON_GetObjectItemCaseSensitive(arg, "value");
    char * typeValue =
        cJSON_GetObjectItemCaseSensitive(arg, "type")->valuestring;
    cJSON *skip = cJSON_GetObjectItemCaseSensitive(arg, "skip");
    const s8 *name = cJSON_GetObjectItemCaseSensitive(arg, "name")->valuestring;
    cJSON *domain = cJSON_GetObjectItemCaseSensitive(arg, "domain");
	u8 *      real_type_n =
        cJSON_GetObjectItemCaseSensitive(arg, "real_type")->valuestring;
    if (!skip) {
      /* If the key skip is not defined, define it with the value of outer
       * structure. */
      cJSON *skipped = (g_skip == 0) ? cJSON_CreateFalse() : cJSON_CreateTrue();
      cJSON_AddItemToObject(arg, "skip", skipped);
    }
    skip = cJSON_GetObjectItemCaseSensitive(arg, "skip");

    if (name) {
      strncpy(api->args_info[cnt].name, name,
              sizeof(api->args_info[cnt].name) - 1);
      api->args_info[cnt].name[sizeof(api->args_info[cnt].name) - 1] = 0;
    } else {
      api->args_info[cnt].name[0] = 0;
    }

    if (!strcmp(typeValue, "struct")) {
      cnt = parse_struct(value, api, cnt, skip->type == 1 ? 0 : 1);
    } else if (strstr(typeValue, "fixed_") == typeValue &&
               strlen(typeValue) < 9) {
	  if (!real_type_n) FATAL("Missing \"real_type\"?");
      char *t = typeValue + 6;

      u8 type_len = 0;
      while (*t != '\0') {
        type_len = type_len * 10 + *t - '0';
        t++;
      }

      u8 offset;
      for (offset = UNSIGNED_CHAR; offset <= DOUBLE; offset++) {
        if (!strncmp(real_type_name[offset], real_type_n,
                     MIN(strlen(real_type_name[offset]), strlen(real_type_n))))
          break;
      }
      if (offset == DOUBLE + 1) FATAL("Error \"real_type\" value?");
      if (typename_to_size[offset] != type_len)
        FATAL("Seriously? Real_type is not match with fixed_X?");

      api->args_info[cnt].real_type = offset;
      api->args_info[cnt].type = FIXED_DATA;
      api->args_info[cnt].len = type_len / 8;
      api->args_info[cnt].value = value;

      if (domain) {
        parse_domain(domain->valuestring, &api->args_info[cnt].domain,
                     api->args_info[cnt].type, api->args_info[cnt].len);
      }

    } else {
      api->args_info[cnt].real_type = STRING;
      api->args_info[cnt].type = UNFIXED_DATA;
      char *str_t = value->valuestring;

      api->args_info[cnt].len = strlen(str_t);
      api->args_info[cnt].value = value;

      if (domain) {
        parse_domain(domain->valuestring, &api->args_info[cnt].domain,
                     api->args_info[cnt].type, api->args_info[cnt].len);
      }
    }
    cnt++;
    i++;
  }
  return cnt - 1;
}

void parse_json(const char *monitor, struct api_entry* api) {
  /* First, check if json file's format is legal. */
  if (!check_format(monitor)) {
    /* Save a copy of cjson structure */
    api->monitor_json = cJSON_Parse(monitor);
    cJSON *args =
        cJSON_GetObjectItemCaseSensitive(api->monitor_json, "arguments");
    cJSON *arg = NULL;

    api->argument_num =
        cJSON_GetObjectItemCaseSensitive(api->monitor_json, "num")->valueint;
    api->args_info = (struct argument_entry *)ck_alloc(
        sizeof(struct argument_entry) * api->argument_num);
    api->cur_arg_id = 0;

    s32 cnt = 0;
    cJSON_ArrayForEach(arg, args) {
      u8 *typeValue =
          cJSON_GetObjectItemCaseSensitive(arg, "type")->valuestring;
      cJSON *skip = cJSON_GetObjectItemCaseSensitive(arg, "skip");
      cJSON *value = cJSON_GetObjectItemCaseSensitive(arg, "value");
      const s8 *name =
          cJSON_GetObjectItemCaseSensitive(arg, "name")->valuestring;
      cJSON *domain = cJSON_GetObjectItemCaseSensitive(arg, "domain");
	  u8 *real_type_n =
          cJSON_GetObjectItemCaseSensitive(arg, "real_type")->valuestring;
      if (!skip) {
        /* If the key 'skip' is not defined, just define it with default value
         * false */
        cJSON *skipped = cJSON_CreateFalse();
        cJSON_AddItemToObject(arg, "skip", skipped);
      }

      skip = cJSON_GetObjectItemCaseSensitive(arg, "skip");
      api->args_info[cnt].skip_fuzz = skip->type == 1 ? 0 : 1;
      api->args_info[cnt].len = 0;

      if (name) {
        strncpy(api->args_info[cnt].name, name,
                sizeof(api->args_info[cnt].name) - 1);
        api->args_info[cnt].name[sizeof(api->args_info[cnt].name) - 1] = 0;
      } else {
        api->args_info[cnt].name[0] = 0;
      }

      if (!strcmp(typeValue, "struct")) {
        cnt = parse_struct(value, api, cnt, api->args_info[cnt].skip_fuzz);
      } else if (strstr(typeValue, "fixed_") == typeValue &&
                 strlen(typeValue) < 9) {
		if (!real_type_n) FATAL("Missing \"real_type\"?");
        char *t = typeValue + 6;

        u8 type_len = 0;
        while (*t != '\0') {
          type_len = type_len * 10 + *t - '0';
          t++;
        }
        u8 offset;
        for (offset = UNSIGNED_CHAR; offset <= DOUBLE; offset++) {
          if (!strncmp(real_type_name[offset], real_type_n,
                  MIN(strlen(real_type_name[offset]), strlen(real_type_n))))
            break;
        }
        if (offset == DOUBLE + 1) FATAL("Error \"real_type\" value?");
        if (typename_to_size[offset] != type_len)
          FATAL("Seriously? Real_type is not match with fixed_X?");

        api->args_info[cnt].real_type = offset;
        api->args_info[cnt].type = FIXED_DATA;
        api->args_info[cnt].len = type_len / 8;
        api->args_info[cnt].value = value;

        if (domain) {
          parse_domain(domain->valuestring, &api->args_info[cnt].domain,
                       api->args_info[cnt].type, api->args_info[cnt].len);
        }

      } else {
        api->args_info[cnt].real_type = STRING;
        api->args_info[cnt].type = UNFIXED_DATA;
        char *str_t = value->valuestring;

        api->args_info[cnt].len = strlen(str_t);
        api->args_info[cnt].value = value;

        if (domain) {
          parse_domain(domain->valuestring, &api->args_info[cnt].domain,
                       api->args_info[cnt].type, api->args_info[cnt].len);
        }

      }
      cnt++;
    }
  }
}

struct input_data *construct_input(struct api_entry *api) {
  /* After use this function, please free the return value in time in case heap
   * overflow */
  if (!api->monitor_json) return NULL;
  /* Pre-read the length of string, automatically malloc enough space */
  u32 max_len = 0;

  u8 **orig_value = ck_alloc(sizeof(u8 *) * api->argument_num); /* need to free ~! */

  for (s32 i = 0; i < api->argument_num; i++) {
    if (api->args_info[i].type == FIXED_DATA ||
         api->args_info[i].type == (FIXED_DATA | STRUCT_DATA)) {
      orig_value[i] = api->args_info[i].value->valuestring;
      u8 *t = orig_value[i];
      memset(api->args_info[i].real_val_str, 0, 50);
      u8 *tmp = api->args_info[i].real_val_str;
      u64 num = 0;
      while (*t != 0) {
        num = num * 10 + *t - '0';
        t++;
      }

      if (api->args_info[i].real_type == UNSIGNED_CHAR)
        sprintf(tmp, "%d", (u8)num);
      else if (api->args_info[i].real_type == SIGNED_CHAR)
        sprintf(tmp, "%d", (s8)num);
      else if (api->args_info[i].real_type == UNSIGNED_SHORT)
        sprintf(tmp, "%d", (u16)num);
      else if (api->args_info[i].real_type == SIGNED_SHORT)
        sprintf(tmp, "%d", (s16)num);
      else if (api->args_info[i].real_type == UNSIGNED_INT)
        sprintf(tmp, "%d", (u32)num);
      else if (api->args_info[i].real_type == SIGNED_INT)
        sprintf(tmp, "%d", (s32)num);
      else if (api->args_info[i].real_type == UNSIGNED_LL)
        sprintf(tmp, "%lld", (u64)num);
      else if (api->args_info[i].real_type == SIGNED_LL)
        sprintf(tmp, "%lld", (s64)num);
      else if (api->args_info[i].real_type == FLOAT) {
        s32   a = (s32)num;
        float b;
        memcpy(&b, &a, sizeof(float));
        /* IEEE-754 */
        if (api->cur_arg_id == i) {
          s32 *p = (s32 *)&b;
          s32  mask = UR(148) - 20; /* -20 ~ 127 : exp */
          mask = mask & 0x000000FF;
          mask = mask << 23;
          *p = *p & 0x807fffff | mask;
        }
        sprintf(tmp, "%.7f", b);
      }
      else if (api->args_info[i].real_type == DOUBLE) {
        s64   a = (s64)num;
        double b;
        memcpy(&b, &a, sizeof(double));
        /* IEEE-754 */
        if (api->cur_arg_id == i) {
          s64 *p = (s64 *)&b;
          s64  mask = UR(1064) - 40; /* -40 ~ 1023 : exp */
          mask = mask & 0x00000000000007FF;
          mask = mask << 52;
          *p = *p & 0x800fffffffffffff | mask;
        }
        sprintf(tmp, "%.15lf", b);
      } else {
        FATAL("?????");
      }
        

      api->args_info[i].value->valuestring = tmp;
    }

    max_len += strlen(api->args_info[i].value->valuestring);
    max_len += strlen(api->args_info[i].name);
    /* type=[0-10] */
    if (api->args_info[i].type == FIXED_DATA ||
        api->args_info[i].type == (FIXED_DATA | STRUCT_DATA)) {
      max_len += 6; /* type=[0-9] */
    } else {
      max_len += 7; /* type=10 */
    }
  }

  max_len = max_len + api->argument_num * 4; /* the length of ",=\r\n" */

  u8 *res_data = ck_alloc(max_len + 1);

  struct input_data *res = ck_alloc(sizeof(struct input_data));
  res->data = res_data;
  res->len = max_len;

  u32 idx = 0;
  for (s32 i = 0; i < api->argument_num; i++) {
    u8  type_n[] = "type=";
    for (s32 j = 0; j < strlen(type_n); j++) {
      res_data[idx++] = type_n[j];
    }
    if (api->args_info[i].real_type <= 9)
      res_data[idx++] = api->args_info[i].real_type + '0';
    else {
      res_data[idx++] = '1';
      res_data[idx++] = '0';
    }
    res_data[idx++] = ',';
    u8 *name = api->args_info[i].name;
    for (s32 j = 0; j < strlen(name); j++) {
      res_data[idx++] = name[j];
    }
    res_data[idx++] = '=';

    u8 *tmp = api->args_info[i].value->valuestring;
    for (s32 j = 0; j < strlen(tmp); j++) {
      res_data[idx++] = tmp[j];
    }
    if (api->args_info[i].type == FIXED_DATA ||
        api->args_info[i].type == (FIXED_DATA | STRUCT_DATA)) {
      api->args_info[i].value->valuestring = orig_value[i];
    }
    res_data[idx++] = '\r';
    res_data[idx++] = '\n';
  }

  res_data[max_len] = 0;
  ck_free(orig_value);
  return res;
}

void clean_entries(struct api_entry *api) {
  /* do some cleaning job~ */

  for (int i = 0; i < api->argument_num; i++) {
    ck_free(api->args_info[i].domain.discrete);
    ck_free(api->args_info[i].domain.range);
  }
  ck_free(api->args_info);

  cJSON_Delete(api->monitor_json);

  ck_free(api);
}

static void add_range(_domain *domain, u64 left, u64 right) {
  domain->range =
      ck_realloc(domain->range, sizeof(_range) * (++domain->r_size));

  domain->range[domain->r_size - 1].max = right;
  domain->range[domain->r_size - 1].min = left;

  domain->total_n += right - left + 1;
}

static void add_single_v(_domain *domain, u64 value) {

  if (domain->discrete[0] + 1 == domain->d_size) {
    /* exponential increase, avoid 'realloc' frequently */

    domain->d_size <<= 1;
    domain->discrete =
        ck_realloc(domain->discrete, sizeof(u64) * domain->d_size);
  }

  domain->discrete[++domain->discrete[0]] = value;

  domain->total_n++;
}

void parse_domain(const char *str, _domain *domain, u8 type, s32 len) {

    domain->r_size = 0;

    domain->d_size = 16;
    domain->discrete = ck_alloc(sizeof(u64) * domain->d_size);
    domain->discrete[0] = 0;

    // parse string like "1-3,5,7,8-10"

    u8 charset[] = " ,-0123456789";
    u8 set_l = strlen(charset);
    u8 j;
    s32 i;
    s32 str_l = strlen(str);
    u8  is_v = 0, is_sep = 0, is_range = 0;
    u64 _min = 0, _max = 0;

    for (i = 0; i < str_l; i++) {

        // check char set
        for (j = 0; j < set_l; j++) {
            if (str[i] == charset[j]) break;
        }
        if (j == set_l) FATAL("Invalid character '%c' at %d", str[i], i);

        if (str[i] == ' ') continue;

        if (str[i] == ',') { 
            
            if (!is_v) FATAL("Invalid character '%c' at %d", str[i], i);

            is_v = 0;

            if (is_range) {

                is_range = 0;

                if (_min > _max)
                  FATAL("Oops! min > max? ");

                if (_min == _max) {

                    add_single_v(domain, _min);

                } else {
                    
                    add_range(domain, _min, _max);
                }

            } else {
                
                add_single_v(domain, _min);

            }

            is_sep = 1;
            _min = _max = 0;
        }

        if (str[i] >= '0' && str[i] <= '9') { 
            is_sep = 0;
            is_v = 1;

            if (!is_range) {

                _min = _min * 10 + str[i] - '0';

            } else {

                _max = _max * 10 + str[i] - '0';

            }


        }

        if (str[i] == '-') { 
            if (is_range) FATAL("Double '-' at %d?", i);
            is_range = 1;
        }

        if (i == str_l - 1) {

            if (is_sep) 
                FATAL("Unknown domain string tail!");

            if (is_range) {

              if (_min > _max) FATAL("Oops! min > max? ");

              if (_min == _max) {
                add_single_v(domain, _min);

              } else {
                add_range(domain, _min, _max);
              }

            } else {
              add_single_v(domain, _min);
            }

        }

    }

}

/* Generate a random number (from 0 to limit - 1). This may
   have slight bias. */

static inline u64 UR64(u64 limit) {
  static rand_cnt = 0;

  s32 fd = open("/dev/urandom", O_RDONLY);
  if (fd < 0) PFATAL("Unable to open /dev/urandom");

  if (unlikely(!rand_cnt--)) {
    u64 seed[2];

    ck_read(fd, &seed, sizeof(seed), "/dev/urandom");

    srandom(seed[0]);
    rand_cnt = (RESEED_RNG / 2) + (seed[1] % RESEED_RNG);
  }

  close(fd);

  u64 ret = ((random() << 32) | random()) % limit;

  return ret % limit;
}


u64 gen_rand(_domain *domain, s32 len) {
  /* domain: 1. pure range like a-b; 2. a-b and {c, d, e, ...} */

  if (!domain) FATAL("The domain pointer is NULL?");

  if (!domain->total_n) FATAL("Oops! The domain is empty?");

  if (UR64(100) > VALID_INPUT_PROB) {
    /* 10% (default) generate random input */

    u64 ret = UR64(-1);
    ret = ret & ((u64)-1 >> (64 - len * 8));

    return ret;
  }

  u8 c = (domain->discrete[0] ? 1 : 0) + (domain->r_size ? 1 : 0);

  if (c == 2) {
    /* Little bias */

    double prob = domain->total_n;
    prob = (domain->discrete[0] / prob) * 100;

    if (UR64(100) < prob) {
      /* Choose discrete values randomly */

      return domain->discrete[UR64(domain->discrete[0]) + 1];

    } else {
      /* Choose range values randomly */

      s32 i = UR64(domain->r_size);

      return domain->range[i].min +
             UR64(domain->range[i].max - domain->range[i].min + 1);
    }

  } else {
    if (domain->discrete[0]) {
      return domain->discrete[UR64(domain->discrete[0]) + 1];

    } else {
      s32 i = UR64(domain->r_size);

      return domain->range[i].min +
             UR64(domain->range[i].max - domain->range[i].min + 1);
    }
  }
}


u64 fetch_right_val(struct api_entry *api, u8 *t, s32 i) {
  u64 res = 0;

  if (api->args_info[i].real_type == UNSIGNED_CHAR) {
    u8 temp_v = 0;
    sscanf(t, "%hhu", &temp_v);
    res = temp_v;
  } else if (api->args_info[i].real_type == SIGNED_CHAR) {
    s8 temp_v = 0;
    sscanf(t, "%hhd", &temp_v);
    res = temp_v;
  } else if (api->args_info[i].real_type == UNSIGNED_SHORT) {
    u16 temp_v = 0;
    sscanf(t, "%hu", &temp_v);
    res = temp_v;
  } else if (api->args_info[i].real_type == SIGNED_SHORT) {
    s16 temp_v = 0;
    sscanf(t, "%hd", &temp_v);
    res = temp_v;
  } else if (api->args_info[i].real_type == UNSIGNED_INT) {
    u32 temp_v = 0;
    sscanf(t, "%u", &temp_v);
    res = temp_v;
  } else if (api->args_info[i].real_type == SIGNED_INT) {
    s32 temp_v = 0;
    sscanf(t, "%d", &temp_v);
    res = temp_v;
  } else if (api->args_info[i].real_type == UNSIGNED_LL) {
    u64 temp_v = 0;
    sscanf(t, "%llu", &temp_v);
    res = temp_v;
  } else if (api->args_info[i].real_type == SIGNED_LL) {
    s64 temp_v = 0;
    sscanf(t, "%lld", &temp_v);
    res = temp_v;
  } else if (api->args_info[i].real_type == FLOAT) {
    float temp_v = 0;
    sscanf(t, "%f", &temp_v);
    memcpy(&res, &temp_v, sizeof(float));
  } else if (api->args_info[i].real_type == DOUBLE) {
    double temp_v = 0;
    sscanf(t, "%lf", &temp_v);
    memcpy(&res, &temp_v, sizeof(double));
  }

  return res;
}