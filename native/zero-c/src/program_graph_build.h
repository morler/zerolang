#ifndef ZERO_C_PROGRAM_GRAPH_BUILD_H
#define ZERO_C_PROGRAM_GRAPH_BUILD_H

#include "program_graph.h"

typedef struct {
  const char *artifact;
  char *graph_hash;
  char *module_identity;
  const char *lowering;
} ZProgramGraphBuildSource;

bool z_program_graph_command_kind_is_known(const char *kind);
bool z_program_graph_build_source_present(const ZProgramGraphBuildSource *source);
bool z_program_graph_prepare_build_input(const char *artifact_path, const ZTargetInfo *target, Program *program, SourceInput *input, ZProgramGraphBuildSource *source, ZDiag *diag);

#endif
