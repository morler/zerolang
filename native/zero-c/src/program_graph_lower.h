#ifndef ZERO_C_PROGRAM_GRAPH_LOWER_H
#define ZERO_C_PROGRAM_GRAPH_LOWER_H

#include "program_graph.h"

bool z_program_graph_lower_to_program(const ZProgramGraph *graph, Program *out, ZDiag *diag);

#endif
