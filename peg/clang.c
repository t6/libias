/*-
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 *
 * Copyright (c) 2021 Tobias Kortkamp <tobik@FreeBSD.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "config.h"

#include "peg.h"
#include "peg/macros.h"

// Reference: https://cs.wmich.edu/~gupta/teaching/cs4850/sumII06/The%20syntax%20of%20C%20in%20Backus_Naur%20form.htm

RULE(abstract_declarator);
RULE(additive_expression);
RULE(and_expression);
RULE(assignment_expression);
RULE(assignment_operator);
RULE(cast_expression);
RULE(character_constant);
RULE(compound_statement);
RULE(conditional_expression);
RULE(constant_expression);
RULE(constant);
RULE(declaration_specifier);
RULE(declaration);
RULE(declarator);
RULE(direct_abstract_declarator);
RULE(direct_declarator);
RULE(enum_specifier);
RULE(enumeration_constant);
RULE(enumerator_list);
RULE(enumerator);
RULE(equality_expression);
RULE(exclusive_or_expression);
RULE(expression_statement);
RULE(expression);
RULE(external_declaration);
RULE(floating_constant);
RULE(function_definition);
RULE(identifier);
RULE(inclusive_or_expression);
RULE(init_declarator);
RULE(initializer_list);
RULE(initializer);
RULE(integer_constant);
RULE(iteration_statement);
RULE(jump_statement);
RULE(labeled_statement);
RULE(logical_and_expression);
RULE(logical_or_expression);
RULE(multiplicative_expression);
RULE(parameter_declaration);
RULE(parameter_list);
RULE(parameter_type_list);
RULE(pointer);
RULE(postfix_expression);
RULE(primary_expression);
RULE(relational_expression);
RULE(selection_statement);
RULE(shift_expression);
RULE(specifier_qualifier);
RULE(statement);
RULE(storage_class_specifier);
RULE(string);
RULE(struct_declaration);
RULE(struct_declarator_list);
RULE(struct_declarator);
RULE(struct_or_union_specifier);
RULE(struct_or_union);
RULE(type_name);
RULE(type_qualifier);
RULE(type_specifier);
RULE(typedef_name);
RULE(unary_expression);
RULE(unary_operator);
RULE(ws);

MAIN_RULE(translation_unit) {
	return ANY(external_declaration);
}

RULE(external_declaration) {
	if (!MATCH(function_definition))
	if (!MATCH(declaration))
	return 0;
	return 1;
}

RULE(function_definition) {
	if (ANY(declaration_specifier))
	if (MATCH(declarator))
	if (ANY(declaration))
	if (MATCH(compound_statement))
	return 1;
	return 0;
}

RULE(declaration_specifier) {
	if (!MATCH(storage_class_specifier))
	if (!MATCH(type_specifier))
	if (!MATCH(type_qualifier))
	return 0;
	return 1;
}

RULE(storage_class_specifier_0) {
	if (!STRING("auto"))
	if (!STRING("register"))
	if (!STRING("static"))
	if (!STRING("extern"))
	if (!STRING("typedef"))
	return 0;
	return 1;
}

RULE(storage_class_specifier) {
	if (ANY(ws))
	if (MATCH(storage_class_specifier_0))
	if (SOME(ws))
	return 1;
	return 0;
}

RULE(type_specifier_0) {
	if (!STRING("void"))
	if (!STRING("char"))
	if (!STRING("short"))
	if (!STRING("int"))
	if (!STRING("long"))
	if (!STRING("float"))
	if (!STRING("double"))
	if (!STRING("signed"))
	if (!STRING("unsigned"))
	if (!MATCH(struct_or_union_specifier))
	if (!MATCH(enum_specifier))
	if (!MATCH(typedef_name))
	return 0;
	return 1;
}

RULE(type_specifier) {
	if (ANY(ws))
	if (MATCH(type_specifier_0))
	if (SOME(ws))
	return 1;
	return 0;
}

RULE(struct_or_union_specifier) {
	if (MATCH(struct_or_union))
	if (OPT(MATCH(identifier)))
	if (OPT(SOME(struct_declaration)))
	return 1;
	return 0;
}

RULE(struct_or_union_0) {
	if (!STRING("struct"))
	if (!STRING("union"))
	return 0;
	return 1;
}

RULE(struct_or_union) {
	if (ANY(ws))
	if (MATCH(struct_or_union_0))
	if (ANY(ws))
	return 1;
	return 0;
}

RULE(struct_declaration) {
	if (ANY(specifier_qualifier))
	if (MATCH(struct_declarator_list))
	return 1;
	return 0;
}

RULE(specifier_qualifier) {
	if (!MATCH(type_specifier))
	if (!MATCH(type_qualifier))
	return 0;
	return 1;
}


RULE(struct_declarator_list_0) {
	if (MATCH(struct_declarator_list))
	if (ANY(ws))
	if (CHAR(','))
	if (ANY(ws))
	if (MATCH(struct_declarator))
	return 1;
	return 0;
}

RULE(struct_declarator_list) {
	if (!MATCH(struct_declarator_list_0))
	if (!MATCH(struct_declarator))
	return 0;
	return 1;
}

RULE(struct_declarator_0) {
	if (MATCH(declarator))
	if (ANY(ws))
	if (CHAR(':'))
	if (ANY(ws))
	if (MATCH(constant_expression))
	return 1;
	return 0;
}

RULE(struct_declarator_1) {
	if (ANY(ws))
	if (CHAR(':'))
	if (ANY(ws))
	if (MATCH(constant_expression))
	return 1;
	return 0;
}

RULE(struct_declarator) {
	if (!MATCH(struct_declarator_0))
	if (!MATCH(declarator))
	if (!MATCH(struct_declarator_1))
	return 0;
	return 1;
}

RULE(declarator) {
	if (OPT(MATCH(pointer)))
	if (MATCH(direct_declarator))
	return 1;
	return 0;
}

RULE(pointer) {
	if (ANY(ws))
	if (CHAR('*'))
	if (ANY(ws))
	if (ANY(type_qualifier))
	if (OPT(MATCH(pointer)))
	return 1;
	return 0;
}

RULE(type_qualifier_0) {
	if (!STRING("const"))
	if (!STRING("volatile"))
	return 0;
	return 1;
}

RULE(type_qualifier) {
	if (ANY(ws))
	if (MATCH(type_qualifier_0))
	if (SOME(ws))
	return 1;
	return 0;
}

RULE(direct_declarator_0) {
	if (ANY(ws))
	if (CHAR('('))
	if (ANY(ws))
	if (MATCH(declarator))
	if (ANY(ws))
	if (CHAR(')'))
	if (ANY(ws))
	return 1;
	return 0;
}

RULE(direct_declarator_1) {
	if (MATCH(direct_declarator))
	if (ANY(ws))
	if (CHAR('['))
	if (ANY(ws))
	if (OPT(MATCH(constant_expression)))
	if (ANY(ws))
	if (CHAR(']'))
	if (ANY(ws))
	return 1;
	return 0;
}

RULE(direct_declarator_2) {
	if (MATCH(direct_declarator))
	if (ANY(ws))
	if (CHAR('('))
	if (ANY(ws))
	if (MATCH(parameter_type_list))
	if (ANY(ws))
	if (CHAR(')'))
	if (ANY(ws))
	return 1;
	return 0;
}

RULE(direct_declarator_3) {
	if (MATCH(direct_declarator))
	if (ANY(ws))
	if (CHAR('('))
	if (ANY(ws))
	if (ANY(identifier))
	if (ANY(ws))
	if (CHAR(')'))
	if (ANY(ws))
	return 1;
	return 0;
}

RULE(direct_declarator) {
	if (!MATCH(identifier))
	if (!MATCH(direct_declarator_0))
	if (!MATCH(direct_declarator_1))
	if (!MATCH(direct_declarator_2))
	if (!MATCH(direct_declarator_3))
	return 0;
	return 1;
}

RULE(constant_expression) { return MATCH(conditional_expression); }

RULE(conditional_expression_0) {
	if (MATCH(logical_or_expression))
	if (ANY(ws))
	if (CHAR('?'))
	if (ANY(ws))
	if (MATCH(expression))
	if (ANY(ws))
	if (CHAR(':'))
	if (ANY(ws))
	if (MATCH(conditional_expression))
	return 1;
	return 0;
}

RULE(conditional_expression) {
	if (!MATCH(logical_or_expression))
	if (!MATCH(conditional_expression_0))
	return 0;
	return 1;
}

RULE(logical_or_expression_0) {
	if (MATCH(logical_or_expression))
	if (ANY(ws))
	if (STRING("||"))
	if (ANY(ws))
	if (MATCH(logical_and_expression))
	return 1;
	return 0;
}

RULE(logical_or_expression) {
	if (!MATCH(logical_and_expression))
	if (!MATCH(logical_or_expression_0))
	return 0;
	return 1;
}

RULE(logical_and_expression_0) {
	if (MATCH(logical_and_expression))
	if (ANY(ws))
	if (STRING("&&"))
	if (ANY(ws))
	if (MATCH(inclusive_or_expression))
	return 1;
	return 0;
}

RULE(logical_and_expression) {
	if (!MATCH(inclusive_or_expression))
	if (!MATCH(logical_and_expression_0))
	return 0;
	return 1;
}

RULE(inclusive_or_expression) {
	if (!MATCH(exclusive_or_expression))
	if (!MATCH(inclusive_or_expression))
	if (!MATCH(exclusive_or_expression))
	return 0;
	return 1;
}

RULE(exclusive_or_expression_0) {
	if (MATCH(exclusive_or_expression))
	if (ANY(ws))
	if (CHAR('^'))
	if (ANY(ws))
	if (MATCH(and_expression))
	return 1;
	return 0;
}

RULE(exclusive_or_expression) {
	if (!MATCH(and_expression))
	if (!MATCH(exclusive_or_expression_0))
	return 0;
	return 1;
}

RULE(and_expression_0) {
	if (MATCH(and_expression))
	if (ANY(ws))
	if (CHAR('&'))
	if (ANY(ws))
	if (MATCH(equality_expression))
	return 1;
	return 0;
}

RULE(and_expression) {
	if (!MATCH(equality_expression))
	if (!MATCH(and_expression_0))
	return 0;
	return 1;
}

RULE(equality_expression_0) {
	if (MATCH(equality_expression))
	if (ANY(ws))
	if (STRING("=="))
	if (ANY(ws))
	if (MATCH(relational_expression))
	return 1;
	return 0;
}

RULE(equality_expression_1) {
	if (MATCH(equality_expression))
	if (ANY(ws))
	if (STRING("!="))
	if (ANY(ws))
	if (MATCH(relational_expression))
	return 1;
	return 0;
}

RULE(equality_expression) {
	if (!MATCH(relational_expression))
	if (!MATCH(equality_expression_0))
	if (!MATCH(equality_expression_1))
	return 0;
	return 1;
}

RULE(relational_expression_0) {
	if (MATCH(relational_expression))
	if (ANY(ws))
	if (STRING("<="))
	if (ANY(ws))
	if (MATCH(shift_expression))
	return 1;
	return 0;
}

RULE(relational_expression_1) {
	if (MATCH(relational_expression))
	if (ANY(ws))
	if (STRING(">="))
	if (ANY(ws))
	if (MATCH(shift_expression))
	return 1;
	return 0;
}

RULE(relational_expression_2) {
	if (MATCH(relational_expression))
	if (ANY(ws))
	if (CHAR('<'))
	if (ANY(ws))
	if (MATCH(shift_expression))
	return 1;
	return 0;
}

RULE(relational_expression_3) {
	if (MATCH(relational_expression))
	if (ANY(ws))
	if (CHAR('>'))
	if (ANY(ws))
	if (MATCH(shift_expression))
	return 1;
	return 0;
}

RULE(relational_expression) {
	if (!MATCH(shift_expression))
	if (!MATCH(relational_expression_0))
	if (!MATCH(relational_expression_1))
	if (!MATCH(relational_expression_2))
	if (!MATCH(relational_expression_3))
	return 0;
	return 1;
}

RULE(shift_expression_0) {
	if (MATCH(shift_expression))
	if (ANY(ws))
	if (STRING("<<"))
	if (ANY(ws))
	if (MATCH(additive_expression))
	return 1;
	return 0;
}

RULE(shift_expression_1) {
	if (MATCH(shift_expression))
	if (ANY(ws))
	if (STRING(">>"))
	if (ANY(ws))
	if (MATCH(additive_expression))
	return 1;
	return 0;
}

RULE(shift_expression) {
	if (!MATCH(additive_expression))
	if (!MATCH(shift_expression_0))
	if (!MATCH(shift_expression_1))
	return 0;
	return 1;
}

RULE(additive_expression_0) {
	if (MATCH(additive_expression))
	if (ANY(ws))
	if (CHAR('+'))
	if (ANY(ws))
	if (MATCH(multiplicative_expression))
	return 1;
	return 0;
}

RULE(additive_expression_1) {
	if (MATCH(additive_expression))
	if (ANY(ws))
	if (CHAR('-'))
	if (ANY(ws))
	if (MATCH(multiplicative_expression))
	return 1;
	return 0;
}

RULE(additive_expression) {
	if (!MATCH(multiplicative_expression))
	if (!MATCH(additive_expression_0))
	if (!MATCH(additive_expression_1))
	return 0;
	return 1;
}

RULE(multiplicative_expression_0) {
	if (MATCH(multiplicative_expression))
	if (ANY(ws))
	if (CHAR('*'))
	if (ANY(ws))
	if (MATCH(cast_expression))
	return 1;
	return 0;
}

RULE(multiplicative_expression_1) {
	if (MATCH(multiplicative_expression))
	if (ANY(ws))
	if (CHAR('/'))
	if (ANY(ws))
	if (MATCH(cast_expression))
	return 1;
	return 0;
}

RULE(multiplicative_expression_2) {
	if (MATCH(multiplicative_expression))
	if (ANY(ws))
	if (CHAR('%'))
	if (ANY(ws))
	if (MATCH(cast_expression))
	return 1;
	return 0;
}

RULE(multiplicative_expression) {
	if (!MATCH(cast_expression))
	if (!MATCH(multiplicative_expression_0))
	if (!MATCH(multiplicative_expression_1))
	if (!MATCH(multiplicative_expression_2))
	return 0;
	return 1;
}

RULE(cast_expression_0) {
	if (ANY(ws))
	if (CHAR('('))
	if (ANY(ws))
	if (MATCH(type_name))
	if (ANY(ws))
	if (CHAR(')'))
	if (ANY(ws))
	if (MATCH(cast_expression))
	return 1;
	return 0;
}

RULE(cast_expression) {
	if (!MATCH(unary_expression))
	if (!MATCH(cast_expression_0))
	return 0;
	return 1;
}

RULE(unary_expression_0) {
	if (ANY(ws))
	if (STRING("++"))
	if (ANY(ws))
	if (MATCH(unary_expression))
	return 1;
	return 0;
}

RULE(unary_expression_1) {
	if (ANY(ws))
	if (STRING("--"))
	if (ANY(ws))
	if (MATCH(unary_expression))
	return 1;
	return 0;
}

RULE(unary_expression_2) {
	if (MATCH(unary_operator))
	if (MATCH(cast_expression))
	return 1;
	return 0;
}

RULE(unary_expression_3) {
	if (ANY(ws))
	if (STRING("sizeof"))
	if (ANY(ws))
	if (MATCH(unary_expression))
	return 1;
	return 0;
}

RULE(unary_expression_4) {
	if (ANY(ws))
	if (STRING("sizeof"))
	if (ANY(ws))
	if (MATCH(type_name))
	return 1;
	return 0;
}

RULE(unary_expression) {
	if (!MATCH(postfix_expression))
	if (!MATCH(unary_expression_0))
	if (!MATCH(unary_expression_1))
	if (!MATCH(unary_expression_2))
	if (!MATCH(unary_expression_3))
	if (!MATCH(unary_expression_4))
	return 0;
	return 1;
}

RULE(postfix_expression_0) {
	if (MATCH(postfix_expression))
	if (ANY(ws))
	if (CHAR('['))
	if (ANY(ws))
	if (MATCH(expression))
	if (ANY(ws))
	if (CHAR(']'))
	if (ANY(ws))
	return 1;
	return 0;
}

RULE(postfix_expression_1) {
	if (MATCH(postfix_expression))
	if (ANY(ws))
	if (CHAR('('))
	if (ANY(ws))
	if (ANY(assignment_expression))
	if (ANY(ws))
	if (CHAR(')'))
	if (ANY(ws))
	return 1;
	return 0;
}

RULE(postfix_expression_2) {
	if (MATCH(postfix_expression))
	if (ANY(ws))
	if (CHAR('.'))
	if (ANY(ws))
	if (MATCH(identifier))
	return 1;
	return 0;
}

RULE(postfix_expression_3) {
	if (MATCH(postfix_expression))
	if (ANY(ws))
	if (STRING("->"))
	if (ANY(ws))
	if (MATCH(identifier))
	return 1;
	return 0;
}

RULE(postfix_expression_4) {
	if (MATCH(postfix_expression))
	if (ANY(ws))
	if (STRING("++"))
	if (ANY(ws))
	return 1;
	return 0;
}

RULE(postfix_expression_5) {
	if (MATCH(postfix_expression))
	if (ANY(ws))
	if (STRING("--"))
	if (ANY(ws))
	return 1;
	return 0;
}

RULE(postfix_expression) {
	if (!MATCH(primary_expression))
	if (!MATCH(postfix_expression_0))
	if (!MATCH(postfix_expression_1))
	if (!MATCH(postfix_expression_2))
	if (!MATCH(postfix_expression_3))
	if (!MATCH(postfix_expression_4))
	if (!MATCH(postfix_expression_5))
	return 0;
	return 1;
}

RULE(primary_expression_0) {
	if (ANY(ws))
	if (CHAR('('))
	if (ANY(ws))
	if (MATCH(expression))
	if (ANY(ws))
	if (CHAR(')'))
	if (ANY(ws))
	return 1;
	return 0;
}

RULE(primary_expression) {
	if (!MATCH(identifier))
	if (!MATCH(constant))
	if (!MATCH(string))
	if (!MATCH(primary_expression_0))
	return 0;
	return 1;
}

RULE(constant) {
	if (!MATCH(integer_constant))
	if (!MATCH(character_constant))
	if (!MATCH(floating_constant))
	if (!MATCH(enumeration_constant))
	return 0;
	return 1;
}

RULE(expression_0) {
	if (MATCH(expression))
	if (ANY(ws))
	if (CHAR(','))
	if (ANY(ws))
	if (MATCH(assignment_expression))
	return 1;
	return 0;
}

RULE(expression) {
	if (!MATCH(assignment_expression))
	if (!MATCH(expression_0))
	return 0;
	return 1;
}

RULE(assignment_expression_0) {
	if (MATCH(unary_expression))
	if (MATCH(assignment_operator))
	if (MATCH(assignment_expression))
	return 1;
	return 0;
}

RULE(assignment_expression) {
	if (!MATCH(conditional_expression))
	if (!MATCH(assignment_expression_0))
	return 0;
	return 1;
}

RULE(assignment_operator_0) {
	if (!STRING("=="))
	if (!STRING("*="))
	if (!STRING("/="))
	if (!STRING("%="))
	if (!STRING("+="))
	if (!STRING("_="))
	if (!STRING("(="))
	if (!STRING("))="))
	if (!STRING("&="))
	if (!STRING("^="))
	return 0;
	return 1;
}

RULE(assignment_operator) {
	if (ANY(ws))
	if (MATCH(assignment_operator_0))
	if (ANY(ws))
	return 1;
	return 0;
}

RULE(unary_operator_0) { return SET('&', '*', '+', '_', '~', '!'); }
RULE(unary_operator) {
	if (ANY(ws))
	if (MATCH(unary_operator_0))
	if (ANY(ws))
	return 1;
	return 0;
}

RULE(type_name) {
	if (SOME(specifier_qualifier))
	if (OPT(MATCH(abstract_declarator)))
	return 1;
	return 0;
}

RULE(parameter_type_list_0) {
	if (MATCH(parameter_list))
	if (ANY(ws))
	if (CHAR(','))
	if (ANY(ws))
	if (STRING("..."))
	if (ANY(ws))
	return 1;
	return 0;
}

RULE(parameter_type_list) {
	if (!MATCH(parameter_list))
	if (!MATCH(parameter_type_list_0))
	return 0;
	return 1;
}

RULE(parameter_list_0) {
	if (MATCH(parameter_list))
	if (ANY(ws))
	if (CHAR(','))
	if (ANY(ws))
	if (MATCH(parameter_declaration))
	return 1;
	return 0;
}

RULE(parameter_list) {
	if (!MATCH(parameter_declaration))
	if (!MATCH(parameter_list_0))
	return 0;
	return 1;
}

RULE(parameter_declaration_0) {
	if (SOME(declaration_specifier))
	if (MATCH(declarator))
	return 1;
	return 0;
}

RULE(parameter_declaration_1) {
	if (SOME(declaration_specifier))
	if (MATCH(abstract_declarator))
	return 1;
	return 0;
}

RULE(parameter_declaration) {
	if (!MATCH(parameter_declaration_0))
	if (!MATCH(parameter_declaration_1))
	if (!SOME(declaration_specifier))
	return 0;
	return 1;
}

RULE(abstract_declarator_0) {
	if (!MATCH(pointer))
	if (!MATCH(direct_abstract_declarator))
	return 0;
	return 1;
}

RULE(abstract_declarator) {
	if (!MATCH(abstract_declarator_0))
	if (!MATCH(pointer))
	if (!MATCH(direct_abstract_declarator))
	return 0;
	return 1;
}

RULE(direct_abstract_declarator_0) {
	if (ANY(ws))
	if (CHAR('('))
	if (ANY(ws))
	if (MATCH(abstract_declarator))
	if (ANY(ws))
	if (CHAR(')'))
	if (ANY(ws))
	return 1;
	return 0;
}

RULE(direct_abstract_declarator_1) {
	if (OPT(MATCH(direct_abstract_declarator)))
	if (ANY(ws))
	if (CHAR('['))
	if (ANY(ws))
	if (OPT(MATCH(constant_expression)))
	if (ANY(ws))
	if (CHAR(']'))
	if (ANY(ws))
	return 1;
	return 0;
}

RULE(direct_abstract_declarator_2) {
	if (OPT(MATCH(direct_abstract_declarator)))
	if (ANY(ws))
	if (CHAR('('))
	if (ANY(ws))
	if (OPT(MATCH(parameter_type_list)))
	if (ANY(ws))
	if (CHAR(')'))
	if (ANY(ws))
	return 1;
	return 0;
}

RULE(direct_abstract_declarator) {
	if (!MATCH(direct_abstract_declarator_0))
	if (!MATCH(direct_abstract_declarator_1))
	if (!MATCH(direct_abstract_declarator_2))
	return 0;
	return 1;
}

RULE(enum_specifier_0) {
	if (ANY(ws))
	if (STRING("enum"))
	if (ANY(ws))
	if (MATCH(identifier))
	if (ANY(ws))
	if (CHAR('{'))
	if (ANY(ws))
	if (MATCH(enumerator_list))
	if (ANY(ws))
	if (CHAR('}'))
	if (ANY(ws))
	return 1;
	return 0;
}

RULE(enum_specifier_1) {
	if (ANY(ws))
	if (STRING("enum"))
	if (ANY(ws))
	if (CHAR('{'))
	if (ANY(ws))
	if (MATCH(enumerator_list))
	if (ANY(ws))
	if (CHAR('}'))
	if (ANY(ws))
	if (MATCH(identifier))
	return 1;
	return 0;
}

RULE(enum_specifier_2) {
	if (ANY(ws))
	if (STRING("enum"))
	if (ANY(ws))
	if (MATCH(identifier))
	return 1;
	return 0;
}

RULE(enum_specifier) {
	if (!MATCH(enum_specifier_0))
	if (!MATCH(enum_specifier_1))
	if (!MATCH(enum_specifier_2))
	return 0;
	return 1;
}

RULE(enumerator_list_0) {
	if (MATCH(enumerator_list))
	if (ANY(ws))
	if (CHAR(','))
	if (ANY(ws))
	if (MATCH(enumerator))
	return 1;
	return 0;
}

RULE(enumerator_list) {
	if (!MATCH(enumerator_list_0))
	if (!MATCH(enumerator))
	return 0;
	return 1;
}

RULE(enumerator_0) {
	if (MATCH(identifier))
	if (ANY(ws))
	if (CHAR('='))
	if (ANY(ws))
	if (!MATCH(constant_expression))
	return 0;
	return 1;
}

RULE(enumerator) {
	if (!MATCH(enumerator_0))
	if (!MATCH(identifier))
	return 0;
	return 1;
}

RULE(typedef_name) { return MATCH(identifier); }

RULE(declaration) {
	if (SOME(declaration_specifier))
	if (ANY(init_declarator))
	if (ANY(ws))
	if (CHAR(';'))
	if (ANY(ws))
	return 1;
	return 0;
}

RULE(init_declarator_0) {
	if (MATCH(declarator))
	if (ANY(ws))
	if (CHAR('='))
	if (ANY(ws))
	if (MATCH(initializer))
	return 1;
	return 0;
}

RULE(init_declarator) {
	if (!MATCH(init_declarator_0))
	if (!MATCH(declarator))
	return 1;
	return 0;
}

RULE(initializer_0) {
	if (ANY(ws))
	if (CHAR('{'))
	if (ANY(ws))
	if (MATCH(initializer_list))
	if (ANY(ws))
	if (CHAR(','))
	if (ANY(ws))
	if (CHAR('}'))
	if (ANY(ws))
	return 1;
	return 0;
}

RULE(initializer_1) {
	if (ANY(ws))
	if (CHAR('{'))
	if (ANY(ws))
	if (MATCH(initializer_list))
	if (ANY(ws))
	if (CHAR('}'))
	if (ANY(ws))
	return 1;
	return 0;
}

RULE(initializer) {
	if (!MATCH(assignment_expression))
	if (!MATCH(initializer_0))
	if (!MATCH(initializer_1))
	return 0;
	return 1;
}

RULE(initializer_list_0) {
	if (MATCH(initializer_list))
	if (ANY(ws))
	if (CHAR(','))
	if (ANY(ws))
	if (MATCH(initializer))
	return 1;
	return 0;
}

RULE(initializer_list) {
	if (!MATCH(initializer))
	if (!MATCH(initializer_list_0))
	return 0;
	return 1;
}

RULE(compound_statement) {
	if (ANY(ws))
	if (CHAR('{'))
	if (ANY(ws))
	if (ANY(declaration))
	if (ANY(statement))
	if (ANY(ws))
	if (CHAR('}'))
	if (ANY(ws))
	return 1;
	return 0;
}

RULE(statement) {
	if (!MATCH(labeled_statement))
	if (!MATCH(expression_statement))
	if (!MATCH(compound_statement))
	if (!MATCH(selection_statement))
	if (!MATCH(iteration_statement))
	if (!MATCH(jump_statement))
	return 0;
	return 1;
}

RULE(labeled_statement_0) {
	if (MATCH(identifier))
	if (ANY(ws))
	if (CHAR(':'))
	if (ANY(ws))
	if (MATCH(statement))
	return 1;
	return 0;
}

RULE(labeled_statement_1) {
	if (ANY(ws))
	if (STRING("case"))
	if (ANY(ws))
	if (MATCH(constant_expression))
	if (ANY(ws))
	if (CHAR(':'))
	if (ANY(ws))
	if (MATCH(statement))
	return 1;
	return 0;
}

RULE(labeled_statement_2) {
	if (ANY(ws))
	if (STRING("default"))
	if (ANY(ws))
	if (CHAR(':'))
	if (ANY(ws))
	if (MATCH(statement))
	return 1;
	return 0;
}

RULE(labeled_statement) {
	if (!MATCH(labeled_statement_0))
	if (!MATCH(labeled_statement_1))
	if (!MATCH(labeled_statement_2))
	return 0;
	return 1;
}

RULE(expression_statement) {
	if (OPT(MATCH(expression)))
	if (ANY(ws))
	if (CHAR(';'))
	if (ANY(ws))
	return 1;
	return 0;
}

RULE(selection_statement_0) {
	if (ANY(ws))
	if (STRING("if"))
	if (ANY(ws))
	if (CHAR('('))
	if (ANY(ws))
	if (MATCH(expression))
	if (ANY(ws))
	if (CHAR(')'))
	if (ANY(ws))
	if (MATCH(statement))
	if (ANY(ws))
	if (STRING("else"))
	if (ANY(ws))
	if (MATCH(statement))
	return 1;
	return 0;
}

RULE(selection_statement_1) {
	if (ANY(ws))
	if (STRING("if"))
	if (ANY(ws))
	if (CHAR('('))
	if (ANY(ws))
	if (MATCH(expression))
	if (ANY(ws))
	if (CHAR(')'))
	if (ANY(ws))
	if (MATCH(statement))
	return 1;
	return 0;
}

RULE(selection_statement_2) {
	if (ANY(ws))
	if (STRING("switch"))
	if (ANY(ws))
	if (CHAR('('))
	if (ANY(ws))
	if (MATCH(expression))
	if (ANY(ws))
	if (CHAR(')'))
	if (ANY(ws))
	if (MATCH(statement))
	return 1;
	return 0;
}

RULE(selection_statement) {
	if (!MATCH(selection_statement_0))
	if (!MATCH(selection_statement_1))
	if (!MATCH(selection_statement_2))
	return 0;
	return 1;
}

RULE(iteration_statement_0) {
	if (ANY(ws))
	if (STRING("while"))
	if (ANY(ws))
	if (CHAR('('))
	if (ANY(ws))
	if (MATCH(expression))
	if (ANY(ws))
	if (CHAR(')'))
	if (ANY(ws))
	if (MATCH(statement))
	return 1;
	return 0;
}

RULE(iteration_statement_1) {
	if (ANY(ws))
	if (STRING("do"))
	if (ANY(ws))
	if (MATCH(statement))
	if (ANY(ws))
	if (STRING("while"))
	if (ANY(ws))
	if (CHAR('('))
	if (ANY(ws))
	if (MATCH(expression))
	if (ANY(ws))
	if (CHAR(')'))
	if (ANY(ws))
	if (CHAR(';'))
	if (ANY(ws))
	return 1;
	return 0;
}

RULE(iteration_statement_2) {
	if (ANY(ws))
	if (STRING("for"))
	if (ANY(ws))
	if (CHAR('('))
	if (ANY(ws))
	if (OPT(MATCH(expression)))
	if (ANY(ws))
	if (CHAR(';'))
	if (ANY(ws))
	if (OPT(MATCH(expression)))
	if (ANY(ws))
	if (CHAR(';'))
	if (ANY(ws))
	if (OPT(MATCH(expression)))
	if (ANY(ws))
	if (CHAR(')'))
	if (ANY(ws))
	if (MATCH(statement))
	return 1;
	return 0;
}

RULE(iteration_statement) {
	if (!MATCH(iteration_statement_0))
	if (!MATCH(iteration_statement_1))
	if (!MATCH(iteration_statement_2))
	return 0;
	return 1;
}

RULE(jump_statement_0) {
	if (ANY(ws))
	if (STRING("goto"))
	if (ANY(ws))
	if (MATCH(identifier))
	if (ANY(ws))
	if (CHAR(';'))
	if (ANY(ws))
	return 1;
	return 0;
}

RULE(jump_statement_1) {
	if (ANY(ws))
	if (STRING("continue"))
	if (ANY(ws))
	if (CHAR(';'))
	if (ANY(ws))
	return 1;
	return 0;
}

RULE(jump_statement_2) {
	if (ANY(ws))
	if (STRING("break"))
	if (ANY(ws))
	if (CHAR(';'))
	if (ANY(ws))
	return 1;
	return 0;
}

RULE(jump_statement_3) {
	if (ANY(ws))
	if (STRING("return"))
	if (ANY(ws))
	if (OPT(MATCH(expression)))
	if (ANY(ws))
	if (CHAR(';'))
	if (ANY(ws))
	return 1;
	return 0;
}

RULE(jump_statement) {
	if (!MATCH(jump_statement_0))
	if (!MATCH(jump_statement_1))
	if (!MATCH(jump_statement_2))
	if (!MATCH(jump_statement_3))
	return 0;
	return 1;
}

RULE(identifier) {
	// TODO
	return 0;
}

RULE(character_constant) {
	// TODO
	return 0;
}

RULE(enumeration_constant) {
	// TODO
	return 0;
}

RULE(floating_constant) {
	// TODO
	return 0;
}

RULE(integer_constant) {
	// TODO
	return 0;
}

RULE(string_character_escaped) {
	// TODO
	return 0;
}

RULE(string_character_unescaped) {
	// TODO
	return 0;
}

RULE(string_character) {
	if (!MATCH(string_character_unescaped))
	if (!MATCH(string_character_escaped))
	return 0;
	return 1;
}

RULE(string) {
	if (ANY(ws))
	if (CHAR('"'))
	if (ANY(string_character))
	if (CHAR('"'))
	if (ANY(ws))
	return 1;
	return 0;
}

RULE(ws) {
	if (SET(' ', '\t', '\r', '\n'))
	return 1;
	return 0;
}

