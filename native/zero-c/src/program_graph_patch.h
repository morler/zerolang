#ifndef ZERO_C_PROGRAM_GRAPH_PATCH_H
#define ZERO_C_PROGRAM_GRAPH_PATCH_H

#include "program_graph.h"

typedef struct {
  size_t index;
  int line;
  char *op;
  char *node;
  char *field;
  char *expected;
  char *actual;
  char *value;
  bool has_expected;
  bool ok;
  char code[16];
  char message[160];
} ZProgramGraphPatchOpResult;

typedef struct {
  bool ok;
  char code[16];
  char message[160];
  char *expected;
  char *actual;
  char *expected_graph_hash;
  char *actual_graph_hash;
  ZProgramGraphPatchOpResult *operations;
  size_t operation_len;
  size_t operation_cap;
} ZProgramGraphPatchResult;

bool z_program_graph_apply_patch_file(const char *path, ZProgramGraph *graph, ZProgramGraphPatchResult *result, ZDiag *diag);
void z_program_graph_patch_result_free(ZProgramGraphPatchResult *result);

#endif
