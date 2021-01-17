#include <lisp/util/unit_test.h>

/* util */
void test_test(unit_context_t * ctx);
void test_xmalloc(unit_context_t * ctx);
void test_xstring(unit_context_t * ctx);
void test_assertion(unit_context_t * ctx);
void test_dl_list(unit_context_t * ctx);
void test_hash_table(unit_context_t * ctx);

/* core */
void test_cell(unit_context_t * ctx);
void test_vm(unit_context_t * ctx);
void test_cons(unit_context_t * ctx);
void test_array(unit_context_t * ctx);

int main(int argc, const char ** argv)
{
  unit_context_t * ctx = unit_create_context();

  test_assertion(ctx);
  test_xmalloc(ctx);
  test_xstring(ctx);
  test_test(ctx);
  test_dl_list(ctx);
  test_hash_table(ctx);

  test_cell(ctx);
  test_vm(ctx);
  test_cons(ctx);
  test_array(ctx);

  int parse_result = unit_parse_argv(ctx, argc, argv);
  if(parse_result == UNIT_ARGV_RUN) 
  {
    unit_run(stdout, ctx);
    unit_final_report(stdout, ctx);
  }
  else if(parse_result == UNIT_ARGV_ERROR) 
  {
    unit_print_help(stderr, ctx, argv[0]);
  }
  else if(parse_result == UNIT_ARGV_HELP) 
  {
    unit_print_help(stdout, ctx, argv[0]);
  }
  unit_free_context(ctx);
  if(parse_result == UNIT_ARGV_ERROR) 
  {
    return 8;
  }
  else 
  {
    return 0;
  }
}
