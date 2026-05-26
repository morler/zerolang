#include "program_graph_compare.h"

#include <stdio.h>
#include <string.h>

static bool compare_text_eq(const char *left, const char *right) {
  return strcmp(left ? left : "", right ? right : "") == 0;
}

static bool compare_fail(
  ZProgramGraphCompare *out,
  const char *code,
  const char *message,
  const char *field,
  size_t left_index,
  size_t right_index,
  size_t left_count,
  size_t right_count
) {
  if (out) {
    out->ok = false;
    snprintf(out->code, sizeof(out->code), "%s", code ? code : "GRC000");
    snprintf(out->message, sizeof(out->message), "%s", message ? message : "program graphs differ");
    snprintf(out->field, sizeof(out->field), "%s", field ? field : "");
    out->left_index = left_index;
    out->right_index = right_index;
    out->left_count = left_count;
    out->right_count = right_count;
  }
  return false;
}

static size_t compare_missing_index(void) {
  return (size_t)-1;
}

static bool compare_edge_kind(const ZProgramGraphEdge *edge, const char *kind) {
  return edge && compare_text_eq(edge->kind, kind);
}

static bool compare_is_declared_type_ref(const ZProgramGraph *graph, size_t index) {
  if (!graph || index >= graph->node_len) return false;
  const ZProgramGraphNode *node = &graph->nodes[index];
  if (node->kind != Z_PROGRAM_GRAPH_NODE_TYPE_REF) return false;
  for (size_t i = 0; i < graph->edge_len; i++) {
    const ZProgramGraphEdge *edge = &graph->edges[i];
    if (edge->target == Z_PROGRAM_GRAPH_EDGE_TARGET_NODE &&
        compare_edge_kind(edge, "declaredType") &&
        compare_text_eq(edge->to, node->id)) {
      return true;
    }
  }
  return false;
}

static bool compare_has_module_named(const ZProgramGraph *graph, const char *name) {
  for (size_t i = 0; graph && name && i < graph->node_len; i++) {
    const ZProgramGraphNode *node = &graph->nodes[i];
    if (node->kind == Z_PROGRAM_GRAPH_NODE_MODULE && compare_text_eq(node->name, name)) return true;
  }
  return false;
}

static bool compare_is_local_import(const ZProgramGraph *graph, size_t index) {
  if (!graph || index >= graph->node_len) return false;
  const ZProgramGraphNode *node = &graph->nodes[index];
  if (node->name && strncmp(node->name, "std.", strlen("std.")) == 0) return false;
  return node->kind == Z_PROGRAM_GRAPH_NODE_IMPORT && compare_has_module_named(graph, node->name);
}

static bool compare_skip_node(const ZProgramGraph *graph, size_t index) {
  return compare_is_declared_type_ref(graph, index) || compare_is_local_import(graph, index);
}

static bool compare_skip_edge(const ZProgramGraph *graph, const ZProgramGraphEdge *edge) {
  if (!graph || !edge) return false;
  size_t source = compare_missing_index();
  for (size_t i = 0; i < graph->node_len; i++) {
    if (compare_text_eq(graph->nodes[i].id, edge->from)) {
      source = i;
      break;
    }
  }
  if (source != compare_missing_index() && compare_skip_node(graph, source)) return true;
  if (edge->target != Z_PROGRAM_GRAPH_EDGE_TARGET_NODE) return false;
  size_t target = compare_missing_index();
  for (size_t i = 0; i < graph->node_len; i++) {
    if (compare_text_eq(graph->nodes[i].id, edge->to)) {
      target = i;
      break;
    }
  }
  return target != compare_missing_index() && compare_skip_node(graph, target);
}

static size_t compare_semantic_node_count(const ZProgramGraph *graph) {
  size_t count = 0;
  for (size_t i = 0; graph && i < graph->node_len; i++) {
    if (!compare_skip_node(graph, i)) count++;
  }
  return count;
}

static size_t compare_semantic_edge_count(const ZProgramGraph *graph) {
  size_t count = 0;
  for (size_t i = 0; graph && i < graph->edge_len; i++) {
    if (!compare_skip_edge(graph, &graph->edges[i])) count++;
  }
  return count;
}

static size_t compare_nth_semantic_node(const ZProgramGraph *graph, size_t semantic_index) {
  size_t seen = 0;
  for (size_t i = 0; graph && i < graph->node_len; i++) {
    if (compare_skip_node(graph, i)) continue;
    if (seen == semantic_index) return i;
    seen++;
  }
  return compare_missing_index();
}

static size_t compare_nth_semantic_edge(const ZProgramGraph *graph, size_t semantic_index) {
  size_t seen = 0;
  for (size_t i = 0; graph && i < graph->edge_len; i++) {
    if (compare_skip_edge(graph, &graph->edges[i])) continue;
    if (seen == semantic_index) return i;
    seen++;
  }
  return compare_missing_index();
}

static size_t compare_semantic_index_for_raw_node(const ZProgramGraph *graph, size_t raw_index) {
  if (!graph || raw_index >= graph->node_len || compare_skip_node(graph, raw_index)) return compare_missing_index();
  size_t semantic_index = 0;
  for (size_t i = 0; i < raw_index; i++) {
    if (!compare_skip_node(graph, i)) semantic_index++;
  }
  return semantic_index;
}

static size_t compare_node_index_by_id(const ZProgramGraph *graph, const char *id) {
  for (size_t i = 0; graph && id && i < graph->node_len; i++) {
    if (compare_text_eq(graph->nodes[i].id, id)) return i;
  }
  return compare_missing_index();
}

static size_t compare_node_index_by_symbol_id(const ZProgramGraph *graph, const char *id) {
  for (size_t i = 0; graph && id && i < graph->node_len; i++) {
    if (compare_text_eq(graph->nodes[i].symbol_id, id)) return i;
  }
  return compare_missing_index();
}

static size_t compare_node_index_by_type_id(const ZProgramGraph *graph, const char *id) {
  for (size_t i = 0; graph && id && i < graph->node_len; i++) {
    if (compare_text_eq(graph->nodes[i].type_id, id)) return i;
  }
  return compare_missing_index();
}

static size_t compare_node_index_by_effect_id(const ZProgramGraph *graph, const char *id) {
  for (size_t i = 0; graph && id && i < graph->node_len; i++) {
    if (compare_text_eq(graph->nodes[i].effect_id, id)) return i;
  }
  return compare_missing_index();
}

static size_t compare_target_index(const ZProgramGraph *graph, const ZProgramGraphEdge *edge) {
  if (!edge) return compare_missing_index();
  size_t raw_index = compare_missing_index();
  switch (edge->target) {
    case Z_PROGRAM_GRAPH_EDGE_TARGET_NODE:
      raw_index = compare_node_index_by_id(graph, edge->to);
      break;
    case Z_PROGRAM_GRAPH_EDGE_TARGET_SYMBOL:
      raw_index = compare_node_index_by_symbol_id(graph, edge->to);
      break;
    case Z_PROGRAM_GRAPH_EDGE_TARGET_TYPE:
      raw_index = compare_node_index_by_type_id(graph, edge->to);
      break;
    case Z_PROGRAM_GRAPH_EDGE_TARGET_EFFECT:
      raw_index = compare_node_index_by_effect_id(graph, edge->to);
      break;
  }
  return compare_semantic_index_for_raw_node(graph, raw_index);
}

static bool compare_node_text_field(const char *field, const char *left, const char *right, size_t left_index, size_t right_index, ZProgramGraphCompare *out) {
  if (compare_text_eq(left, right)) return true;
  return compare_fail(out, "GRC004", "node semantic field differs", field, left_index, right_index, 0, 0);
}

static bool compare_node_bool_field(const char *field, bool left, bool right, size_t left_index, size_t right_index, ZProgramGraphCompare *out) {
  if (left == right) return true;
  return compare_fail(out, "GRC005", "node flag differs", field, left_index, right_index, 0, 0);
}

static bool compare_nodes(const ZProgramGraph *left, const ZProgramGraph *right, ZProgramGraphCompare *out) {
  size_t left_count = compare_semantic_node_count(left);
  size_t right_count = compare_semantic_node_count(right);
  if (left_count != right_count) {
    return compare_fail(out, "GRC002", "node count differs", "nodes", 0, 0, left_count, right_count);
  }
  for (size_t i = 0; i < left_count; i++) {
    size_t left_index = compare_nth_semantic_node(left, i);
    size_t right_index = compare_nth_semantic_node(right, i);
    const ZProgramGraphNode *left_node = &left->nodes[left_index];
    const ZProgramGraphNode *right_node = &right->nodes[right_index];
    if (left_node->kind != right_node->kind) return compare_fail(out, "GRC003", "node kind differs", "kind", left_index, right_index, 0, 0);
    if (!compare_node_text_field("name", left_node->name, right_node->name, left_index, right_index, out)) return false;
    if (!compare_node_text_field("type", left_node->type, right_node->type, left_index, right_index, out)) return false;
    if (!compare_node_text_field("value", left_node->value, right_node->value, left_index, right_index, out)) return false;
    if (!compare_node_bool_field("public", left_node->is_public, right_node->is_public, left_index, right_index, out)) return false;
    if (!compare_node_bool_field("mutable", left_node->is_mutable, right_node->is_mutable, left_index, right_index, out)) return false;
    if (!compare_node_bool_field("static", left_node->is_static, right_node->is_static, left_index, right_index, out)) return false;
    if (!compare_node_bool_field("fallible", left_node->fallible, right_node->fallible, left_index, right_index, out)) return false;
    if (!compare_node_bool_field("exportC", left_node->export_c, right_node->export_c, left_index, right_index, out)) return false;
  }
  return true;
}

static bool compare_edges(const ZProgramGraph *left, const ZProgramGraph *right, ZProgramGraphCompare *out) {
  size_t left_count = compare_semantic_edge_count(left);
  size_t right_count = compare_semantic_edge_count(right);
  if (left_count != right_count) {
    return compare_fail(out, "GRC006", "edge count differs", "edges", 0, 0, left_count, right_count);
  }
  for (size_t i = 0; i < left_count; i++) {
    size_t left_index = compare_nth_semantic_edge(left, i);
    size_t right_index = compare_nth_semantic_edge(right, i);
    const ZProgramGraphEdge *left_edge = &left->edges[left_index];
    const ZProgramGraphEdge *right_edge = &right->edges[right_index];
    size_t left_from = compare_semantic_index_for_raw_node(left, compare_node_index_by_id(left, left_edge->from));
    size_t right_from = compare_semantic_index_for_raw_node(right, compare_node_index_by_id(right, right_edge->from));
    size_t left_to = compare_target_index(left, left_edge);
    size_t right_to = compare_target_index(right, right_edge);
    if (left_from != right_from) return compare_fail(out, "GRC007", "edge source differs", "from", left_index, right_index, left_from, right_from);
    if (left_to != right_to) return compare_fail(out, "GRC008", "edge target differs", "to", left_index, right_index, left_to, right_to);
    if (!compare_text_eq(left_edge->kind, right_edge->kind)) return compare_fail(out, "GRC009", "edge kind differs", "kind", left_index, right_index, 0, 0);
    if (left_edge->target != right_edge->target) return compare_fail(out, "GRC010", "edge target domain differs", "target", left_index, right_index, 0, 0);
    if (left_edge->order != right_edge->order) return compare_fail(out, "GRC011", "edge order differs", "order", left_index, right_index, left_edge->order, right_edge->order);
  }
  return true;
}

bool z_program_graph_semantic_compare(const ZProgramGraph *left, const ZProgramGraph *right, ZProgramGraphCompare *out) {
  if (out) *out = (ZProgramGraphCompare){.ok = true};
  if (!left || !right) return compare_fail(out, "GRC001", "program graph is missing", "graph", 0, 0, left ? 1 : 0, right ? 1 : 0);
  if (out) {
    out->left_semantic_nodes = compare_semantic_node_count(left);
    out->right_semantic_nodes = compare_semantic_node_count(right);
    out->left_semantic_edges = compare_semantic_edge_count(left);
    out->right_semantic_edges = compare_semantic_edge_count(right);
  }
  if (left->schema_version != right->schema_version) {
    return compare_fail(out, "GRC012", "schema version differs", "schemaVersion", 0, 0, left->schema_version, right->schema_version);
  }
  if (!compare_text_eq(left->module_identity, right->module_identity)) {
    return compare_fail(out, "GRC013", "module identity differs", "moduleIdentity", 0, 0, 0, 0);
  }
  if (!compare_nodes(left, right, out)) return false;
  if (!compare_edges(left, right, out)) return false;
  if (out) out->ok = true;
  return true;
}
