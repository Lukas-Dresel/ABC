/*
 * ConstraintSolver.h
 *
 *  Created on: Jun 24, 2015
 *      Author: baki
 */

#ifndef SOLVER_CONSTRAINTSOLVER_H_
#define SOLVER_CONSTRAINTSOLVER_H_

#include <cstdbool>
#include <map>
#include <vector>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>

#include <glog/logging.h>

#include "../smt/typedefs.h"
#include "../smt/ast.h"
#include "../smt/Visitor.h"
#include "../theory/ArithmeticFormula.h"
#include "../theory/BinaryIntAutomaton.h"
#include "../theory/IntAutomaton.h"
#include "../theory/StringAutomaton.h"
#include "../theory/UnaryAutomaton.h"
#include "../options/Solver.h"
#include "Ast2Dot.h"
#include "VariableValueComputer.h"
#include "ArithmeticConstraintSolver.h"
#include "ConstraintInformation.h"
#include "SymbolTable.h"
#include "Value.h"
#include "StringConstraintSolver.h"


namespace Vlab {
namespace Solver {

class ConstraintSolver: public SMT::Visitor {
  typedef std::map<SMT::Term_ptr, Value_ptr> TermValueMap;
  typedef std::vector<std::vector<SMT::Term_ptr>> VariablePathTable;
 public:
  ConstraintSolver(SMT::Script_ptr, SymbolTable_ptr, ConstraintInformation_ptr);
  virtual ~ConstraintSolver();

  void start() override;
  void start(int iteration_count);
  void end() override;

  void visitScript(SMT::Script_ptr) override;
  void visitCommand(SMT::Command_ptr) override;
  void visitAssert(SMT::Assert_ptr) override;
  void visitTerm(SMT::Term_ptr) override;
  void visitExclamation(SMT::Exclamation_ptr) override;
  void visitExists(SMT::Exists_ptr) override;
  void visitForAll(SMT::ForAll_ptr) override;
  void visitLet(SMT::Let_ptr) override;
  void visitAnd(SMT::And_ptr) override;
  void visitOr(SMT::Or_ptr) override;
  void visitNot(SMT::Not_ptr) override;
  void visitUMinus(SMT::UMinus_ptr) override;
  void visitMinus(SMT::Minus_ptr) override;
  void visitPlus(SMT::Plus_ptr) override;
  void visitTimes(SMT::Times_ptr) override;
  void visitEq(SMT::Eq_ptr) override;
  void visitNotEq(SMT::NotEq_ptr) override;
  void visitGt(SMT::Gt_ptr) override;
  void visitGe(SMT::Ge_ptr) override;
  void visitLt(SMT::Lt_ptr) override;
  void visitLe(SMT::Le_ptr) override;
  void visitConcat(SMT::Concat_ptr) override;
  void visitIn(SMT::In_ptr) override;
  void visitNotIn(SMT::NotIn_ptr) override;
  void visitLen(SMT::Len_ptr) override;
  void visitContains(SMT::Contains_ptr) override;
  void visitNotContains(SMT::NotContains_ptr) override;
  void visitBegins(SMT::Begins_ptr) override;
  void visitNotBegins(SMT::NotBegins_ptr) override;
  void visitEnds(SMT::Ends_ptr) override;
  void visitNotEnds(SMT::NotEnds_ptr) override;
  void visitIndexOf(SMT::IndexOf_ptr) override;
  void visitLastIndexOf(SMT::LastIndexOf_ptr) override;
  void visitCharAt(SMT::CharAt_ptr) override;
  void visitSubString(SMT::SubString_ptr) override;
  void visitToUpper(SMT::ToUpper_ptr) override;
  void visitToLower(SMT::ToLower_ptr) override;
  void visitTrim(SMT::Trim_ptr) override;
  void visitToString(SMT::ToString_ptr) override;
  void visitToInt(SMT::ToInt_ptr) override;
  void visitReplace(SMT::Replace_ptr) override;
  void visitCount(SMT::Count_ptr) override;
  void visitIte(SMT::Ite_ptr) override;
  void visitReConcat(SMT::ReConcat_ptr) override;
  void visitReUnion(SMT::ReUnion_ptr) override;
  void visitReInter(SMT::ReInter_ptr) override;
  void visitReStar(SMT::ReStar_ptr) override;
  void visitRePlus(SMT::RePlus_ptr) override;
  void visitReOpt(SMT::ReOpt_ptr) override;
  void visitToRegex(SMT::ToRegex_ptr) override;
  void visitUnknownTerm(SMT::Unknown_ptr) override;
  void visitAsQualIdentifier(SMT::AsQualIdentifier_ptr) override;
  void visitQualIdentifier(SMT::QualIdentifier_ptr) override;
  void visitTermConstant(SMT::TermConstant_ptr) override;
  void visitSort(SMT::Sort_ptr) override;
  void visitTVariable(SMT::TVariable_ptr) override;
  void visitTBool(SMT::TBool_ptr) override;
  void visitTInt(SMT::TInt_ptr) override;
  void visitTString(SMT::TString_ptr) override;
  void visitAttribute(SMT::Attribute_ptr) override;
  void visitSortedVar(SMT::SortedVar_ptr) override;
  void visitVarBinding(SMT::VarBinding_ptr) override;
  void visitIdentifier(SMT::Identifier_ptr) override;
  void visitPrimitive(SMT::Primitive_ptr) override;
  void visitVariable(SMT::Variable_ptr) override;

 protected:
  Value_ptr getTermValue(SMT::Term_ptr term);
  bool setTermValue(SMT::Term_ptr term, Value_ptr value);
  void clearTermValue(SMT::Term_ptr term);
  void clearTermValuesAndLocalLetVars();
  void setVariablePath(SMT::QualIdentifier_ptr qi_term);
  void update_variables();
  void visit_children_of(SMT::Term_ptr term);
  bool check_and_visit(SMT::Term_ptr term);
  void process_mixed_integer_string_constraints_in(SMT::Term_ptr term);

  bool still_sat_;
  int iteration_count_;
  SMT::Script_ptr root_;
  SymbolTable_ptr symbol_table_;
  ConstraintInformation_ptr constraint_information_;

  ArithmeticConstraintSolver arithmetic_constraint_solver_;
  StringConstraintSolver string_constraint_solver_;

  TermValueMap term_values_;

  std::vector<SMT::Term_ptr> path_trace_;
  VariablePathTable variable_path_table_;

  // for relational variables that need to be updated
  std::vector<SMT::Variable_ptr> tagged_variables;
 private:
  static const int VLOG_LEVEL;
};

} /* namespace Solver */
} /* namespace Vlab */

#endif /* SOLVER_CONSTRAINTSOLVER_H_ */
