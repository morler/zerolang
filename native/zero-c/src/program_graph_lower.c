#include "program_graph_lower.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  const ZProgramGraph *graph;
  ZDiag *diag;
} GraphLower;

static void *lower_grow_items(void *items, size_t len, size_t *cap, size_t initial, size_t item_size) {
  if (len + 1 > *cap) {
    *cap = z_grow_capacity(*cap, len + 1, initial);
    return z_checked_reallocarray(items, *cap, item_size);
  }
  return items;
}

static void lower_push_function(FunctionVec *vec, Function item) {
  vec->items = lower_grow_items(vec->items, vec->len, &vec->cap, 8, sizeof(Function));
  vec->items[vec->len++] = item;
}

static void lower_push_stmt(StmtVec *vec, Stmt *item) {
  vec->items = lower_grow_items(vec->items, vec->len, &vec->cap, 8, sizeof(Stmt *));
  vec->items[vec->len++] = item;
}

static void lower_push_expr(ExprVec *vec, Expr *item) {
  vec->items = lower_grow_items(vec->items, vec->len, &vec->cap, 4, sizeof(Expr *));
  vec->items[vec->len++] = item;
}

static void lower_push_type_arg(TypeArgVec *vec, TypeArg item) {
  vec->items = lower_grow_items(vec->items, vec->len, &vec->cap, 4, sizeof(TypeArg));
  vec->items[vec->len++] = item;
}

static void lower_push_field(FieldInitVec *vec, FieldInit item) {
  vec->items = lower_grow_items(vec->items, vec->len, &vec->cap, 4, sizeof(FieldInit));
  vec->items[vec->len++] = item;
}

static void lower_push_param(ParamVec *vec, Param item) {
  vec->items = lower_grow_items(vec->items, vec->len, &vec->cap, 4, sizeof(Param));
  vec->items[vec->len++] = item;
}

static void lower_push_use(UseImportVec *vec, UseImport item) {
  vec->items = lower_grow_items(vec->items, vec->len, &vec->cap, 4, sizeof(UseImport));
  vec->items[vec->len++] = item;
}

static void lower_push_c_import(CImportVec *vec, CImport item) {
  vec->items = lower_grow_items(vec->items, vec->len, &vec->cap, 4, sizeof(CImport));
  vec->items[vec->len++] = item;
}

static void lower_push_const(ConstVec *vec, ConstDecl item) {
  vec->items = lower_grow_items(vec->items, vec->len, &vec->cap, 4, sizeof(ConstDecl));
  vec->items[vec->len++] = item;
}

static void lower_push_alias(TypeAliasVec *vec, TypeAlias item) {
  vec->items = lower_grow_items(vec->items, vec->len, &vec->cap, 4, sizeof(TypeAlias));
  vec->items[vec->len++] = item;
}

static void lower_push_shape(ShapeVec *vec, Shape item) {
  vec->items = lower_grow_items(vec->items, vec->len, &vec->cap, 4, sizeof(Shape));
  vec->items[vec->len++] = item;
}

static void lower_push_interface(InterfaceVec *vec, InterfaceDecl item) {
  vec->items = lower_grow_items(vec->items, vec->len, &vec->cap, 4, sizeof(InterfaceDecl));
  vec->items[vec->len++] = item;
}

static void lower_push_enum(EnumVec *vec, EnumDecl item) {
  vec->items = lower_grow_items(vec->items, vec->len, &vec->cap, 4, sizeof(EnumDecl));
  vec->items[vec->len++] = item;
}

static void lower_push_choice(ChoiceVec *vec, Choice item) {
  vec->items = lower_grow_items(vec->items, vec->len, &vec->cap, 4, sizeof(Choice));
  vec->items[vec->len++] = item;
}

static void lower_push_match_arm(MatchArmVec *vec, MatchArm item) {
  vec->items = lower_grow_items(vec->items, vec->len, &vec->cap, 4, sizeof(MatchArm));
  vec->items[vec->len++] = item;
}

static bool lower_text_eq(const char *left, const char *right) {
  return strcmp(left ? left : "", right ? right : "") == 0;
}

static bool lower_has_diag(const GraphLower *lower) {
  return lower && lower->diag && lower->diag->code != 0;
}

static bool lower_fail(GraphLower *lower, const ZProgramGraphNode *node, const char *message, const char *expected, const char *actual, const char *help) {
  if (!lower || !lower->diag || lower->diag->code != 0) return false;
  lower->diag->code = 2002;
  lower->diag->path = node && node->path && node->path[0] ? node->path : "<program-graph>";
  lower->diag->line = node && node->line > 0 ? node->line : 1;
  lower->diag->column = node && node->column > 0 ? node->column : 1;
  lower->diag->length = 1;
  snprintf(lower->diag->message, sizeof(lower->diag->message), "%s", message ? message : "cannot lower program graph");
  snprintf(lower->diag->expected, sizeof(lower->diag->expected), "%s", expected ? expected : "supported ProgramGraph shape");
  snprintf(lower->diag->actual, sizeof(lower->diag->actual), "%s", actual ? actual : (node ? z_program_graph_node_kind_name(node->kind) : "missing graph node"));
  if (help) snprintf(lower->diag->help, sizeof(lower->diag->help), "%s", help);
  return false;
}

static const ZProgramGraphNode *lower_find_node(const ZProgramGraph *graph, const char *id) {
  for (size_t i = 0; graph && id && i < graph->node_len; i++) {
    if (lower_text_eq(graph->nodes[i].id, id)) return &graph->nodes[i];
  }
  return NULL;
}

static const ZProgramGraphEdge *lower_ordered_edge(const ZProgramGraph *graph, const char *from, const char *kind, size_t order) {
  for (size_t i = 0; graph && from && kind && i < graph->edge_len; i++) {
    const ZProgramGraphEdge *edge = &graph->edges[i];
    if (edge->target == Z_PROGRAM_GRAPH_EDGE_TARGET_NODE &&
        edge->order == order &&
        lower_text_eq(edge->from, from) &&
        lower_text_eq(edge->kind, kind)) {
      return edge;
    }
  }
  return NULL;
}

static const ZProgramGraphNode *lower_ordered_node(const ZProgramGraph *graph, const char *from, const char *kind, size_t order) {
  const ZProgramGraphEdge *edge = lower_ordered_edge(graph, from, kind, order);
  return edge ? lower_find_node(graph, edge->to) : NULL;
}

static const ZProgramGraphEdge *lower_next_edge_by_order(const ZProgramGraph *graph, const char *from, const char *kind, bool have_last, size_t last_order) {
  const ZProgramGraphEdge *best = NULL;
  for (size_t i = 0; graph && from && kind && i < graph->edge_len; i++) {
    const ZProgramGraphEdge *edge = &graph->edges[i];
    if (edge->target != Z_PROGRAM_GRAPH_EDGE_TARGET_NODE ||
        !lower_text_eq(edge->from, from) ||
        !lower_text_eq(edge->kind, kind) ||
        (have_last && edge->order <= last_order)) {
      continue;
    }
    if (!best || edge->order < best->order) best = edge;
  }
  return best;
}

static bool lower_binary_operator(const char *text) {
  const char *ops[] = {"+", "-", "*", "/", "%", "&&", "||", "==", "!=", "<", "<=", ">", ">=", "+%", "+|", NULL};
  for (int i = 0; ops[i]; i++) {
    if (lower_text_eq(text, ops[i])) return true;
  }
  return false;
}

static bool lower_literal_is_raw(const char *value) {
  if (!value || !value[0]) return false;
  if (lower_text_eq(value, "true") || lower_text_eq(value, "false") || lower_text_eq(value, "null")) return true;
  const char *p = value;
  if (*p == '-') p++;
  bool digit = false;
  while (*p) {
    if (isdigit((unsigned char)*p)) {
      digit = true;
      p++;
      continue;
    }
    if (*p == '_' || *p == '.' || isalpha((unsigned char)*p)) {
      p++;
      continue;
    }
    if ((*p == '-' || *p == '+') && p > value && (*(p - 1) == 'e' || *(p - 1) == 'E')) {
      p++;
      continue;
    }
    return false;
  }
  return digit;
}

static bool lower_starts_with(const char *text, const char *prefix) {
  if (!text || !prefix) return false;
  size_t len = strlen(prefix);
  return strncmp(text, prefix, len) == 0;
}

static Expr *lower_new_expr(ExprKind kind, const ZProgramGraphNode *node) {
  Expr *expr = z_checked_malloc(sizeof(Expr));
  memset(expr, 0, sizeof(*expr));
  expr->kind = kind;
  expr->line = node && node->line > 0 ? node->line : 1;
  expr->column = node && node->column > 0 ? node->column : 1;
  return expr;
}

static Stmt *lower_new_stmt(StmtKind kind, const ZProgramGraphNode *node) {
  Stmt *stmt = z_checked_malloc(sizeof(Stmt));
  memset(stmt, 0, sizeof(*stmt));
  stmt->kind = kind;
  stmt->line = node && node->line > 0 ? node->line : 1;
  stmt->column = node && node->column > 0 ? node->column : 1;
  return stmt;
}

static Expr *lower_expr(GraphLower *lower, const ZProgramGraphNode *node);
static StmtVec lower_block(GraphLower *lower, const ZProgramGraphNode *block);

static Expr *lower_required_expr(GraphLower *lower, const ZProgramGraphNode *owner, const char *edge_kind, size_t order, const char *context) {
  const ZProgramGraphNode *node = lower_ordered_node(lower->graph, owner ? owner->id : NULL, edge_kind, order);
  if (!node) {
    lower_fail(lower, owner, "program graph is missing required expression edge", context ? context : "expression edge", edge_kind, NULL);
    return NULL;
  }
  return lower_expr(lower, node);
}

static Expr *lower_optional_expr(GraphLower *lower, const ZProgramGraphNode *owner, const char *edge_kind, size_t order) {
  const ZProgramGraphNode *node = lower_ordered_node(lower->graph, owner ? owner->id : NULL, edge_kind, order);
  return node ? lower_expr(lower, node) : NULL;
}

static void lower_type_args(GraphLower *lower, const ZProgramGraphNode *node, TypeArgVec *out) {
  bool have_last = false;
  size_t last_order = 0;
  for (;;) {
    const ZProgramGraphEdge *edge = lower_next_edge_by_order(lower->graph, node ? node->id : NULL, "typeArg", have_last, last_order);
    if (!edge) break;
    const ZProgramGraphNode *type = lower_find_node(lower->graph, edge->to);
    last_order = edge->order;
    have_last = true;
    if (!type) continue;
    lower_push_type_arg(out, (TypeArg){
      .type = z_strdup(type->type && type->type[0] ? type->type : ""),
      .line = type->line > 0 ? type->line : 1,
      .column = type->column > 0 ? type->column : 1,
    });
  }
}

static void lower_args(GraphLower *lower, const ZProgramGraphNode *node, ExprVec *out) {
  bool have_last = false;
  size_t last_order = 0;
  for (;;) {
    const ZProgramGraphEdge *edge = lower_next_edge_by_order(lower->graph, node ? node->id : NULL, "arg", have_last, last_order);
    if (!edge) break;
    const ZProgramGraphNode *arg = lower_find_node(lower->graph, edge->to);
    last_order = edge->order;
    have_last = true;
    if (!arg) continue;
    lower_push_expr(out, lower_expr(lower, arg));
    if (lower_has_diag(lower)) return;
  }
}

static void lower_shape_fields(GraphLower *lower, const ZProgramGraphNode *node, FieldInitVec *out) {
  bool have_last = false;
  size_t last_order = 0;
  for (;;) {
    const ZProgramGraphEdge *edge = lower_next_edge_by_order(lower->graph, node ? node->id : NULL, "field", have_last, last_order);
    if (!edge) break;
    const ZProgramGraphNode *field = lower_find_node(lower->graph, edge->to);
    last_order = edge->order;
    have_last = true;
    if (!field) continue;
    Expr *value = lower_required_expr(lower, field, "value", 0, "field initializer value");
    if (lower_has_diag(lower)) return;
    lower_push_field(out, (FieldInit){
      .name = z_strdup(field->name && field->name[0] ? field->name : ""),
      .value = value,
      .line = field->line > 0 ? field->line : 1,
      .column = field->column > 0 ? field->column : 1,
    });
  }
}

static Expr *lower_literal_expr(const ZProgramGraphNode *node) {
  if (lower_text_eq(node->type, "String")) {
    Expr *expr = lower_new_expr(EXPR_STRING, node);
    expr->text = z_strdup(node->value ? node->value : "");
    return expr;
  }
  if (lower_text_eq(node->type, "char")) {
    Expr *expr = lower_new_expr(EXPR_CHAR, node);
    expr->text = z_strdup(node->value ? node->value : "0");
    return expr;
  }
  if (lower_text_eq(node->value, "true") || lower_text_eq(node->value, "false")) {
    Expr *expr = lower_new_expr(EXPR_BOOL, node);
    expr->bool_value = lower_text_eq(node->value, "true");
    return expr;
  }
  if (lower_text_eq(node->value, "null")) return lower_new_expr(EXPR_NULL, node);
  if (!lower_literal_is_raw(node->value)) {
    Expr *expr = lower_new_expr(EXPR_STRING, node);
    expr->text = z_strdup(node->value ? node->value : "");
    return expr;
  }
  Expr *expr = lower_new_expr(EXPR_NUMBER, node);
  expr->text = z_strdup(node->value ? node->value : "0");
  return expr;
}

static Expr *lower_expr(GraphLower *lower, const ZProgramGraphNode *node) {
  if (!node) {
    lower_fail(lower, NULL, "program graph is missing expression node", "expression node", "missing node", NULL);
    return NULL;
  }
  Expr *expr = NULL;
  switch (node->kind) {
    case Z_PROGRAM_GRAPH_NODE_IDENTIFIER:
      expr = lower_new_expr(EXPR_IDENT, node);
      expr->text = z_strdup(node->name && node->name[0] ? node->name : "");
      lower_type_args(lower, node, &expr->type_args);
      return expr;
    case Z_PROGRAM_GRAPH_NODE_LITERAL:
      return lower_literal_expr(node);
    case Z_PROGRAM_GRAPH_NODE_FIELD_ACCESS:
      expr = lower_new_expr(EXPR_MEMBER, node);
      expr->text = z_strdup(node->name && node->name[0] ? node->name : "");
      expr->left = lower_required_expr(lower, node, "left", 0, "field access receiver");
      lower_type_args(lower, node, &expr->type_args);
      return expr;
    case Z_PROGRAM_GRAPH_NODE_INDEX_ACCESS:
      expr = lower_new_expr(EXPR_INDEX, node);
      expr->left = lower_required_expr(lower, node, "left", 0, "index receiver");
      expr->right = lower_required_expr(lower, node, "right", 1, "index expression");
      return expr;
    case Z_PROGRAM_GRAPH_NODE_SLICE:
      expr = lower_new_expr(EXPR_SLICE, node);
      expr->left = lower_required_expr(lower, node, "left", 0, "slice receiver");
      lower_push_expr(&expr->args, lower_optional_expr(lower, node, "arg", 0));
      lower_push_expr(&expr->args, lower_optional_expr(lower, node, "arg", 1));
      return expr;
    case Z_PROGRAM_GRAPH_NODE_CALL:
    case Z_PROGRAM_GRAPH_NODE_METHOD_CALL:
      if (lower_binary_operator(node->name) &&
          lower_ordered_node(lower->graph, node->id, "left", 0) &&
          lower_ordered_node(lower->graph, node->id, "right", 1)) {
        expr = lower_new_expr(EXPR_BINARY, node);
        expr->text = z_strdup(node->name);
        expr->left = lower_required_expr(lower, node, "left", 0, "binary left operand");
        expr->right = lower_required_expr(lower, node, "right", 1, "binary right operand");
        return expr;
      }
      expr = lower_new_expr(EXPR_CALL, node);
      expr->left = lower_optional_expr(lower, node, "left", 0);
      if (!expr->left && node->name && node->name[0]) {
        expr->left = lower_new_expr(EXPR_IDENT, node);
        expr->left->text = z_strdup(node->name);
      }
      if (!expr->left) lower_fail(lower, node, "program graph call is missing callee", "left edge or callee name", "missing callee", NULL);
      lower_type_args(lower, node, &expr->type_args);
      lower_args(lower, node, &expr->args);
      return expr;
    case Z_PROGRAM_GRAPH_NODE_CAST:
      expr = lower_new_expr(EXPR_CAST, node);
      expr->text = z_strdup(node->name && node->name[0] ? node->name : (node->type ? node->type : ""));
      expr->left = lower_required_expr(lower, node, "left", 0, "cast operand");
      return expr;
    case Z_PROGRAM_GRAPH_NODE_BORROW:
      expr = lower_new_expr(EXPR_BORROW, node);
      expr->mutable_borrow = node->is_mutable;
      expr->left = lower_required_expr(lower, node, "left", 0, "borrow operand");
      return expr;
    case Z_PROGRAM_GRAPH_NODE_CHECK:
      expr = lower_new_expr(EXPR_CHECK, node);
      expr->left = lower_required_expr(lower, node, "left", 0, "checked expression");
      return expr;
    case Z_PROGRAM_GRAPH_NODE_RESCUE:
      expr = lower_new_expr(EXPR_RESCUE, node);
      expr->text = z_strdup(node->name && node->name[0] ? node->name : "");
      expr->left = lower_required_expr(lower, node, "left", 0, "rescue expression");
      expr->right = lower_required_expr(lower, node, "right", 1, "rescue fallback expression");
      return expr;
    case Z_PROGRAM_GRAPH_NODE_META:
      expr = lower_new_expr(EXPR_META, node);
      expr->left = lower_required_expr(lower, node, "left", 0, "meta expression");
      return expr;
    case Z_PROGRAM_GRAPH_NODE_SHAPE_LITERAL:
      expr = lower_new_expr(EXPR_SHAPE_LITERAL, node);
      expr->text = z_strdup(node->name && node->name[0] ? node->name : "");
      lower_shape_fields(lower, node, &expr->fields);
      return expr;
    case Z_PROGRAM_GRAPH_NODE_ARRAY_LITERAL:
      expr = lower_new_expr(EXPR_ARRAY_LITERAL, node);
      expr->array_repeat = lower_text_eq(node->value, "repeat");
      lower_args(lower, node, &expr->args);
      return expr;
    default:
      lower_fail(lower, node, "program graph expression kind is not supported by direct lowering", "expression node", z_program_graph_node_kind_name(node->kind), NULL);
      return NULL;
  }
}

static MatchArm lower_match_arm(GraphLower *lower, const ZProgramGraphNode *node) {
  MatchArm arm = {
    .case_name = z_strdup(node && node->name ? node->name : ""),
    .payload_name = node && node->value && node->value[0] ? z_strdup(node->value) : NULL,
    .line = node && node->line > 0 ? node->line : 1,
    .column = node && node->column > 0 ? node->column : 1,
  };
  const ZProgramGraphNode *range_end = lower_ordered_node(lower->graph, node ? node->id : NULL, "rangeEnd", 0);
  if (range_end) arm.range_end = z_strdup(range_end->value && range_end->value[0] ? range_end->value : (range_end->name ? range_end->name : ""));
  arm.guard = lower_optional_expr(lower, node, "guard", 0);
  const ZProgramGraphNode *body = lower_ordered_node(lower->graph, node ? node->id : NULL, "body", 0);
  if (!body) lower_fail(lower, node, "program graph match arm is missing body block", "body block edge", "missing body", NULL);
  else arm.body = lower_block(lower, body);
  return arm;
}

static void lower_match_arms(GraphLower *lower, const ZProgramGraphNode *node, MatchArmVec *out) {
  bool have_last = false;
  size_t last_order = 0;
  for (;;) {
    const ZProgramGraphEdge *edge = lower_next_edge_by_order(lower->graph, node ? node->id : NULL, "arm", have_last, last_order);
    if (!edge) break;
    const ZProgramGraphNode *arm = lower_find_node(lower->graph, edge->to);
    last_order = edge->order;
    have_last = true;
    if (!arm) continue;
    lower_push_match_arm(out, lower_match_arm(lower, arm));
    if (lower_has_diag(lower)) return;
  }
}

static Stmt *lower_stmt(GraphLower *lower, const ZProgramGraphNode *node) {
  if (!node) {
    lower_fail(lower, NULL, "program graph is missing statement node", "statement node", "missing node", NULL);
    return NULL;
  }
  Stmt *stmt = NULL;
  switch (node->kind) {
    case Z_PROGRAM_GRAPH_NODE_LET:
      stmt = lower_new_stmt(STMT_LET, node);
      stmt->name = z_strdup(node->name && node->name[0] ? node->name : "");
      stmt->type = node->type && node->type[0] ? z_strdup(node->type) : NULL;
      stmt->mutable_binding = node->is_mutable;
      stmt->expr = lower_required_expr(lower, node, "expr", 0, "let initializer");
      return stmt;
    case Z_PROGRAM_GRAPH_NODE_ASSIGNMENT:
      stmt = lower_new_stmt(STMT_ASSIGN, node);
      stmt->target = lower_required_expr(lower, node, "target", 0, "assignment target");
      stmt->expr = lower_required_expr(lower, node, "expr", 0, "assignment value");
      return stmt;
    case Z_PROGRAM_GRAPH_NODE_DEFER:
      stmt = lower_new_stmt(STMT_DEFER, node);
      stmt->expr = lower_required_expr(lower, node, "expr", 0, "defer expression");
      return stmt;
    case Z_PROGRAM_GRAPH_NODE_CHECK:
      stmt = lower_new_stmt(STMT_CHECK, node);
      stmt->expr = lower_required_expr(lower, node, "expr", 0, "check expression");
      return stmt;
    case Z_PROGRAM_GRAPH_NODE_RETURN:
      stmt = lower_new_stmt(STMT_RETURN, node);
      stmt->expr = lower_optional_expr(lower, node, "expr", 0);
      return stmt;
    case Z_PROGRAM_GRAPH_NODE_EXPRESSION_STATEMENT:
      stmt = lower_new_stmt(STMT_EXPR, node);
      stmt->expr = lower_required_expr(lower, node, "expr", 0, "statement expression");
      return stmt;
    case Z_PROGRAM_GRAPH_NODE_IF:
      stmt = lower_new_stmt(STMT_IF, node);
      stmt->expr = lower_required_expr(lower, node, "expr", 0, "if condition");
      stmt->then_body = lower_block(lower, lower_ordered_node(lower->graph, node->id, "then", 0));
      stmt->else_body = lower_block(lower, lower_ordered_node(lower->graph, node->id, "else", 1));
      return stmt;
    case Z_PROGRAM_GRAPH_NODE_WHILE:
      stmt = lower_new_stmt(STMT_WHILE, node);
      stmt->expr = lower_required_expr(lower, node, "expr", 0, "while condition");
      stmt->then_body = lower_block(lower, lower_ordered_node(lower->graph, node->id, "then", 0));
      return stmt;
    case Z_PROGRAM_GRAPH_NODE_FOR:
      stmt = lower_new_stmt(STMT_FOR, node);
      stmt->name = z_strdup(node->name && node->name[0] ? node->name : "");
      stmt->expr = lower_required_expr(lower, node, "expr", 0, "for range start");
      stmt->range_end = lower_required_expr(lower, node, "rangeEnd", 1, "for range end");
      stmt->then_body = lower_block(lower, lower_ordered_node(lower->graph, node->id, "then", 0));
      return stmt;
    case Z_PROGRAM_GRAPH_NODE_BREAK:
      return lower_new_stmt(STMT_BREAK, node);
    case Z_PROGRAM_GRAPH_NODE_CONTINUE:
      return lower_new_stmt(STMT_CONTINUE, node);
    case Z_PROGRAM_GRAPH_NODE_MATCH:
      stmt = lower_new_stmt(STMT_MATCH, node);
      stmt->expr = lower_required_expr(lower, node, "expr", 0, "match expression");
      lower_match_arms(lower, node, &stmt->match_arms);
      return stmt;
    case Z_PROGRAM_GRAPH_NODE_RAISE:
      stmt = lower_new_stmt(STMT_RAISE, node);
      stmt->name = z_strdup(node->name && node->name[0] ? node->name : "");
      return stmt;
    default:
      lower_fail(lower, node, "program graph statement kind is not supported by direct lowering", "statement node", z_program_graph_node_kind_name(node->kind), NULL);
      return NULL;
  }
}

static StmtVec lower_block(GraphLower *lower, const ZProgramGraphNode *block) {
  StmtVec body = {0};
  if (!block) return body;
  if (block->kind != Z_PROGRAM_GRAPH_NODE_BLOCK) {
    lower_fail(lower, block, "program graph body edge does not point to a block", "Block node", z_program_graph_node_kind_name(block->kind), NULL);
    return body;
  }
  bool have_last = false;
  size_t last_order = 0;
  for (;;) {
    const ZProgramGraphEdge *edge = lower_next_edge_by_order(lower->graph, block->id, "statement", have_last, last_order);
    if (!edge) break;
    const ZProgramGraphNode *stmt_node = lower_find_node(lower->graph, edge->to);
    last_order = edge->order;
    have_last = true;
    Stmt *stmt = lower_stmt(lower, stmt_node);
    if (lower_has_diag(lower)) return body;
    lower_push_stmt(&body, stmt);
  }
  return body;
}

static void lower_params(GraphLower *lower, const ZProgramGraphNode *owner, const char *edge_kind, ParamVec *out) {
  bool have_last = false;
  size_t last_order = 0;
  for (;;) {
    const ZProgramGraphEdge *edge = lower_next_edge_by_order(lower->graph, owner ? owner->id : NULL, edge_kind, have_last, last_order);
    if (!edge) break;
    const ZProgramGraphNode *node = lower_find_node(lower->graph, edge->to);
    last_order = edge->order;
    have_last = true;
    if (!node) continue;
    Param param = {
      .name = z_strdup(node->name && node->name[0] ? node->name : ""),
      .type = node->type && node->type[0] ? z_strdup(node->type) : NULL,
      .default_value = lower_optional_expr(lower, node, "default", 0),
      .is_static = node->is_static,
      .line = node->line > 0 ? node->line : 1,
      .column = node->column > 0 ? node->column : 1,
    };
    lower_push_param(out, param);
    if (lower_has_diag(lower)) return;
  }
}

static Function lower_function(GraphLower *lower, const ZProgramGraphNode *node) {
  Function fun = {
    .name = z_strdup(node && node->name ? node->name : ""),
    .test_name = node && lower_starts_with(node->name, "__zero_test_") ? z_strdup(node->value ? node->value : "") : NULL,
    .return_type = z_strdup(node && node->type && node->type[0] ? node->type : "Void"),
    .is_public = node && node->is_public,
    .raises = node && node->fallible,
    .is_test = node && lower_starts_with(node->name, "__zero_test_"),
    .export_c = node && node->export_c,
    .line = node && node->line > 0 ? node->line : 1,
    .column = node && node->column > 0 ? node->column : 1,
  };
  lower_params(lower, node, "typeParam", &fun.type_params);
  lower_params(lower, node, "param", &fun.params);
  lower_params(lower, node, "error", &fun.errors);
  fun.has_error_set = fun.errors.len > 0;
  const ZProgramGraphNode *body = lower_ordered_node(lower->graph, node ? node->id : NULL, "body", 0);
  if (!body) lower_fail(lower, node, "program graph function is missing body block", "body block edge", "missing body", NULL);
  else fun.body = lower_block(lower, body);
  return fun;
}

static void lower_methods(GraphLower *lower, const ZProgramGraphNode *owner, FunctionVec *out) {
  bool have_last = false;
  size_t last_order = 0;
  for (;;) {
    const ZProgramGraphEdge *edge = lower_next_edge_by_order(lower->graph, owner ? owner->id : NULL, "method", have_last, last_order);
    if (!edge) break;
    const ZProgramGraphNode *method = lower_find_node(lower->graph, edge->to);
    last_order = edge->order;
    have_last = true;
    if (!method) continue;
    if (method->kind != Z_PROGRAM_GRAPH_NODE_FUNCTION) {
      lower_fail(lower, method, "program graph method edge does not point to a function", "Function node", z_program_graph_node_kind_name(method->kind), NULL);
      return;
    }
    lower_push_function(out, lower_function(lower, method));
    if (lower_has_diag(lower)) return;
  }
}

static void lower_import(GraphLower *lower, Program *program, const ZProgramGraphNode *node) {
  (void)lower;
  lower_push_use(&program->use_imports, (UseImport){
    .module = z_strdup(node->name && node->name[0] ? node->name : ""),
    .alias = node->value && node->value[0] ? z_strdup(node->value) : NULL,
    .line = node->line > 0 ? node->line : 1,
    .column = node->column > 0 ? node->column : 1,
    .end_column = node->column > 0 ? node->column : 1,
  });
}

static void lower_c_import(GraphLower *lower, Program *program, const ZProgramGraphNode *node) {
  (void)lower;
  lower_push_c_import(&program->c_imports, (CImport){
    .header = z_strdup(node->value && node->value[0] ? node->value : ""),
    .alias = z_strdup(node->name && node->name[0] ? node->name : ""),
    .line = node->line > 0 ? node->line : 1,
    .column = node->column > 0 ? node->column : 1,
  });
}

static void lower_const(GraphLower *lower, Program *program, const ZProgramGraphNode *node) {
  ConstDecl item = {
    .name = z_strdup(node->name && node->name[0] ? node->name : ""),
    .type = node->type && node->type[0] ? z_strdup(node->type) : NULL,
    .expr = lower_required_expr(lower, node, "value", 0, "const value"),
    .is_public = node->is_public,
    .line = node->line > 0 ? node->line : 1,
    .column = node->column > 0 ? node->column : 1,
  };
  lower_push_const(&program->consts, item);
}

static void lower_alias(GraphLower *lower, Program *program, const ZProgramGraphNode *node) {
  (void)lower;
  lower_push_alias(&program->aliases, (TypeAlias){
    .name = z_strdup(node->name && node->name[0] ? node->name : ""),
    .target = z_strdup(node->type && node->type[0] ? node->type : ""),
    .is_public = node->is_public,
    .line = node->line > 0 ? node->line : 1,
    .column = node->column > 0 ? node->column : 1,
  });
}

static void lower_shape(GraphLower *lower, Program *program, const ZProgramGraphNode *node) {
  Shape shape = {
    .name = z_strdup(node->name && node->name[0] ? node->name : ""),
    .layout = z_strdup(node->value && node->value[0] ? node->value : "auto"),
    .is_public = node->is_public,
    .line = node->line > 0 ? node->line : 1,
    .column = node->column > 0 ? node->column : 1,
  };
  lower_params(lower, node, "typeParam", &shape.type_params);
  lower_params(lower, node, "field", &shape.fields);
  lower_methods(lower, node, &shape.methods);
  lower_push_shape(&program->shapes, shape);
}

static void lower_interface(GraphLower *lower, Program *program, const ZProgramGraphNode *node) {
  InterfaceDecl item = {
    .name = z_strdup(node->name && node->name[0] ? node->name : ""),
    .is_public = node->is_public,
    .line = node->line > 0 ? node->line : 1,
    .column = node->column > 0 ? node->column : 1,
  };
  lower_params(lower, node, "typeParam", &item.type_params);
  lower_methods(lower, node, &item.methods);
  lower_push_interface(&program->interfaces, item);
}

static void lower_enum(GraphLower *lower, Program *program, const ZProgramGraphNode *node) {
  EnumDecl item = {
    .name = z_strdup(node->name && node->name[0] ? node->name : ""),
    .type = node->type && node->type[0] ? z_strdup(node->type) : NULL,
    .line = node->line > 0 ? node->line : 1,
    .column = node->column > 0 ? node->column : 1,
  };
  lower_params(lower, node, "case", &item.cases);
  lower_push_enum(&program->enums, item);
}

static void lower_choice(GraphLower *lower, Program *program, const ZProgramGraphNode *node) {
  Choice item = {
    .name = z_strdup(node->name && node->name[0] ? node->name : ""),
    .line = node->line > 0 ? node->line : 1,
    .column = node->column > 0 ? node->column : 1,
  };
  lower_params(lower, node, "case", &item.cases);
  lower_push_choice(&program->choices, item);
}

static void lower_top_level(GraphLower *lower, Program *program, const ZProgramGraphNode *node) {
  if (!node || lower_has_diag(lower)) return;
  switch (node->kind) {
    case Z_PROGRAM_GRAPH_NODE_IMPORT:
      lower_import(lower, program, node);
      break;
    case Z_PROGRAM_GRAPH_NODE_C_IMPORT:
      lower_c_import(lower, program, node);
      break;
    case Z_PROGRAM_GRAPH_NODE_CONST:
      lower_const(lower, program, node);
      break;
    case Z_PROGRAM_GRAPH_NODE_TYPE_ALIAS:
      lower_alias(lower, program, node);
      break;
    case Z_PROGRAM_GRAPH_NODE_SHAPE:
      lower_shape(lower, program, node);
      break;
    case Z_PROGRAM_GRAPH_NODE_INTERFACE:
      lower_interface(lower, program, node);
      break;
    case Z_PROGRAM_GRAPH_NODE_ENUM:
      lower_enum(lower, program, node);
      break;
    case Z_PROGRAM_GRAPH_NODE_CHOICE:
      lower_choice(lower, program, node);
      break;
    case Z_PROGRAM_GRAPH_NODE_FUNCTION:
      lower_push_function(&program->functions, lower_function(lower, node));
      break;
    default:
      lower_fail(lower, node, "program graph declaration kind is not supported by direct lowering", "declaration node", z_program_graph_node_kind_name(node->kind), NULL);
      break;
  }
}

static void lower_top_level_edges(GraphLower *lower, Program *program, const ZProgramGraphNode *module, const char *edge_kind) {
  bool have_last = false;
  size_t last_order = 0;
  for (;;) {
    const ZProgramGraphEdge *edge = lower_next_edge_by_order(lower->graph, module ? module->id : NULL, edge_kind, have_last, last_order);
    if (!edge) break;
    const ZProgramGraphNode *node = lower_find_node(lower->graph, edge->to);
    last_order = edge->order;
    have_last = true;
    lower_top_level(lower, program, node);
    if (lower_has_diag(lower)) return;
  }
}

static void lower_module(GraphLower *lower, Program *program, const ZProgramGraphNode *module) {
  lower_top_level_edges(lower, program, module, "cImport");
  lower_top_level_edges(lower, program, module, "import");
  lower_top_level_edges(lower, program, module, "const");
  lower_top_level_edges(lower, program, module, "alias");
  lower_top_level_edges(lower, program, module, "shape");
  lower_top_level_edges(lower, program, module, "interface");
  lower_top_level_edges(lower, program, module, "enum");
  lower_top_level_edges(lower, program, module, "choice");
  lower_top_level_edges(lower, program, module, "function");
}

bool z_program_graph_lower_to_program(const ZProgramGraph *graph, Program *out, ZDiag *diag) {
  if (!out) return false;
  *out = (Program){0};
  GraphLower lower = {.graph = graph, .diag = diag};
  if (!graph) return lower_fail(&lower, NULL, "program graph is missing", "ProgramGraph artifact", "missing graph", NULL);

  size_t module_count = 0;
  for (size_t i = 0; i < graph->node_len; i++) {
    const ZProgramGraphNode *node = &graph->nodes[i];
    if (node->kind != Z_PROGRAM_GRAPH_NODE_MODULE) continue;
    module_count++;
    lower_module(&lower, out, node);
    if (lower_has_diag(&lower)) {
      z_free_program(out);
      return false;
    }
  }
  if (module_count == 0) {
    lower_fail(&lower, NULL, "program graph has no module node", "at least one Module node", "missing module", NULL);
    z_free_program(out);
    return false;
  }
  return true;
}
