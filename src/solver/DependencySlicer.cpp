#include "DependencySlicer.h"

#include <glog/logging.h>
#include <glog/vlog_is_on.h>
#include <cstdbool>
#include <iostream>
#include <iterator>
#include <string>
#include <utility>

#include "smt/ast.h"
#include "smt/Visitor.h"
#include "utils/List.h"

namespace Vlab {
namespace Solver {

using namespace SMT;
const int DependencySlicer::VLOG_LEVEL = 20;

DependencySlicer::DependencySlicer(Script_ptr script, SymbolTable_ptr symbol_table)
    : AstTraverser(script),
      symbol_table_(symbol_table),
      current_term_(nullptr) {
  setCallbacks();
}

DependencySlicer::~DependencySlicer() {
}

void DependencySlicer::start() {
  DVLOG(VLOG_LEVEL) << "Starting the Dependency Slicer";

  symbol_table_->push_scope(root);
  visitScript(root);
  symbol_table_->pop_scope();

  end();
}

void DependencySlicer::end() {
  if (VLOG_IS_ON(VLOG_LEVEL)) {
    for (auto& m : symbol_table_->get_component_map()) {
      if (m.first != root) {
        DVLOG(VLOG_LEVEL) << "Number of components for scope " << *static_cast<Term_ptr>(m.first) << " is " << m.second.size();
      } else {
        DVLOG(VLOG_LEVEL) << "Number of components for scope root :: " << m.second.size();
      }
      for (auto& c : m.second) {
        DVLOG(VLOG_LEVEL) << "Component:: " << c->ToString();
      }
    }
  }
}

void DependencySlicer::setCallbacks() {
  auto term_callback = [this] (Term_ptr term) -> bool {
    switch (term->type()) {
    case Term::Type::TERMCONSTANT: {
      return false;
    }
    default:
      return true;
    }
  };

  auto command_callback = [](Command_ptr command) -> bool {
    if (Command::Type::ASSERT == command->getType()) {
      return true;
    }
    return false;
  };

  setCommandPreCallback(command_callback);
  setTermPreCallback(term_callback);
}

void DependencySlicer::visitAssert(Assert_ptr assert_command) {
  if ((Term::Type::OR not_eq assert_command->term->type()) and (Term::Type::AND not_eq assert_command->term->type())) {
    // There is only one constraint
    symbol_table_->add_component(new Component(assert_command->term));
  } else {
    visit_children_of(assert_command);
  }
}

void DependencySlicer::visitAnd(And_ptr and_term) {
  for (auto& term : *(and_term->term_list)) {
    current_term_ = term;
    visit(term);
    current_term_ = nullptr;
  }

  auto components = GetComponentsFor(and_term->term_list);
  symbol_table_->add_components(components);
  // reset data
  term_variable_map_.clear();
}

void DependencySlicer::visitOr(Or_ptr or_term) {
  for (auto& term : *(or_term->term_list)) {
    symbol_table_->push_scope(term);
    if (Term::Type::AND not_eq term->type()) {
      // a single component, handle here
      symbol_table_->add_component(new Component(term));
    } else {
      visit(term);
    }
    symbol_table_->pop_scope();
  }

  // TODO handle components for a Or term at symbol table level (we use dnf form)
}

void DependencySlicer::visitQualIdentifier(SMT::QualIdentifier_ptr qi_term) {
  Variable_ptr variable = symbol_table_->getVariable(qi_term->getVarName());
  if (not variable->isLocalLetVar()) {
    add_variable_current_term_mapping(variable);
  }
}

void DependencySlicer::add_variable_current_term_mapping(SMT::Variable_ptr variable) {
  term_variable_map_[current_term_].insert(variable);
}

std::vector<Component_ptr> DependencySlicer::GetComponentsFor(SMT::TermList_ptr term_list) {
  std::map<Term_ptr, Component_ptr> term_to_component_map;
  Component_ptr current_component = nullptr;
  for (auto it = term_list->begin(); it != term_list->end(); ++it) {
    auto term = *it;
    // find or initialize component that includes term
    auto map_entry = term_to_component_map.find(term);
    if (map_entry == term_to_component_map.end()) {
      current_component = new Component(term);
      term_to_component_map[term] = current_component;
    } else {
      current_component = map_entry->second;
    }
    // check if current term has a shared variable with other terms
    for (auto test_it = it + 1; test_it != term_list->end(); ++test_it) {
      auto other_term = *test_it;
      if (Util::List::has_intersection(term_variable_map_[term].begin(), term_variable_map_[term].end(), term_variable_map_[other_term].begin(), term_variable_map_[other_term].end())) {
        current_component->add_term(other_term);
        term_to_component_map[other_term] = current_component;
      }
    }
  }

  std::set<Component_ptr> unique_components;
  for (auto& entry : term_to_component_map) {
    unique_components.insert(entry.second);
  }

  std::vector<Component_ptr> result (unique_components.begin(), unique_components.end());
  return result;
}

}  //Vlab
}  //Solver