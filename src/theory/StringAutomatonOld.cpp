/*
 * StringAutomaton.cpp
 *
 *  Created on: Jun 24, 2015
 *      Author: baki
 */

#include "StringAutomaton.h"

namespace Vlab {
namespace Theory {

const int StringAutomaton::VLOG_LEVEL = 8;

int StringAutomaton::name_counter = 0;

int StringAutomaton::DEFAULT_NUM_OF_VARIABLES = 8;

StringAutomaton::StringAutomaton(DFA_ptr dfa)
        : Automaton(Automaton::Type::STRING, dfa, StringAutomaton::DEFAULT_NUM_OF_VARIABLES), formula_{nullptr} {
}

StringAutomaton::StringAutomaton(DFA_ptr dfa, const int number_of_bdd_variables)
        : Automaton(Automaton::Type::STRING, dfa, number_of_bdd_variables), formula_{nullptr} {
}

StringAutomaton::StringAutomaton(const StringAutomaton& other)
        : Automaton(other), formula_{other.formula_} {
}

StringAutomaton::~StringAutomaton() {
  delete formula_;
}

StringAutomaton_ptr StringAutomaton::clone() const {
  StringAutomaton_ptr cloned_auto = new StringAutomaton(*this);
  DVLOG(VLOG_LEVEL) << cloned_auto->id_ << " = [" << this->id_ << "]->clone()";
  return cloned_auto;
}

StringAutomaton_ptr StringAutomaton::MakePhi(const int number_of_bdd_variables) {
  DFA_ptr non_accepting_string_dfa = Automaton::DFAMakePhi(number_of_bdd_variables);
  StringAutomaton_ptr non_accepting_string_auto = new StringAutomaton(non_accepting_string_dfa, number_of_bdd_variables);
  DVLOG(VLOG_LEVEL) << non_accepting_string_auto->id_ << " = MakePhi(" << number_of_bdd_variables << ")";
  return non_accepting_string_auto;
}

StringAutomaton_ptr StringAutomaton::MakeEmptyString(const int number_of_bdd_variables) {
  DFA_ptr empty_string_dfa = Automaton::DFAMakeEmpty(number_of_bdd_variables);
  StringAutomaton_ptr empty_string_auto = new StringAutomaton(empty_string_dfa, number_of_bdd_variables);
  DVLOG(VLOG_LEVEL) << empty_string_auto->id_ << " = MakeEmptyString(" << number_of_bdd_variables << ")";
  return empty_string_auto;
}

StringAutomaton_ptr StringAutomaton::MakeString(const std::string str, const int number_of_bdd_variables) {
  if (str.empty()) {
    return StringAutomaton::MakeEmptyString();
  }

  const int str_length = str.length();
  const int number_of_states = str_length + 2;
  char* statuses = new char[number_of_states];

  dfaSetup(number_of_states, number_of_bdd_variables, GetBddVariableIndices(number_of_bdd_variables));

  for (int i = 0; i < str_length; i++) {
    dfaAllocExceptions(1);
    dfaStoreException(i + 1, const_cast<char*>(GetBinaryStringMSB((unsigned long) str[i], number_of_bdd_variables).data()));
    dfaStoreState(str_length + 1);
    statuses[i] = '-';
  }

  dfaAllocExceptions(0);
  dfaStoreState(str_length + 1);
  statuses[str_length] = '+';

  //sink state
  dfaAllocExceptions(0);
  dfaStoreState(str_length + 1);
  statuses[str_length + 1] = '-';

  DFA_ptr result_dfa = dfaBuild(statuses);
  StringAutomaton_ptr result_auto = new StringAutomaton(result_dfa, number_of_bdd_variables);
  delete[] statuses;

  DVLOG(VLOG_LEVEL) << result_auto->id_ << " = MakeString(\"" << str << "\")";

  return result_auto;
}

StringAutomaton_ptr StringAutomaton::MakeAnyString(const int number_of_bdd_variables) {
  char statuses[2] { '+', '-' };
  int *variable_indices = GetBddVariableIndices(number_of_bdd_variables);
  dfaSetup(1, number_of_bdd_variables, variable_indices);
  dfaAllocExceptions(0);
  dfaStoreState(0);
  DFA_ptr any_string_dfa = dfaBuild(statuses);
  StringAutomaton_ptr any_string = new StringAutomaton(any_string_dfa, number_of_bdd_variables);
  DVLOG(VLOG_LEVEL) << any_string->id_ << " = MakeAnyString()";
  return any_string;
}

StringAutomaton_ptr StringAutomaton::MakeAnyOtherString(const std::string str, const int num_of_variables) {
  StringAutomaton_ptr str_auto = MakeString(str);
  StringAutomaton_ptr not_contains_me_auto = str_auto->getAnyStringNotContainsMe();
  delete str_auto; str_auto = nullptr;
  DVLOG(VLOG_LEVEL) << not_contains_me_auto->id_ << " = StringAutomaton::MakeAnyOtherString(" << str << ")";
  return not_contains_me_auto;
}

StringAutomaton_ptr StringAutomaton::MakeCharRange(const char from, const char to, const int number_of_bdd_variables) {
  unsigned long from_char = (unsigned long) from;
  unsigned long to_char = (unsigned long) to;
  if (from_char > to_char) {
    std::swap(from_char, to_char);
  }

  char statuses[3] { '-', '+', '-' };
  int* variable_indices = GetBddVariableIndices(number_of_bdd_variables);

  dfaSetup(3, number_of_bdd_variables, variable_indices);

  int initial_state = to_char - from_char;

  //state 0
  dfaAllocExceptions(initial_state + 1);
  for (unsigned long index = from_char; index <= to_char; index++) {
    std::vector<char> v = GetBinaryFormat(index,number_of_bdd_variables);
    dfaStoreException(1, const_cast<char*>(GetBinaryStringMSB(index, number_of_bdd_variables).data()));
  }
  dfaStoreState(2);

  //state 1
  dfaAllocExceptions(0);
  dfaStoreState(2);

  //state 2
  dfaAllocExceptions(0);
  dfaStoreState(2);

  DFA_ptr range_dfa = dfaBuild(statuses);
  StringAutomaton_ptr range_auto = new StringAutomaton(range_dfa, number_of_bdd_variables);

  DVLOG(VLOG_LEVEL) << range_auto->id_ << " = MakeCharRange('" << from << "', '" << to << "')";

  return range_auto;
}

StringAutomaton_ptr StringAutomaton::MakeAnyChar(const int number_of_bdd_variables) {
  StringAutomaton_ptr any_char_auto = StringAutomaton::MakeAnyStringLengthEqualTo(1, number_of_bdd_variables);
  DVLOG(VLOG_LEVEL) << any_char_auto->id_ << " = MakeAnyChar()";
  return any_char_auto;
}

StringAutomaton_ptr StringAutomaton::MakeRegexAuto(const std::string regex, const int number_of_bdd_variables) {
  Util::RegularExpression regular_expression (regex);
  StringAutomaton_ptr regex_auto = StringAutomaton::MakeRegexAuto(&regular_expression, number_of_bdd_variables);
  DVLOG(VLOG_LEVEL) << regex_auto->id_ << " = MakeRegexAuto(" << regex << ")";

  return regex_auto;
}

StringAutomaton_ptr StringAutomaton::MakeRegexAuto(Util::RegularExpression_ptr regular_expression, const int number_of_bdd_variables) {
  Automaton_ptr regex_auto = nullptr;
  StringAutomaton_ptr regex_expr1_auto = nullptr;
  StringAutomaton_ptr regex_expr2_auto = nullptr;

  switch (regular_expression->type()) {
  case Util::RegularExpression::Type::UNION:
    regex_expr1_auto = StringAutomaton::MakeRegexAuto(regular_expression->get_expr1(), number_of_bdd_variables);
    regex_expr2_auto = StringAutomaton::MakeRegexAuto(regular_expression->get_expr2(), number_of_bdd_variables);
    regex_auto = regex_expr1_auto->Union(regex_expr2_auto);
    delete regex_expr1_auto;
    delete regex_expr2_auto;
    break;
  case Util::RegularExpression::Type::CONCATENATION:
    regex_expr1_auto = StringAutomaton::MakeRegexAuto(regular_expression->get_expr1(), number_of_bdd_variables);
    regex_expr2_auto = StringAutomaton::MakeRegexAuto(regular_expression->get_expr2(), number_of_bdd_variables);
    regex_auto = regex_expr1_auto->Concat(regex_expr2_auto);
    delete regex_expr1_auto;
    delete regex_expr2_auto;
    break;
  case Util::RegularExpression::Type::INTERSECTION:
    regex_expr1_auto = StringAutomaton::MakeRegexAuto(regular_expression->get_expr1(), number_of_bdd_variables);
    regex_expr2_auto = StringAutomaton::MakeRegexAuto(regular_expression->get_expr2(), number_of_bdd_variables);
    regex_auto = regex_expr1_auto->Intersect(regex_expr2_auto);
    delete regex_expr1_auto;
    delete regex_expr2_auto;
    break;
  case Util::RegularExpression::Type::OPTIONAL:
    regex_expr1_auto = StringAutomaton::MakeRegexAuto(regular_expression->get_expr1(), number_of_bdd_variables);
    regex_auto = regex_expr1_auto->optional();
    delete regex_expr1_auto;
    break;
  case Util::RegularExpression::Type::REPEAT_STAR:
    regex_expr1_auto = StringAutomaton::MakeRegexAuto(regular_expression->get_expr1(), number_of_bdd_variables);
    regex_auto = regex_expr1_auto->kleeneClosure();
    delete regex_expr1_auto;
    break;
  case Util::RegularExpression::Type::REPEAT_PLUS:
    regex_expr1_auto = StringAutomaton::MakeRegexAuto(regular_expression->get_expr1(), number_of_bdd_variables);
    regex_auto = regex_expr1_auto->closure();
    delete regex_expr1_auto;
    break;
  case Util::RegularExpression::Type::REPEAT_MIN:
    regex_expr1_auto = StringAutomaton::MakeRegexAuto(regular_expression->get_expr1(), number_of_bdd_variables);
    regex_auto = regex_expr1_auto->repeat(regular_expression->get_min());
    delete regex_expr1_auto;
    break;
  case Util::RegularExpression::Type::REPEAT_MINMAX:
    regex_expr1_auto = StringAutomaton::MakeRegexAuto(regular_expression->get_expr1(), number_of_bdd_variables);
    regex_auto = regex_expr1_auto->repeat(regular_expression->get_min(), regular_expression->get_max());
    delete regex_expr1_auto;
    break;
  case Util::RegularExpression::Type::COMPLEMENT:
    regex_expr1_auto = StringAutomaton::MakeRegexAuto(regular_expression->get_expr1(), number_of_bdd_variables);
    regex_auto = regex_expr1_auto->Complement();
    delete regex_expr1_auto;
    break;
  case Util::RegularExpression::Type::CHAR:
    regex_auto = StringAutomaton::MakeString(std::string(1, regular_expression->get_character()), number_of_bdd_variables);
    break;
  case Util::RegularExpression::Type::CHAR_RANGE:
    regex_auto = StringAutomaton::MakeCharRange(regular_expression->get_from_character(), regular_expression->get_to_character(), number_of_bdd_variables);
    break;
  case Util::RegularExpression::Type::ANYCHAR:
    regex_auto = StringAutomaton::MakeAnyChar(number_of_bdd_variables);
    break;
  case Util::RegularExpression::Type::EMPTY:
    regex_auto = StringAutomaton::MakePhi(number_of_bdd_variables);
    break;
  case Util::RegularExpression::Type::STRING:
    regex_auto = StringAutomaton::MakeString(regular_expression->get_string(), number_of_bdd_variables);
    break;
  case Util::RegularExpression::Type::ANYSTRING:
    regex_auto = StringAutomaton::MakeAnyString(number_of_bdd_variables);
    break;
  case Util::RegularExpression::Type::AUTOMATON:
    LOG(FATAL)<< "Unsupported regular expression" << *regular_expression;
    break;
  case Util::RegularExpression::Type::INTERVAL:
    LOG(FATAL) << "Unsupported regular expression" << *regular_expression;
    break;
  default:
    LOG(FATAL) << "Unsupported regular expression" << *regular_expression;
    break;
  }

  StringAutomaton_ptr ret_auto = static_cast<StringAutomaton_ptr>(regex_auto);
  return ret_auto;
}

StringAutomaton_ptr StringAutomaton::MakeAnyStringLengthEqualTo(const int length, const int number_of_bdd_variables) {
  DFA_ptr length_dfa = Automaton::DFAMakeAcceptingAnyWithInRange(length, length, number_of_bdd_variables);
  StringAutomaton_ptr length_auto = new StringAutomaton(length_dfa, number_of_bdd_variables);
  DVLOG(VLOG_LEVEL) << length_auto->id_ << " = MakeAnyStringLengthEqualTo(" << length <<  ")";
  return length_auto;
}

StringAutomaton_ptr StringAutomaton::MakeAnyStringLengthLessThan(const int length, const int number_of_bdd_variables){
  DFA_ptr length_dfa = Automaton::DFAMakeAcceptingAnyWithInRange(0, length-1, number_of_bdd_variables);
  StringAutomaton_ptr length_auto = new StringAutomaton(length_dfa, number_of_bdd_variables);
  DVLOG(VLOG_LEVEL) << length_auto->id_ << " = MakeAnyStringLengthLessThan(" << length <<  ")";
  return length_auto;
}

StringAutomaton_ptr StringAutomaton::MakeAnyStringLengthLessThanOrEqualTo(const int length, const int number_of_bdd_variables){
  DFA_ptr length_dfa = Automaton::DFAMakeAcceptingAnyWithInRange(0, length, number_of_bdd_variables);
  StringAutomaton_ptr length_auto = new StringAutomaton(length_dfa, number_of_bdd_variables);
  DVLOG(VLOG_LEVEL) << length_auto->id_ << " = MakeAnyStringLengthLessThanOrEqualTo(" << length <<  ")";
  return length_auto;
}

StringAutomaton_ptr StringAutomaton::MakeAnyStringLengthGreaterThan(const int length, const int number_of_bdd_variables) {
  DFA_ptr length_dfa = Automaton::DFAMakeAcceptingAnyAfterLength(length + 1, number_of_bdd_variables);
  StringAutomaton_ptr length_auto = new StringAutomaton(length_dfa, number_of_bdd_variables);
  DVLOG(VLOG_LEVEL) << length_auto->id_ << " = MakeAnyStringLengthGreaterThan(" << length <<  ")";
  return length_auto;
}

StringAutomaton_ptr StringAutomaton::MakeAnyStringLengthGreaterThanOrEqualTo(const int length, const int number_of_bdd_variables) {
  DFA_ptr length_dfa = Automaton::DFAMakeAcceptingAnyAfterLength(length, number_of_bdd_variables);
  StringAutomaton_ptr length_auto = new StringAutomaton(length_dfa, number_of_bdd_variables);
  DVLOG(VLOG_LEVEL) << length_auto->id_ << " = MakeAnyStringLengthGreaterThanOrEqualTo(" << length <<  ")";
  return length_auto;
}

StringAutomaton_ptr StringAutomaton::MakeAnyStringWithLengthInRange(const int start, const int end, const int number_of_bdd_variables) {
  DFA_ptr length_dfa = Automaton::DFAMakeAcceptingAnyWithInRange(start, end, number_of_bdd_variables);
  StringAutomaton_ptr length_auto = new StringAutomaton(length_dfa, number_of_bdd_variables);
  DVLOG(VLOG_LEVEL) << length_auto->id_ << " = MakeAnyStringWithLengthInRange(" << start << "," << end <<  ")";
  return length_auto;
}

StringAutomaton_ptr StringAutomaton::MakeAutomaton(DFA_ptr dfa, const int number_of_variables) {
  return new StringAutomaton(dfa, number_of_variables);
}

/**
 * Initial Re-implementation of  'dfa_concat_extrabit' in LibStranger
 *TODO Fix empty string bug that happens in case (concat /.{0,1}/ /{1,1}/)
 */
//StringAutomaton_ptr StringAutomaton::concat(StringAutomaton_ptr other_auto) {
//  StringAutomaton_ptr left_auto = this, right_auto = other_auto;
//
//  if (left_auto->IsEmptyLanguage() or right_auto->IsEmptyLanguage()) {
//    return StringAutomaton::MakePhi();
//  } else if (left_auto->isEmptyString()) {
//    return right_auto->clone();
//  } else if (right_auto->isEmptyString()) {
//    return left_auto->clone();
//  }
//
//  bool left_hand_side_has_emtpy_string = left_auto->hasEmptyString();
//  bool right_hand_side_has_empty_string = right_auto->hasEmptyString();
//
//  if (left_hand_side_has_emtpy_string or right_hand_side_has_empty_string) {
//    auto any_string_other_than_empty = StringAutomaton::MakeAnyStringLengthGreaterThan(0);
//    if (left_hand_side_has_emtpy_string) {
//      left_auto = left_auto->Intersect(any_string_other_than_empty);
//    }
//
//    if (right_hand_side_has_empty_string) {
//      right_auto = right_auto->Intersect(any_string_other_than_empty);
//    }
//    delete any_string_other_than_empty;
//  }
//
//  int var = left_auto->num_of_bdd_variables_;
//  int* indices = GetBddVariableIndices(var);
//  int tmp_num_of_variables,
//      state_id_shift_amount,
//      expected_num_of_states,
//      sink_state_left_auto,
//      sink_state_right_auto,
//      to_state = 0,
//      loc,
//      i = 0,
//      j = 0;
//
//  bool is_start_state_reachable;
//  paths state_paths = nullptr, pp = nullptr;
//  trace_descr tp = nullptr;
//
//  std::map<std::vector<char>*, int> exceptions_left_auto;
//  std::map<std::vector<char>*, int> exceptions_right_auto;
//  std::map<std::vector<char>*, int> exceptions_fix;
//  std::vector<char>* current_exception = nullptr;
//  char* statuses = nullptr;
//  tmp_num_of_variables = left_auto->num_of_bdd_variables_ + 1; // add one extra bit
//  state_id_shift_amount = left_auto->dfa_->ns;
//  expected_num_of_states = left_auto->dfa_->ns + right_auto->dfa_->ns;
//
//  is_start_state_reachable = right_auto->TEMPisStartStateReachableFromAnAcceptingState();
//  if (not is_start_state_reachable) {
//    expected_num_of_states = expected_num_of_states  - 1; // if start state is reachable from an accepting state, it will be merge with accepting states of left hand side
//  }
//  // variable initializations
//  sink_state_left_auto = left_auto->GetSinkState();
//  sink_state_right_auto = right_auto->GetSinkState();
//
//  bool left_sink = true, right_sink = true;
//  int sink = sink_state_left_auto;
//
//  if(sink_state_left_auto < 0 && sink_state_right_auto < 0) {
//    left_sink = right_sink = false;
//    sink = expected_num_of_states;
//    expected_num_of_states++;
//  } else if(sink_state_left_auto < 0) {
//    left_sink = false;
//    sink = sink_state_right_auto + state_id_shift_amount;
//    if(not is_start_state_reachable) {
//      sink--;
//    }
//  } else if(sink_state_right_auto < 0) {
//    right_sink = false;
//  } else {
//    expected_num_of_states--;
//  }
//
//  statuses = new char[expected_num_of_states + 1];
//  int* concat_indices = GetBddVariableIndices(tmp_num_of_variables);
//
//  dfaSetup(expected_num_of_states, tmp_num_of_variables, concat_indices); //sink states are merged
//  state_paths = pp = make_paths(right_auto->dfa_->bddm, right_auto->dfa_->q[right_auto->dfa_->s]);
//  while (pp) {
//    if (!right_sink || pp->to != sink_state_right_auto ) {
//      to_state = pp->to + state_id_shift_amount;
//      // if there is a self loop keep it
//      if (pp->to == (unsigned)right_auto->dfa_->s ) {
//        to_state -= 2;
//      } else {
//        if (left_sink && right_sink && pp->to > (unsigned)sink_state_right_auto ) {
//          to_state--; //to new state, sink state will be eliminated and hence need -1
//        }
//        if ((not is_start_state_reachable) && pp->to > (unsigned)right_auto->dfa_->s) {
//          to_state--; // to new state, init state will be eliminated if init is not reachable
//        }
//      }
//
//      current_exception = new std::vector<char>();
//      for (j = 0; j < right_auto->num_of_bdd_variables_; j++) {
//        //the following for loop can be avoided if the indices are in order
//        for (tp = pp->trace; tp && (tp->index != (unsigned)indices[j]); tp = tp->next);
//        if (tp) {
//          if (tp->value) {
//            current_exception->push_back('1');
//          }
//          else {
//            current_exception->push_back('0');
//          }
//        }
//        else {
//          current_exception->push_back('X');
//        }
//      }
//
//      current_exception->push_back('1'); // new path
//      current_exception->push_back('\0');
//      exceptions_right_auto[current_exception] = to_state;
//    }
//    current_exception = nullptr;
//    tp = nullptr;
//    pp = pp->next;
//  }
//
//  kill_paths(state_paths);
//  state_paths = pp = nullptr;
//
//  for (i = 0; i < left_auto->dfa_->ns; i++) {
//    state_paths = pp = make_paths(left_auto->dfa_->bddm, left_auto->dfa_->q[i]);
//    while (pp) {
//      if (left_sink && pp->to == (unsigned)sink_state_left_auto) {
//        pp = pp->next;
//        continue;
//      }
//      to_state = pp->to;
//      current_exception = new std::vector<char>();
//      for (j = 0; j < left_auto->num_of_bdd_variables_; j++) {
//        for (tp = pp->trace; tp && (tp->index != (unsigned)indices[j]); tp = tp->next);
//        if (tp) {
//          if (tp->value) {
//            current_exception->push_back('1');
//          } else {
//            current_exception->push_back('0');
//          }
//        } else {
//          current_exception->push_back('X');
//        }
//      }
//
//      current_exception->push_back('0'); // add extra bit, '0' is used for the exceptions coming from left auto
//      current_exception->push_back('\0');
//      exceptions_left_auto[current_exception] = to_state;
//      tp = nullptr;
//      pp = pp->next;
//    }
//    current_exception = nullptr;
//    // generate concat automaton
//    if (left_auto->IsAcceptingState(i)) {
//      dfaAllocExceptions(exceptions_left_auto.size() + exceptions_right_auto.size());
//      for (auto it = exceptions_left_auto.begin(); it != exceptions_left_auto.end();) {
//        dfaStoreException(it->second, &*it->first->begin());
//        current_exception = it->first;
//        it = exceptions_left_auto.erase(it);
//        delete current_exception;
//      }
//      for (auto it = exceptions_right_auto.begin(); it != exceptions_right_auto.end();) {
//        dfaStoreException(it->second, &*it->first->begin());
//        current_exception = it->first;
//        it = exceptions_right_auto.erase(it);
//        delete current_exception;
//      }
//
//      dfaStoreState(sink);
//      if (right_auto->IsAcceptingState(0)) {
//        statuses[i]='+';
//      }
//      else {
//        statuses[i]='-';
//      }
//    } else {
//      dfaAllocExceptions(exceptions_left_auto.size());
//      for (auto it = exceptions_left_auto.begin(); it != exceptions_left_auto.end();) {
//        dfaStoreException(it->second, &*it->first->begin());
//        current_exception = it->first;
//        it = exceptions_left_auto.erase(it);
//        delete current_exception;
//      }
//      dfaStoreState(sink);
//      statuses[i] = '-';
//    }
//    current_exception = nullptr;
//    kill_paths(state_paths);
//    state_paths = pp = nullptr;
//  }
//
//  //  initflag is 1 iff init is reached by some state. In this case,
//  for (i = 0; i < right_auto->dfa_->ns; i++) {
//    if (i != sink_state_right_auto ) {
//      if ( i != right_auto->dfa_->s || is_start_state_reachable) {
//        state_paths = pp = make_paths(right_auto->dfa_->bddm, right_auto->dfa_->q[i]);
//        while (pp) {
//          if (!right_sink || pp->to != (unsigned)sink_state_right_auto) {
//            to_state = pp->to + state_id_shift_amount;
//
//            if ( right_sink && left_sink && pp->to > (unsigned)sink_state_right_auto) {
//              to_state--; //to new state, sink state will be eliminated and hence need -1
//            }
//
//            if ( (not is_start_state_reachable) && pp->to > (unsigned)right_auto->dfa_->s) {
//              to_state--; // to new state, init state will be eliminated if init is not reachable
//            }
//
//            current_exception = new std::vector<char>();
//            for (j = 0; j < var; j++) {
//              for (tp = pp->trace; tp && (tp->index != (unsigned)indices[j]); tp =tp->next);
//              if (tp) {
//                if (tp->value){
//                  current_exception->push_back('1');
//                }
//                else {
//                  current_exception->push_back('0');
//                }
//              }
//              else {
//                current_exception->push_back('X');
//              }
//            }
//            current_exception->push_back('0'); // old value
//            current_exception->push_back('\0');
//            exceptions_fix[current_exception] = to_state;
//            tp = nullptr;
//            current_exception = nullptr;
//          }
//          pp = pp->next;
//        }
//
//        dfaAllocExceptions(exceptions_fix.size());
//        for (auto it = exceptions_fix.begin(); it != exceptions_fix.end();) {
//          dfaStoreException(it->second, &*it->first->begin());
//          current_exception = it->first;
//          it = exceptions_fix.erase(it);
//          delete current_exception;
//        }
//
//        dfaStoreState(sink);
//
//        loc = state_id_shift_amount + i;
//        if ( (not is_start_state_reachable) && i > right_auto->dfa_->s) {
//          loc--;
//        }
//        if (left_sink && right_sink && i > sink_state_right_auto) {
//          loc--;
//        }
//
//        if ( right_auto->IsAcceptingState(i)) {
//          statuses[loc]='+';
//        } else {
//          statuses[loc]='-';
//        }
//
//        kill_paths(state_paths);
//        state_paths = pp = nullptr;
//      }
//    } else if(!left_sink && right_sink) {
//      dfaAllocExceptions(0);
//      dfaStoreState(sink);
//      statuses[sink] = '-';
//    }
//  }
//
//  if(!right_sink && !left_sink) {
//    dfaAllocExceptions(0);
//    dfaStoreState(sink);
//    statuses[sink] = '-';
//  }
//
//  statuses[expected_num_of_states]='\0';
//
//  DFA_ptr concat_dfa = dfaBuild(statuses);
//  delete[] statuses; statuses = nullptr;
//  delete[] concat_indices; concat_indices = nullptr;
//  DFA_ptr tmp_dfa = dfaProject(concat_dfa, (unsigned) var);
//  dfaFree(concat_dfa);
//  concat_dfa = dfaMinimize(tmp_dfa);
//  dfaFree(tmp_dfa); tmp_dfa = nullptr;
//
//  auto concat_auto = new StringAutomaton(concat_dfa, num_of_bdd_variables_);
//
//  if (left_hand_side_has_emtpy_string) {
//    auto tmp_auto = concat_auto;
//    concat_auto = tmp_auto->Union(other_auto);
//    delete tmp_auto;
//    delete left_auto; left_auto = nullptr;
//  }
//
//  if (right_hand_side_has_empty_string) {
//    auto tmp_auto = concat_auto;
//    concat_auto = tmp_auto->Union(this);
//    delete tmp_auto;
//    delete right_auto; right_auto = nullptr;
//  }
//
//  DVLOG(VLOG_LEVEL) << concat_auto->id_ << " = [" << this->id_ << "]->concat(" << other_auto->id_ << ")";
//
//  return concat_auto;
//
//}

StringAutomaton_ptr StringAutomaton::optional() {
  StringAutomaton_ptr optional_auto = nullptr, empty_string = nullptr;

  empty_string = StringAutomaton::MakeEmptyString();
  optional_auto = static_cast<StringAutomaton_ptr>(this->Union(empty_string));
  delete empty_string;

  DVLOG(VLOG_LEVEL) << optional_auto->id_ << " = [" << this->id_ << "]->optional()";

  return optional_auto;
}

/**
 * TODO improve implementation by refactoring libstranger call
 *
 */
StringAutomaton_ptr StringAutomaton::closure() {
  StringAutomaton_ptr result_auto = nullptr;
  DFA_ptr result_dfa = nullptr, temp_dfa = nullptr;
  paths state_paths, pp;
  trace_descr tp;
  int sink = GetSinkState();
  CHECK_GT(sink,-1);
  int var = DEFAULT_NUM_OF_VARIABLES;
  int len = var + 1; //one extra bit
  int *indices = GetBddVariableIndices(len);
  char *statuses = new char[dfa_->ns+1];
  std::vector<std::pair<int,std::vector<char>>> added_exeps, original_exeps;
  std::vector<char> exep;

  dfaSetup(dfa_->ns, len, indices);
  //construct the added paths
  state_paths = pp = make_paths(dfa_->bddm, dfa_->q[dfa_->s]);
  exep = std::vector<char>(len,'X');
  exep.push_back('\0');
  while (pp) {
    if (pp->to != sink) {
      for (int j = 0; j < var; j++) {
        //the following for loop can be avoided if the indices are in order
        for (tp = pp->trace; tp && (tp->index != indices[j]); tp = tp->next);
        if (tp) {
          if (tp->value)
            exep[j] = '1';
          else
            exep[j] = '0';
        } else
          exep[j] = 'X';
      }
      exep[len-1] = '1'; //new path
      added_exeps.push_back(std::make_pair(pp->to,exep));
    }
    pp = pp->next;
  }
  kill_paths(state_paths);

  exep = std::vector<char>(len,'X');
  exep.push_back('\0');
  for (int i = 0; i < dfa_->ns; i++) {
    state_paths = pp = make_paths(dfa_->bddm, dfa_->q[i]);
    while (pp) {
      if (pp->to != sink) {
        for (int j = 0; j < var; j++) {
          //the following for loop can be avoided if the indices are in order
          for (tp = pp->trace; tp && (tp->index != indices[j]); tp = tp->next);

          if (tp) {
            if (tp->value)
              exep[j] = '1';
            else
              exep[j] = '0';
          } else
            exep[j] = 'X';
        }
        exep[len-1] = '0'; //old value
        original_exeps.push_back(std::make_pair(pp->to,exep));
      }
      pp = pp->next;
    }
    if (dfa_->f[i] == 1) { //add added paths
      dfaAllocExceptions(added_exeps.size() + original_exeps.size());
      for(int k = 0; k < added_exeps.size(); k++) {
        dfaStoreException(added_exeps[k].first,&added_exeps[k].second[0]);
      }
      for(int k = 0; k < original_exeps.size(); k++) {
        dfaStoreException(original_exeps[k].first,&original_exeps[k].second[0]);
      }
      statuses[i] = '+';
    } else {
      dfaAllocExceptions(original_exeps.size());
      for(int k = 0; k < original_exeps.size(); k++) {
        dfaStoreException(original_exeps[k].first,&original_exeps[k].second[0]);
      }
      if (dfa_->f[i] == -1)
        statuses[i] = '-';
      else
        statuses[i] = '0';
    }
    dfaStoreState(sink);
    kill_paths(state_paths);
    original_exeps.clear();
  }
  statuses[dfa_->ns] = '\0';
  temp_dfa = dfaBuild(statuses);
  result_dfa = dfaProject(temp_dfa, (unsigned) var); //var is the index of the extra bit
  dfaFree(temp_dfa);
  temp_dfa = result_dfa;
  result_dfa = dfaMinimize(temp_dfa);
  dfaFree(temp_dfa);

  result_auto = new StringAutomaton(result_dfa);
  delete[] indices;
  delete[] statuses;
  DVLOG(VLOG_LEVEL) << result_auto->id_ << " = [" << this->id_ << "]->closure()";
  return result_auto;
}

StringAutomaton_ptr StringAutomaton::kleeneClosure() {
  StringAutomaton_ptr kleene_closure_auto = nullptr, closure_auto = nullptr, empty_string = nullptr;

  closure_auto = this->closure();
  empty_string = StringAutomaton::MakeEmptyString();
  kleene_closure_auto = static_cast<StringAutomaton_ptr>(closure_auto->Union(empty_string));
  delete closure_auto;
  delete empty_string;

  DVLOG(VLOG_LEVEL) << kleene_closure_auto->id_ << " = [" << this->id_ << "]->kleeneClosure()";

  return kleene_closure_auto;
}


StringAutomaton_ptr StringAutomaton::repeat(unsigned min) {
  StringAutomaton_ptr repeated_auto = nullptr;

  if (min == 0) {
    repeated_auto = this->kleeneClosure();
  } else if (min == 1) {
    repeated_auto = this->closure();
  } else {
    StringAutomaton_ptr closure_auto = this->closure();
    StringAutomaton_ptr range_auto = StringAutomaton::MakeAnyStringLengthGreaterThanOrEqualTo(min);
    repeated_auto = static_cast<StringAutomaton_ptr>(closure_auto->Intersect(range_auto));
    delete range_auto; range_auto = nullptr;
    delete closure_auto; closure_auto = nullptr;
  }

  DVLOG(VLOG_LEVEL) << repeated_auto->id_ << " = [" << this->id_ << "]->repeat(" << min << ")";

  return repeated_auto;
}

StringAutomaton_ptr StringAutomaton::repeat(unsigned min, unsigned max) {
  StringAutomaton_ptr repeated_auto = nullptr;

  if (min == 0) {
    repeated_auto = this->kleeneClosure();
  } else {
    repeated_auto = this->closure();
  }

  StringAutomaton_ptr range_auto = StringAutomaton::MakeAnyStringWithLengthInRange(min, max);
  StringAutomaton_ptr tmp_auto = repeated_auto;
  repeated_auto = static_cast<StringAutomaton_ptr>(tmp_auto->Intersect(range_auto));
  delete range_auto; range_auto = nullptr;
  delete tmp_auto; tmp_auto = nullptr;

  DVLOG(VLOG_LEVEL) << repeated_auto->id_ << " = [" << this->id_ << "]->repeat(" << min << ", " << max << ")";

  return repeated_auto;
}

StringAutomaton_ptr StringAutomaton::suffixes() {
  StringAutomaton_ptr suffixes_auto = nullptr;

  if (this->IsEmptyLanguage()) {
    suffixes_auto = StringAutomaton::MakePhi();
    DVLOG(VLOG_LEVEL) << suffixes_auto->id_ << " = [" << this->id_ << "]->suffixes()";
    return suffixes_auto;
  }

  int number_of_variables = this->num_of_bdd_variables_,
          number_of_states = this->dfa_->ns,
          sink_state = this->GetSinkState(),
          next_state = -1;

  unsigned max = number_of_states;
  if (sink_state != -1) {
    max = max - 1;
  }

  // if number of variables are too large for mona, implement an algorithm that find suffixes by finding
  // sub suffixes and union them
  number_of_variables = this->num_of_bdd_variables_ + std::ceil(std::log2(max)); // number of variables required
  int* indices = GetBddVariableIndices(number_of_variables);
  char* statuses = new char[number_of_states + 1];
  unsigned extra_bits_value = 0;
  int number_of_extra_bits_needed = number_of_variables - this->num_of_bdd_variables_;

  std::vector<char>* current_exception = nullptr;
  std::map<int, std::map<std::vector<char>*, int>> exception_map;

  paths state_paths = nullptr, pp = nullptr;
  trace_descr tp = nullptr;

  for (int s = 0; s < number_of_states; s++) {
    if (s != sink_state) {
      exception_map[s]; // initialize map entry
      state_paths = pp = make_paths(this->dfa_->bddm, this->dfa_->q[s]);
      while (pp) {
        if (pp->to != (unsigned)sink_state) {
          current_exception = new std::vector<char>();
          for (int j = 0; j < this->num_of_bdd_variables_; j++) {
            for (tp = pp->trace; tp && (tp->index != (unsigned)indices[j]); tp = tp->next);
            if (tp) {
              if (tp->value) {
                current_exception->push_back('1');
              } else {
                current_exception->push_back('0');
              }
            } else {
              current_exception->push_back('X');
            }
          }


          exception_map[s][current_exception] = pp->to;
          current_exception = nullptr;
        }
        tp = nullptr;
        pp = pp->next;
      }

      kill_paths(state_paths);
      state_paths = pp = nullptr;
      // add to start state by adding extra bits
      if (s != this->dfa_->s) {
        ++extra_bits_value;
        auto extra_bit_binary_format = GetBinaryFormat(extra_bits_value, number_of_extra_bits_needed);
        for (auto& exceptions : exception_map[s]) {
          current_exception = new std::vector<char>();
          current_exception->insert(current_exception->begin(), exceptions.first->begin(), exceptions.first->end());
          for (int i = 0; i < number_of_extra_bits_needed; i++) {
            current_exception->push_back(extra_bit_binary_format[i]); // new transitions for start state
            exceptions.first->push_back('0'); // default transitions have all zero's in extrabits
          }
          current_exception->push_back('\0');
          exceptions.first->push_back('\0');
          exception_map[this->dfa_->s][current_exception] = exceptions.second;
          current_exception = nullptr;
        }
      } else {
        // initial state default transitions' extra bits extended with all zeros
        for (auto& exceptions : exception_map[this->dfa_->s]) {
          for (int i = 0; i < number_of_extra_bits_needed; i++) {
            exceptions.first->push_back('0'); // default transitions have all zero's in extrabits
          }
          exceptions.first->push_back('\0');
        }
      }
    }
  }

  dfaSetup(number_of_states, number_of_variables, indices);
  for (int s = 0; s < number_of_states; s++) {
    statuses[s] = '-';
    if (s != sink_state) {
      statuses[s] = '-'; // initially
      dfaAllocExceptions(exception_map[s].size());
      for (auto it = exception_map[s].begin(); it != exception_map[s].end();) {
        dfaStoreException(it->second, &*it->first->begin());
        current_exception = it->first;
        it = exception_map[s].erase(it);
        delete current_exception;
      }
      dfaStoreState(sink_state);
      current_exception = nullptr;
      if (IsAcceptingState(s)) {
        statuses[s] = '+';
      }
    } else {
      dfaAllocExceptions(0);
      dfaStoreState(s);
    }
  }

  statuses[number_of_states] = '\0';
  DFA_ptr result_dfa = dfaBuild(statuses);
  delete[] indices;
  delete[] statuses;
  suffixes_auto = new StringAutomaton(dfaMinimize(result_dfa), number_of_variables);
  dfaFree(result_dfa); result_dfa = nullptr;

  while (number_of_extra_bits_needed > 0) {
    suffixes_auto->ProjectAway((unsigned)(suffixes_auto->num_of_bdd_variables_ - 1));
    suffixes_auto->Minimize();
    number_of_extra_bits_needed--;
  }

  DVLOG(VLOG_LEVEL) << suffixes_auto->id_ << " = [" << this->id_ << "]->suffixes()";
  return suffixes_auto;
}

StringAutomaton_ptr StringAutomaton::suffixesAtIndex(int index) {
  return suffixesFromTo(index, index);
}

StringAutomaton_ptr StringAutomaton::suffixesFromIndex(int start){
  return suffixesFromTo(start, this->dfa_->ns);
}

StringAutomaton_ptr StringAutomaton::suffixesFromTo(int start, int end) {
  StringAutomaton_ptr suffixes_auto = nullptr;

  if (this->IsEmptyLanguage()) {
    suffixes_auto = StringAutomaton::MakePhi();
    DVLOG(VLOG_LEVEL) << suffixes_auto->id_ << " = [" << this->id_ << "]->suffixes(" << start << ", " << end << ")";
    return suffixes_auto;
  }

  std::set<int> suffixes_from = getStatesReachableBy(start, end);
  unsigned max = suffixes_from.size();
  if (max == 0) {
    suffixes_auto = StringAutomaton::MakePhi();
    DVLOG(VLOG_LEVEL) << suffixes_auto->id_ << " = [" << this->id_ << "]->suffixes(" << start << ", " << end << ")";
    return suffixes_auto;
  } else if (max == 1) {
    max = 2; // that will increase the number_of_variables by 1, by doing so we get a perfectly minimized auto at the end
  }

  // if number of variables are too large for mona, implement an algorithm that find suffixes by finding
  // sub suffixes and union them
  const int number_of_variables = this->num_of_bdd_variables_ + std::ceil(std::log2(max)), // number of variables required
          number_of_states = this->dfa_->ns + 1; // one extra start for the new start state
  int sink_state = this->GetSinkState();

  int* indices = GetBddVariableIndices(number_of_variables);
  char* statuses = new char[number_of_states + 1];
  unsigned extra_bits_value = 0;

  const int number_of_extra_bits_needed = number_of_variables - this->num_of_bdd_variables_;

  std::vector<char>* current_exception = nullptr;
  std::map<int, std::map<std::vector<char>*, int>> exception_map;

  paths state_paths = nullptr, pp = nullptr;
  trace_descr tp = nullptr;
  for (int s = 0; s < this->dfa_->ns; s++) {
    if (s != sink_state) {
      int state_id = s + 1; // new states are off by one
      exception_map[state_id]; // initialize map entry
      state_paths = pp = make_paths(this->dfa_->bddm, this->dfa_->q[s]);
      while (pp) {
        if (pp->to != (unsigned)sink_state) {
          current_exception = new std::vector<char>();
          for (int j = 0; j < this->num_of_bdd_variables_; j++) {
            for (tp = pp->trace; tp && (tp->index != (unsigned)indices[j]); tp = tp->next);
            if (tp) {
              if (tp->value) {
                current_exception->push_back('1');
              } else {
                current_exception->push_back('0');
              }
            } else {
              current_exception->push_back('X');
            }
          }

          exception_map[state_id][current_exception] = pp->to + 1; // there is a new start state, old states are off by one
          current_exception = nullptr;
        }
        tp = nullptr;
        pp = pp->next;
      }

      kill_paths(state_paths);
      state_paths = pp = nullptr;
      // add to start state by adding extra bits
      if (suffixes_from.find(s) != suffixes_from.end()) {
        auto extra_bit_binary_format = GetBinaryFormat(extra_bits_value, number_of_extra_bits_needed);
        for (auto& exceptions : exception_map[state_id]) {
          current_exception = new std::vector<char>();
          current_exception->insert(current_exception->begin(), exceptions.first->begin(), exceptions.first->end());
          for (int i = 0; i < number_of_extra_bits_needed; i++) {
            current_exception->push_back(extra_bit_binary_format[i]); // new transitions for start state
            exceptions.first->push_back('0');                         // default transitions have all zero's in extrabits
          }
          current_exception->push_back('\0');
          exceptions.first->push_back('\0');
          exception_map[0][current_exception] = exceptions.second; // new start state
          current_exception = nullptr;
        }
        ++extra_bits_value;
      } else {
        // default transitions' extra bits extended with all zeros
        for (auto& exceptions : exception_map[s]) {
          for (int i = 0; i < number_of_extra_bits_needed; i++) {
            exceptions.first->push_back('0'); // default transitions have all zero's in extrabits
          }
          exceptions.first->push_back('\0');
        }
      }
    }
  }

  if (sink_state != -1) {
    ++sink_state; // old states are off by one
  }

  dfaSetup(number_of_states, number_of_variables, indices);
  for (int s = 0; s < number_of_states; s++) {
    statuses[s] = '-';
    if (s != sink_state) {
      int old_state = s - 1;
      statuses[s] = '-'; // initially
      dfaAllocExceptions(exception_map[s].size());
      for (auto it = exception_map[s].begin(); it != exception_map[s].end();) {
        dfaStoreException(it->second, &*it->first->begin());
        current_exception = it->first;
        it = exception_map[s].erase(it);
        delete current_exception;
      }
      dfaStoreState(sink_state);
      current_exception = nullptr;
      if (old_state > -1 and IsAcceptingState(old_state)) {
        statuses[s] = '+';
      }
    } else {
      dfaAllocExceptions(0);
      dfaStoreState(s);
    }
  }

  statuses[number_of_states] = '\0';
  DFA_ptr result_dfa = dfaBuild(statuses);
  delete[] indices;
  delete[] statuses;
  suffixes_auto = new StringAutomaton(dfaMinimize(result_dfa), number_of_variables);
  dfaFree(result_dfa); result_dfa = nullptr;

  for ( int i = 0; i < number_of_extra_bits_needed; ++i) {
    suffixes_auto->ProjectAway((unsigned)(suffixes_auto->num_of_bdd_variables_ - 1));
    suffixes_auto->Minimize();
  }

  DVLOG(VLOG_LEVEL) << suffixes_auto->id_ << " = [" << this->id_ << "]->suffixes(" << start << ", " << end << ")";
  return suffixes_auto;
}

StringAutomaton_ptr StringAutomaton::prefixes(){
  StringAutomaton_ptr prefix_auto = this->clone();
  int sink_state = prefix_auto->GetSinkState();


  for (int s = 0; s < prefix_auto->dfa_->ns; s++) {
    if(s != sink_state){
      prefix_auto->dfa_->f[s] = 1;
    }
  }

  prefix_auto->Minimize();

  DVLOG(VLOG_LEVEL) << prefix_auto->id_ << " = [" << this->id_ << "]->prefixes()";
  return prefix_auto;
}

StringAutomaton_ptr StringAutomaton::prefixesUntilIndex(int index){
  StringAutomaton_ptr prefixes_auto = nullptr;
  StringAutomaton_ptr length_auto = nullptr;
  StringAutomaton_ptr prefixesUntil_auto = nullptr;

  prefixes_auto = this->prefixes();
  length_auto = MakeAnyStringLengthLessThan(index);

  prefixesUntil_auto = static_cast<StringAutomaton_ptr>(prefixes_auto->Intersect(length_auto));
  DVLOG(VLOG_LEVEL) << prefixesUntil_auto->id_ << " = [" << this->id_ << "]->prefixesUntilIndex("<<index<<")";
  return prefixesUntil_auto;
}

StringAutomaton_ptr StringAutomaton::prefixesAtIndex(int index){
  StringAutomaton_ptr length_auto = nullptr;
  auto prefixes_auto = this->prefixes();
  if (index == 0 and this->hasEmptyString()) {
    // when index is 0, result should also accept empty string if subject automaton accepts empty string
    length_auto = StringAutomaton::MakeAnyStringLengthLessThanOrEqualTo(1);
  } else {
    length_auto = MakeAnyStringLengthEqualTo(index + 1);
  }
  auto prefixesAt_auto = static_cast<StringAutomaton_ptr>(prefixes_auto->Intersect(length_auto));
  delete prefixes_auto; prefixes_auto = nullptr;
  delete length_auto; length_auto = nullptr;
  DVLOG(VLOG_LEVEL) << prefixesAt_auto->id_ << " = [" << this->id_ << "]->prefixesAtIndex("<<index<<")";
  return prefixesAt_auto;
}

/**
 * In theory empty string should be always a prefix, suffix, and a factor
 * @return substrings (factor) of a string
 */
StringAutomaton_ptr StringAutomaton::subStrings() {
  StringAutomaton_ptr suffixes_auto = nullptr, sub_strings_auto = nullptr;

  suffixes_auto = this->suffixes();
  sub_strings_auto = suffixes_auto->prefixes();
  delete suffixes_auto; suffixes_auto = nullptr;

  DVLOG(VLOG_LEVEL) << sub_strings_auto->id_ << " = [" << this->id_ << "]->subStrings()";
  return sub_strings_auto;
}

StringAutomaton_ptr StringAutomaton::CharAt(const int index) {

  if (this->IsEmptyLanguage()) {
    auto charat_auto = StringAutomaton::MakePhi();
    DVLOG(VLOG_LEVEL) << charat_auto->id_ << " = [" << this->id_ << "]->charAt(" << index << ")";
    return charat_auto;
  }

  std::set<int> states_at_index = getStatesReachableBy(index);
  unsigned max = states_at_index.size();
  if (max == 0) {
    auto charat_auto = StringAutomaton::MakePhi();
    DVLOG(VLOG_LEVEL) << charat_auto->id_ << " = [" << this->id_ << "]->charAt(" << index << ")";
    return charat_auto;
  }

  // if number of variables are too large for mona, implement an algorithm that find suffixes by finding
  // sub suffixes and union them
  const int number_of_variables = this->num_of_bdd_variables_ + std::ceil(std::log2(max)), // number of variables required
          sink_state = this->GetSinkState();
  int* indices = GetBddVariableIndices(number_of_variables);

  unsigned extra_bits_value = 0;

  const int number_of_extra_bits_needed = number_of_variables - this->num_of_bdd_variables_;

  std::vector<char>* current_exception = nullptr;
  std::set<std::vector<char>*> exceptions;

  paths state_paths = nullptr, pp = nullptr;
  trace_descr tp = nullptr;
  for (int s : states_at_index) {
    state_paths = pp = make_paths(this->dfa_->bddm, this->dfa_->q[s]);
    while (pp) {
      if (pp->to != (unsigned)sink_state) {
        current_exception = new std::vector<char>();
        for (int j = 0; j < this->num_of_bdd_variables_; j++) {
          for (tp = pp->trace; tp && (tp->index != (unsigned)indices[j]); tp = tp->next);
          if (tp) {
            if (tp->value) {
              current_exception->push_back('1');
            } else {
              current_exception->push_back('0');
            }
          } else {
            current_exception->push_back('X');
          }
        }

        auto extra_bit_binary_format = GetBinaryFormat(extra_bits_value, number_of_extra_bits_needed);
        for (int i = 0; i < number_of_extra_bits_needed; i++) {
          current_exception->push_back(extra_bit_binary_format[i]); // collected transition
        }
        current_exception->push_back('\0');
        exceptions.insert(current_exception);
        current_exception = nullptr;
      }
      tp = nullptr;
      pp = pp->next;
    }
    ++extra_bits_value;
    kill_paths(state_paths);
    state_paths = pp = nullptr;
  }

  char statuses[3] = {'-', '+', '-'};
  dfaSetup(3, number_of_variables, indices);
  // 0 -> 1
  dfaAllocExceptions(exceptions.size());
  for (auto it = exceptions.begin(); it != exceptions.end();) {
    auto ex = *it;
    dfaStoreException(1, &(*ex->begin()));
    it = exceptions.erase(it);
    delete ex;
  }
  dfaStoreState(2); // 0 -> 2

  dfaAllocExceptions(0);
  dfaStoreState(2); // 1 -> 2

  dfaAllocExceptions(0);
  dfaStoreState(2); // 2 -> 2

  DFA_ptr result_dfa = dfaBuild(statuses);
  delete[] indices;
  auto charat_auto = new StringAutomaton(dfaMinimize(result_dfa), number_of_variables);
  dfaFree(result_dfa); result_dfa = nullptr;

  for ( int i = 0; i < number_of_extra_bits_needed; ++i) {
    charat_auto->ProjectAway((unsigned)(charat_auto->num_of_bdd_variables_ - 1));
    charat_auto->Minimize();
  }

  DVLOG(VLOG_LEVEL) << charat_auto->id_ << " = [" << this->id_ << "]->CharAt(" << index << ")";
  return charat_auto;
}

StringAutomaton_ptr StringAutomaton::CharAt(IntAutomaton_ptr index_auto) {

  StringAutomaton_ptr prefixes_auto = this->prefixes();
  StringAutomaton_ptr string_length_auto = new StringAutomaton(index_auto->getDFA());
  StringAutomaton_ptr any_char_auto = StringAutomaton::MakeAnyChar();
  StringAutomaton_ptr tmp_length_auto = static_cast<StringAutomaton_ptr>(string_length_auto->Concat(any_char_auto));
  string_length_auto->dfa_ = nullptr; //TODO avoid this in the future by better using unary auto instead of int auto
  delete string_length_auto;
  delete any_char_auto;
  StringAutomaton_ptr charat_indexes_auto = static_cast<StringAutomaton_ptr>(prefixes_auto->Intersect(tmp_length_auto));
  delete prefixes_auto;
  delete tmp_length_auto;

  const int number_of_variables = charat_indexes_auto->num_of_bdd_variables_;

  std::set<std::string> exceptions;
  for (int s = 0; s < charat_indexes_auto->dfa_->ns; ++s) {
    for (int next : charat_indexes_auto->getNextStates(s)) {
      if (charat_indexes_auto->IsAcceptingState(next)) {
        // extract transitions
        auto transitions = Automaton::DFAGetTransitionsFromTo(charat_indexes_auto->dfa_, s, next, number_of_variables);
        exceptions.insert(transitions.begin(), transitions.end());
      }
    }
  }

  const int number_of_exceptions = exceptions.size();
  dfaSetup(3, number_of_variables, GetBddVariableIndices(number_of_variables));
  char statuses[3] { '-', '+', '-' };
  //state 0
  dfaAllocExceptions(number_of_exceptions);
  for (std::string exception : exceptions) {
    dfaStoreException(1, &exception[0]);
  }
  dfaStoreState(2);

  //state 1
  dfaAllocExceptions(0);
  dfaStoreState(2);

  //state 2
  dfaAllocExceptions(0);
  dfaStoreState(2);

  DFA_ptr result_dfa = dfaBuild(statuses);

  StringAutomaton_ptr charat_auto = new StringAutomaton(dfaMinimize(result_dfa), number_of_variables);
  dfaFree(result_dfa); result_dfa = nullptr;


  DVLOG(VLOG_LEVEL) << charat_auto->id_ << " = [" << this->id_ << "]->CharAt(" << index_auto->getId() << ")";
  return charat_auto;
}

StringAutomaton_ptr StringAutomaton::SubString(const int start){
  StringAutomaton_ptr substring_auto = nullptr;
  substring_auto = this->suffixesAtIndex(start);
  DVLOG(VLOG_LEVEL) << substring_auto->id_ << " = [" << this->id_ << "]->subString(" << start << ")";
  return substring_auto;
}

/**
 * TODO decide on substring second param; which one is better:
 * end index, or length of substring
 * subString returns empty when start == end, start is inclusive, end is exclusive
 */
StringAutomaton_ptr StringAutomaton::SubString(const int start, const int end){
  if (start == end) {
    auto substring_auto = StringAutomaton::MakeEmptyString();
    DVLOG(VLOG_LEVEL) << substring_auto->id_ << " = [" << this->id_ << "]->subString(" << start << "," << end << ")";
    return substring_auto;
  }

  int adjusted_end = end;
  if (start < end) {
    --adjusted_end;
  }

  auto suffixes_auto = this->suffixesAtIndex(start);
  auto substring_auto = suffixes_auto->prefixesAtIndex(adjusted_end - start);
  delete suffixes_auto;
  DVLOG(VLOG_LEVEL) << substring_auto->id_ << " = [" << this->id_ << "]->subString(" << start << "," << end << ")";
  return substring_auto;
}

/**
 * @param length_auto is the length of the substring
 * @param search_auto is the strings that cannot appear in the result
 * TODO if search auto contains empty string handle it as a special case
 */
StringAutomaton_ptr StringAutomaton::SubString(IntAutomaton_ptr length_auto, StringAutomaton_ptr search_auto) {
  StringAutomaton_ptr substring_auto = nullptr;

  auto prefix_does_not_contain_search_auto = this->indexOfHelper(search_auto);
  substring_auto = prefix_does_not_contain_search_auto->restrictLengthTo(length_auto);
  delete prefix_does_not_contain_search_auto;

  DVLOG(VLOG_LEVEL) << substring_auto->id_ << " = [" << this->id_ << "]->subString(" << length_auto->getId() << "," << search_auto->id_ << ")";
  return substring_auto;
}

StringAutomaton_ptr StringAutomaton::subString(int start, IntAutomaton_ptr end_auto) {
  auto valid_indexes = IntAutomaton::makeIntGreaterThan(start);
  auto valid_end_indexes = static_cast<IntAutomaton_ptr>(end_auto->Intersect(valid_indexes));
  delete valid_indexes;
  if (valid_end_indexes->IsEmptyLanguage()) {
    return StringAutomaton::MakePhi();
  } else if (valid_end_indexes->isAcceptingSingleInt()) {
    return SubString(start, valid_end_indexes->getAnAcceptingInt());
  }
  LOG (FATAL) << "Fully implement substring with symbolic ints";
  return nullptr;
}

/**
 * A specialized substring operation that works with char or string search
 */
StringAutomaton_ptr StringAutomaton::subStringLastOf(StringAutomaton_ptr search_auto) {
  StringAutomaton_ptr substring_auto = nullptr, contains_auto = nullptr,
          last_index_of_auto = nullptr, right_auto = nullptr,
          search_param_auto = search_auto;

  bool search_has_empty_string = false;

  if (search_param_auto->hasEmptyString()) {
    StringAutomaton_ptr non_empty_string = MakeAnyStringLengthGreaterThan(0);
    search_param_auto = static_cast<StringAutomaton_ptr>(search_param_auto->Intersect(non_empty_string));
    delete non_empty_string; non_empty_string = nullptr;
    search_has_empty_string = true;
  }

  contains_auto = this->contains(search_param_auto);
  if (contains_auto->IsEmptyLanguage()) {
    delete contains_auto; contains_auto = nullptr;
    if (search_has_empty_string) {
      substring_auto = StringAutomaton::MakeEmptyString();
      delete search_param_auto; search_param_auto = nullptr;
    } else {
      substring_auto = StringAutomaton::MakePhi();
    }
    DVLOG(VLOG_LEVEL) << substring_auto->id_ << " = [" << this->id_ << "]->subStringLastOf(" << search_auto->id_  << ")";
    return substring_auto;
  }

  last_index_of_auto = contains_auto->lastIndexOfHelper(search_param_auto);

  // Get substring automaton using preConcatRight
  right_auto = contains_auto->preConcatRight(last_index_of_auto);
  delete last_index_of_auto; last_index_of_auto = nullptr;
  delete contains_auto; contains_auto = nullptr;
  substring_auto = right_auto->restrictLastOccuranceOf(search_param_auto);
  delete right_auto; right_auto = nullptr;

  if (search_has_empty_string) {
    StringAutomaton_ptr tmp_auto = substring_auto;
    StringAutomaton_ptr empty_string = StringAutomaton::MakeEmptyString();
    substring_auto = static_cast<StringAutomaton_ptr>(tmp_auto->Union(empty_string));
    delete tmp_auto; tmp_auto = nullptr;
    delete empty_string; empty_string = nullptr;
  }

  DVLOG(VLOG_LEVEL) << substring_auto->id_ << " = [" << this->id_ << "]->subStringLastOf(" << search_auto->id_ << ")";
  return substring_auto;
}

/**
 * A specialized substring operation that works with char or string search
 */
StringAutomaton_ptr StringAutomaton::subStringFirstOf(StringAutomaton_ptr search_auto) {
  StringAutomaton_ptr substring_auto = nullptr, contains_auto = nullptr,
          index_of_auto = nullptr, right_auto = nullptr,
          search_param_auto = search_auto;

  bool search_has_empty_string = false;

  if (search_param_auto->hasEmptyString()) {
    StringAutomaton_ptr non_empty_string = MakeAnyStringLengthGreaterThan(0);
    search_param_auto = static_cast<StringAutomaton_ptr>(search_param_auto->Intersect(non_empty_string));
    delete non_empty_string; non_empty_string = nullptr;
    search_has_empty_string = true;
  }

  contains_auto = this->contains(search_param_auto);
  if (contains_auto->IsEmptyLanguage()) {
    delete contains_auto; contains_auto = nullptr;
    if (search_has_empty_string) {
      substring_auto = this->clone();
      delete search_param_auto; search_param_auto = nullptr;
    } else {
      substring_auto = StringAutomaton::MakePhi();
    }
    DVLOG(VLOG_LEVEL) << substring_auto->id_ << " = [" << this->id_ << "]->subStringFirstOf(" << search_auto->id_  << ")";
    return substring_auto;
  }

  index_of_auto = contains_auto->indexOfHelper(search_param_auto);

  // Get substring automaton using preConcatRight
  right_auto = contains_auto->preConcatRight(index_of_auto);
  delete index_of_auto; index_of_auto = nullptr;
  delete contains_auto; contains_auto = nullptr;

  substring_auto = right_auto->begins(search_auto);

  delete right_auto; right_auto = nullptr;

  if (search_has_empty_string) {
    StringAutomaton_ptr tmp_auto = substring_auto;
    substring_auto = static_cast<StringAutomaton_ptr>(tmp_auto->Union(this));
    delete tmp_auto; tmp_auto = nullptr;
  }

  DVLOG(VLOG_LEVEL) << substring_auto->id_ << " = [" << this->id_ << "]->subStringFirstOf(" << search_auto->id_ << ")";

  return substring_auto;
}

/**
 * TODO check if any 255 transition goes to a valid state at the end
 * TODO Add support when search auto is not a singleton
 */
IntAutomaton_ptr StringAutomaton::indexOf(StringAutomaton_ptr search_auto) {
  StringAutomaton_ptr contains_auto = nullptr, difference_auto = nullptr,
      index_of_auto = nullptr, search_param_auto = search_auto;
  IntAutomaton_ptr length_auto = nullptr;

  bool has_negative_1 = false;
  bool search_has_empty_string = false;

  if (search_param_auto->hasEmptyString()) {
    StringAutomaton_ptr non_empty_string = MakeAnyStringLengthGreaterThan(0);
    search_param_auto = static_cast<StringAutomaton_ptr>(search_param_auto->Intersect(non_empty_string));
    delete non_empty_string; non_empty_string = nullptr;
    search_has_empty_string = true;
  }

  contains_auto = this->contains(search_param_auto);
  if (contains_auto->IsEmptyLanguage()) {
    delete contains_auto;
    // if search has empty string indexOf also returns 0
    if (search_has_empty_string) {
      length_auto = IntAutomaton::makeZero();
      length_auto->setMinus1(true);
      delete search_param_auto; search_param_auto = nullptr; // search_param_auto auto is not the parameter search auto, it is updated, delete it
    } else {
      length_auto = IntAutomaton::makeInt(-1);
    }

    DVLOG(VLOG_LEVEL) << length_auto->getId() << " = [" << this->id_ << "]->indexOf(" << search_auto->id_  << ")";
    return length_auto;
  }

  // check for the cases where string does not contain the search char, return -1 in that case
  difference_auto = static_cast<StringAutomaton_ptr>(this->Difference(contains_auto));
  if (not difference_auto->IsEmptyLanguage()) {
    has_negative_1 = true;
  }
  delete difference_auto;
  index_of_auto = contains_auto->indexOfHelper(search_param_auto);
  delete contains_auto; contains_auto = nullptr;

  length_auto = index_of_auto->length();
  length_auto->setMinus1(has_negative_1);
  delete index_of_auto; index_of_auto = nullptr;

  // if search has empty string indexOf also returns 0
  if (search_has_empty_string) {
    if (not length_auto->hasZero()) {
      IntAutomaton_ptr tmp = length_auto;
      IntAutomaton_ptr zero_int = IntAutomaton::makeZero();
      length_auto = static_cast<IntAutomaton_ptr>(tmp->Union(zero_int));
      delete tmp; tmp = nullptr;
      delete zero_int;
    }
    delete search_param_auto; search_param_auto = nullptr; // search_param_auto auto is not the parameter search auto, it is updated, delete it
  }

  DVLOG(VLOG_LEVEL) << length_auto->getId() << " = [" << this->id_ << "]->indexOf(" << search_auto->id_  << ")";
  return length_auto;
}

/**
 *
 * TODO Add support when search auto is not a singleton, see test case 09
 */
IntAutomaton_ptr StringAutomaton::lastIndexOf(StringAutomaton_ptr search_auto) {
  StringAutomaton_ptr contains_auto = nullptr, difference_auto = nullptr,
      last_index_of_auto = nullptr, search_param_auto = search_auto;
  IntAutomaton_ptr length_auto = nullptr;

  bool has_negative_1 = false;
  bool search_has_empty_string = false;

  if (search_param_auto->hasEmptyString()) {
    StringAutomaton_ptr non_empty_string = MakeAnyStringLengthGreaterThan(0);
    search_param_auto = static_cast<StringAutomaton_ptr>(search_param_auto->Intersect(non_empty_string));
    delete non_empty_string; non_empty_string = nullptr;
    search_has_empty_string = true;
  }

  contains_auto = this->contains(search_param_auto);
  if (contains_auto->IsEmptyLanguage()) {
    delete contains_auto;
    if (search_has_empty_string) {
      length_auto = this->length();
      length_auto->setMinus1(true);
      delete search_param_auto; search_param_auto = nullptr; // search_param_auto auto is not the parameter search auto, it is updated, delete it
    } else {
      length_auto = IntAutomaton::makeInt(-1);
    }
    DVLOG(VLOG_LEVEL) << length_auto->getId() << " = [" << this->id_ << "]->lastIndexOf(" << search_auto->id_  << ")";
    return length_auto;
  }

  difference_auto = static_cast<StringAutomaton_ptr>(this->Difference(contains_auto));
  if (not difference_auto->IsEmptyLanguage()) {
    has_negative_1 = true;
  }
  delete difference_auto;

  last_index_of_auto = contains_auto->lastIndexOfHelper(search_param_auto);
  delete contains_auto; contains_auto = nullptr;

  length_auto = last_index_of_auto->length();
  length_auto->setMinus1(has_negative_1);
  delete last_index_of_auto; last_index_of_auto = nullptr;

  // if search has empty string lastIndexOf also returns all string lengths
  if (search_has_empty_string) {
    IntAutomaton_ptr string_lengths = this->length();
    IntAutomaton_ptr tmp = length_auto;
    length_auto = static_cast<IntAutomaton_ptr>(tmp->Union(string_lengths));
    delete string_lengths; string_lengths = nullptr;
    delete tmp; tmp = nullptr;
    delete search_param_auto; search_param_auto = nullptr; // search_param_auto auto is not the parameter search auto, it is updated, delete it
  }

  DVLOG(VLOG_LEVEL) << length_auto->getId() << " = [" << this->id_ << "]->lastIndexOf(" << search_auto->id_ << ")";

  return length_auto;
}

StringAutomaton_ptr StringAutomaton::contains(StringAutomaton_ptr search_auto) {
  StringAutomaton_ptr contains_auto = nullptr, any_string_auto = nullptr,
          tmp_auto_1 = nullptr, tmp_auto_2 = nullptr;

  any_string_auto = StringAutomaton::MakeAnyString();
  tmp_auto_1 = static_cast<StringAutomaton_ptr>(any_string_auto->Concat(search_auto));
  tmp_auto_2 = static_cast<StringAutomaton_ptr>(tmp_auto_1->Concat(any_string_auto));

  contains_auto = static_cast<StringAutomaton_ptr>(this->Intersect(tmp_auto_2));
  delete any_string_auto;
  delete tmp_auto_1; delete tmp_auto_2;

  DVLOG(VLOG_LEVEL) << contains_auto->id_ << " = [" << this->id_ << "]->contains(" << search_auto->id_ << ")";

  return contains_auto;
}

StringAutomaton_ptr StringAutomaton::begins(StringAutomaton_ptr search_auto) {
  StringAutomaton_ptr begins_auto = nullptr, any_string_auto = nullptr,
          tmp_auto_1 = nullptr;

  any_string_auto = StringAutomaton::MakeAnyString();
  tmp_auto_1 = static_cast<StringAutomaton_ptr>(search_auto->Concat(any_string_auto));

  begins_auto = static_cast<StringAutomaton_ptr>(this->Intersect(tmp_auto_1));

  DVLOG(VLOG_LEVEL) << begins_auto->id_ << " = [" << this->id_ << "]->begins(" << search_auto->id_ << ")";

  return begins_auto;
}

StringAutomaton_ptr StringAutomaton::ends(StringAutomaton_ptr search_auto) {
  StringAutomaton_ptr ends_auto = nullptr, any_string_auto = nullptr,
          tmp_auto_1 = nullptr;

  any_string_auto = StringAutomaton::MakeAnyString();
  tmp_auto_1 = static_cast<StringAutomaton_ptr>(any_string_auto->Concat(search_auto));

  ends_auto = static_cast<StringAutomaton_ptr>(this->Intersect(tmp_auto_1));

  DVLOG(VLOG_LEVEL) << ends_auto->id_ << " = [" << this->id_ << "]->ends(" << search_auto->id_ << ")";

  return ends_auto;
}

StringAutomaton_ptr StringAutomaton::toUpperCase() {
  DFA_ptr upper_case_dfa = nullptr;
  StringAutomaton_ptr upper_case_auto = nullptr;

  LOG(FATAL) << "implement me";
//  upper_case_dfa = dfaToUpperCase(dfa, StringAutomaton::DEFAULT_NUM_OF_VARIABLES, StringAutomaton::DEFAULT_VARIABLE_INDICES);
//  upper_case_auto = new StringAutomaton(upper_case_dfa, num_of_variables);

  DVLOG(VLOG_LEVEL) << upper_case_auto->id_ << " = [" << this->id_ << "]->toUpperCase()";

  return upper_case_auto;
}

StringAutomaton_ptr StringAutomaton::toLowerCase() {
  DFA_ptr lower_case_dfa = nullptr;
  StringAutomaton_ptr lower_case_auto = nullptr;
  LOG(FATAL) << "implement me";
//  lower_case_dfa = dfaToLowerCase(dfa, StringAutomaton::DEFAULT_NUM_OF_VARIABLES, StringAutomaton::DEFAULT_VARIABLE_INDICES);
//  lower_case_auto = new StringAutomaton(lower_case_dfa, num_of_variables);

  DVLOG(VLOG_LEVEL) << lower_case_auto->id_ << " = [" << this->id_ << "]->toLowerCase()";

  return lower_case_auto;
}

StringAutomaton_ptr StringAutomaton::trim() {
  DFA_ptr trimmed_prefix_dfa = nullptr, trimmed_dfa = nullptr;
  StringAutomaton_ptr trimmed_auto = nullptr, trim_auto = nullptr;

  std::string trim_regex = "' '*";
  trim_auto = StringAutomaton::MakeRegexAuto(trim_regex, num_of_bdd_variables_);
  trimmed_prefix_dfa = RelationalStringAutomaton::trim_prefix(this->dfa_,trim_auto->getDFA(),num_of_bdd_variables_);
  trimmed_dfa = RelationalStringAutomaton::trim_suffix(trimmed_prefix_dfa, trim_auto->getDFA(), num_of_bdd_variables_);
  delete trim_auto;
  dfaFree(trimmed_prefix_dfa);

  trimmed_auto = new StringAutomaton(trimmed_dfa, num_of_bdd_variables_);

  DVLOG(VLOG_LEVEL) << trimmed_auto->id_ << " = [" << this->id_ << "]->trim()";

  return trimmed_auto;
}

StringAutomaton_ptr StringAutomaton::replace(StringAutomaton_ptr search_auto, StringAutomaton_ptr replace_auto) {
  DFA_ptr result_dfa = nullptr;
  StringAutomaton_ptr result_auto = nullptr;
  LOG(FATAL) << "implement me";
//  result_dfa = dfa_general_replace_extrabit(dfa, search_auto->dfa, replace_auto->dfa,
//          StringAutomaton::DEFAULT_NUM_OF_VARIABLES, StringAutomaton::DEFAULT_VARIABLE_INDICES);

  result_auto = new StringAutomaton(result_dfa, num_of_bdd_variables_);

  DVLOG(VLOG_LEVEL) << result_auto->id_ << " = [" << this->id_ << "]->repeat(" << search_auto->id_ << ", " << replace_auto->id_ << ")";

  return result_auto;
}


DFA_ptr StringAutomaton::unaryLength() {
  DFA_ptr unary_dfa = nullptr;
  LOG(FATAL) << "implement me";
//  unary_dfa = dfa_string_to_unaryDFA(this->dfa, num_of_variables, variable_indices);

  DVLOG(VLOG_LEVEL) << "?" << " = [" << this->id_ << "]->unaryLength()";

  return unary_dfa;
}

UnaryAutomaton_ptr StringAutomaton::toUnaryAutomaton() {
  UnaryAutomaton_ptr unary_auto = nullptr;
  DFA_ptr unary_dfa = nullptr, tmp_dfa = nullptr;

  int sink_state = this->GetSinkState(),
          number_of_variables = this->get_number_of_bdd_variables() + 1, // one extra bit
          to_state = 0;
  bool has_sink = true;
  int original_num_states = dfa_->ns;
  if(sink_state < 0) {
    has_sink = false;
    sink_state = 0;
  }

  int* indices = GetBddVariableIndices(number_of_variables);
  char* statuses = new char[original_num_states + 1];
  std::map<std::vector<char>*, int> exceptions;
  std::vector<char>* current_exception = nullptr;

  paths state_paths = nullptr, pp = nullptr;
  trace_descr tp = nullptr;


  dfaSetup(original_num_states, number_of_variables, indices);

  for (int i = 0; i < original_num_states; i++) {

    if(i == sink_state && has_sink) {
      dfaAllocExceptions(0);
      dfaStoreState(sink_state);
      statuses[sink_state] = '-';
      continue;
    }

    state_paths = pp = make_paths(dfa_->bddm, dfa_->q[i]);
    while (pp) {
      if (!has_sink || pp->to != (unsigned)sink_state) {
        to_state = pp->to;
        current_exception = new std::vector<char>();
        for (int j = 0; j < number_of_variables - 1; j++) {
          //the following for loop can be avoided if the indices are in order
          for (tp = pp->trace; tp && (tp->index != (unsigned)indices[j]); tp= tp->next);

          if (tp) {
            if (tp->value){
              current_exception->push_back('1');
            } else{
              current_exception->push_back('0');
            }
          } else {
            current_exception->push_back('X');
          }
        }

        current_exception->push_back('1');
        current_exception->push_back('\0');
        exceptions[current_exception] = to_state;
      }
      current_exception = nullptr;
      tp = nullptr;
      pp = pp->next;
    }

    dfaAllocExceptions(exceptions.size());
    for (auto it = exceptions.begin(); it != exceptions.end();) {
      dfaStoreException(it->second, &*it->first->begin());
      current_exception = it->first;
      it = exceptions.erase(it);
      delete current_exception;
    }
    dfaStoreState(sink_state);
    current_exception = nullptr;
    exceptions.clear();

    if (dfa_->f[i] == 1) {
      statuses[i] = '+';
    } else if (dfa_->f[i] == -1) {
      statuses[i] = '-';
    } else {
      statuses[i] = '0';
    }

    kill_paths(state_paths);
    state_paths = pp = nullptr;
  }
  statuses[this->dfa_->ns] = '\0';
  unary_dfa = dfaBuild(statuses);
  delete[] indices; indices = nullptr;
  delete[] statuses; statuses = nullptr;

  for (int i = 0; i < number_of_variables - 1; i++) { // project away all bits
    tmp_dfa = unary_dfa;
    unary_dfa = dfaProject(tmp_dfa,  (unsigned)i);
    dfaFree(tmp_dfa);
    tmp_dfa = unary_dfa;
    unary_dfa = dfaMinimize(tmp_dfa);
    dfaFree(tmp_dfa);
  }

  int* indices_map = GetBddVariableIndices(number_of_variables);
  indices_map[number_of_variables - 1] = 0;
  dfaReplaceIndices(unary_dfa, indices_map);
  delete[] indices_map;

  // make sure no "dont care" states
  for(int i = 0; i < unary_dfa->ns; i++) {
    if(unary_dfa->f[i] == 0) {
      unary_dfa->f[i] = -1;
    }
  }

  unary_auto = new UnaryAutomaton(unary_dfa);
  DVLOG(VLOG_LEVEL) << unary_auto->getId() << " = [" << this->id_ << "]->toUnaryAutomaton()";
  return unary_auto;
}

IntAutomaton_ptr StringAutomaton::parseToIntAutomaton() {
  IntAutomaton_ptr int_auto = nullptr;
  if (this->isCyclic()) {
    int_auto = IntAutomaton::makeIntGreaterThanOrEqual(0);
  } else if (this->IsEmptyLanguage()) {
    int_auto = IntAutomaton::makePhi();
  } else {
    using StatePaths = std::pair<int, std::vector<std::string>>;
    std::vector<std::string> paths_to_state;
    paths_to_state.push_back("");

    std::stack<StatePaths> dfs_stack;
    dfs_stack.push(std::make_pair(this->dfa_->s, paths_to_state));

    paths state_paths = nullptr, pp = nullptr;
    trace_descr tp = nullptr;
    int current_state;
    int sink_state = this->GetSinkState();


    std::map<int, std::vector<std::string>> current_paths_to_state; // <to state, paths>
    std::vector<char> current_exception;
    std::vector<char> decoded_exception;
    std::vector<int> int_values;

    if (IsAcceptingState(this->dfa_->s)) {
      int_values.push_back(0);
    }

    const int *indices = GetBddVariableIndices(num_of_bdd_variables_);

    while (not dfs_stack.empty()) {
      auto current_state_info = dfs_stack.top(); dfs_stack.pop();
      current_state = current_state_info.first;
      paths_to_state = current_state_info.second;

      state_paths = pp = make_paths(dfa_->bddm, dfa_->q[current_state]);
      while (pp) {
        if (pp->to != (unsigned)sink_state) {
          for (int j = 0; j < num_of_bdd_variables_; j++) {
            for (tp = pp->trace; tp && (tp->index != (unsigned)indices[j]); tp= tp->next);
              if (tp) {
                if (tp->value){
                  current_exception.push_back('1');
                } else{
                  current_exception.push_back('0');
                }
              } else {
                current_exception.push_back('X');
              }
          }
          // update path
          decoded_exception = decodeException(current_exception);
          for (auto ch : decoded_exception) {
            for (auto p : paths_to_state) {
              current_paths_to_state[pp->to].push_back(p + ch);
            }
          }
        }
        current_exception.clear();
        tp = nullptr;
        pp = pp->next;
      }

      for (auto& entry : current_paths_to_state) {
        if (IsAcceptingState(entry.first)) {
          for (auto str_value : entry.second) {
            int_values.push_back(std::stoi(str_value));
          }
        }
        dfs_stack.push(std::make_pair(entry.first, entry.second));
      }

      kill_paths(state_paths);
      state_paths = pp = nullptr;
      current_paths_to_state.clear();
    }

    int_auto = IntAutomaton::makeInts(int_values);
  }

  DVLOG(VLOG_LEVEL) << int_auto->getId() << " = [" << this->id_ << "]->parseToIntAutomaton()";
  return int_auto;
}

IntAutomaton_ptr StringAutomaton::length() {
  IntAutomaton_ptr length_auto = nullptr;
  if (this->IsEmptyLanguage()) {
    length_auto = IntAutomaton::makePhi(num_of_bdd_variables_);
  } else if (this->isAcceptingSingleString()) {
    std::string example = this->getAnAcceptingString();
    length_auto = IntAutomaton::makeInt(example.length(), num_of_bdd_variables_);
  } else {
    UnaryAutomaton_ptr unary_auto = this->toUnaryAutomaton();
    length_auto = unary_auto->toIntAutomaton(num_of_bdd_variables_);
    delete unary_auto; unary_auto = nullptr;
  }

  DVLOG(VLOG_LEVEL) << length_auto->getId() << " = [" << this->id_ << "]->length()";

  return length_auto;
}



StringAutomaton_ptr StringAutomaton::restrictLengthTo(int length) {
  StringAutomaton_ptr restricted_auto = nullptr;
  StringAutomaton_ptr length_string_auto = StringAutomaton::MakeAnyStringLengthEqualTo(length);

  restricted_auto = static_cast<StringAutomaton_ptr>(this->Intersect(length_string_auto));
  delete length_string_auto; length_string_auto = nullptr;

  DVLOG(VLOG_LEVEL) << restricted_auto->id_ << " = [" << this->id_ << "]->restrictLengthTo(" << length << ")";

  return restricted_auto;
}

StringAutomaton_ptr StringAutomaton::restrictLengthTo(IntAutomaton_ptr length_auto) {
  StringAutomaton_ptr restricted_auto = nullptr;
  StringAutomaton_ptr length_string_auto = new StringAutomaton(length_auto->getDFA());

  restricted_auto = static_cast<StringAutomaton_ptr>(this->Intersect(length_string_auto));
  length_string_auto->dfa_ = nullptr;
  delete length_string_auto; length_string_auto = nullptr;

  DVLOG(VLOG_LEVEL) << restricted_auto->id_ << " = [" << this->id_ << "]->restrictLengthTo(" << length_auto->getId() << ")";

  return restricted_auto;
}

StringAutomaton_ptr StringAutomaton::restrictIndexOfTo(int index, StringAutomaton_ptr search_auto) {
  StringAutomaton_ptr restricted_auto = nullptr;
  IntAutomaton_ptr index_auto = nullptr;
  index_auto = IntAutomaton::makeInt(index);
  restricted_auto = this->restrictIndexOfTo(index_auto, search_auto);
  delete index_auto; index_auto = nullptr;
  return restricted_auto;
}

StringAutomaton_ptr StringAutomaton::restrictIndexOfTo(IntAutomaton_ptr index_auto, StringAutomaton_ptr search_auto) {
  StringAutomaton_ptr restricted_auto = nullptr, contains_auto = nullptr,
          not_contains_length_auto = nullptr, not_contains_subject_auto = nullptr,
          tmp_auto_1 = nullptr, tmp_auto_2 = nullptr;

  bool has_negative_1 = index_auto->hasNegative1();

  StringAutomaton_ptr length_string_auto = new StringAutomaton(index_auto->getDFA());
  StringAutomaton_ptr any_string = StringAutomaton::MakeAnyString();

  contains_auto = any_string->contains(search_auto);
  if (index_auto->hasNegative1()) {
    not_contains_subject_auto = static_cast<StringAutomaton_ptr>(this->Difference(contains_auto));
  }

  not_contains_length_auto = static_cast<StringAutomaton_ptr>(length_string_auto->Difference(contains_auto));
  delete contains_auto; contains_auto = nullptr;
  length_string_auto->dfa_ = nullptr;
  delete length_string_auto; length_string_auto = nullptr;

  tmp_auto_1 = static_cast<StringAutomaton_ptr>(not_contains_length_auto->Concat(search_auto));
  tmp_auto_2 = static_cast<StringAutomaton_ptr>(tmp_auto_1->Concat(any_string));
  delete not_contains_length_auto; not_contains_length_auto = nullptr;
  delete tmp_auto_1; tmp_auto_1 = nullptr;
  delete any_string; any_string = nullptr;

  restricted_auto = static_cast<StringAutomaton_ptr>(this->Intersect(tmp_auto_2));
  delete tmp_auto_2; tmp_auto_2 = nullptr;

  if (not_contains_subject_auto not_eq nullptr) {
    tmp_auto_1 = restricted_auto;
    restricted_auto = static_cast<StringAutomaton_ptr>(tmp_auto_1->Union(not_contains_subject_auto));
    delete tmp_auto_1; tmp_auto_1 = nullptr;
    delete not_contains_subject_auto; not_contains_subject_auto = nullptr;
  }

  DVLOG(VLOG_LEVEL) << restricted_auto->id_ << " = [" << this->id_ << "]->restrictIndexOfTo(" << index_auto->getId() << ", " << search_auto->id_ << ")";

  return restricted_auto;
}

StringAutomaton_ptr StringAutomaton::restrictLastIndexOfTo(int index, StringAutomaton_ptr search_auto) {
  StringAutomaton_ptr restricted_auto = nullptr;
  IntAutomaton_ptr index_auto = nullptr;
  index_auto = IntAutomaton::makeInt(index);
  restricted_auto = this->restrictLastIndexOfTo(index_auto, search_auto);
  delete index_auto; index_auto = nullptr;
  return restricted_auto;
}

StringAutomaton_ptr StringAutomaton::restrictLastIndexOfTo(IntAutomaton_ptr index_auto, StringAutomaton_ptr search_auto) {
  StringAutomaton_ptr restricted_auto = nullptr, contains_auto = nullptr,
          not_contains_auto = nullptr, not_contains_subject_auto = nullptr,
          tmp_auto_1 = nullptr, tmp_auto_2 = nullptr;
  StringAutomaton_ptr length_string_auto = new StringAutomaton(index_auto->getDFA());
  StringAutomaton_ptr any_string = StringAutomaton::MakeAnyString();

  contains_auto = any_string->contains(search_auto);
  if (index_auto->hasNegative1()) {
    not_contains_subject_auto = static_cast<StringAutomaton_ptr>(this->Difference(contains_auto));
  }
  not_contains_auto = static_cast<StringAutomaton_ptr>(any_string->Difference(contains_auto));

  delete contains_auto; contains_auto = nullptr;
  delete any_string; any_string = nullptr;

  tmp_auto_1 = static_cast<StringAutomaton_ptr>(length_string_auto->Concat(search_auto));
  tmp_auto_2 = static_cast<StringAutomaton_ptr>(tmp_auto_1->Concat(not_contains_auto));
  length_string_auto->dfa_ = nullptr;
  delete length_string_auto; length_string_auto = nullptr;
  delete tmp_auto_1; tmp_auto_1 = nullptr;
  delete not_contains_auto; not_contains_auto = nullptr;

  restricted_auto = static_cast<StringAutomaton_ptr>(this->Intersect(tmp_auto_2));
  delete tmp_auto_2; tmp_auto_2 = nullptr;

  if (not_contains_subject_auto not_eq nullptr) {
    tmp_auto_1 = restricted_auto;
    restricted_auto = static_cast<StringAutomaton_ptr>(tmp_auto_1->Union(not_contains_subject_auto));
    delete tmp_auto_1; tmp_auto_1 = nullptr;
    delete not_contains_subject_auto; not_contains_subject_auto = nullptr;
  }

  DVLOG(VLOG_LEVEL) << restricted_auto->id_ << " = [" << this->id_ << "]->restrictLastIndexOfTo(" << index_auto->getId() << ", " << search_auto->id_ << ")";

  return restricted_auto;
}

/**
 * Given search auto s, finds Intersection with
 * s . (Sigma - s)*
 *
 */
StringAutomaton_ptr StringAutomaton::restrictLastOccuranceOf(StringAutomaton_ptr search_auto) {
  StringAutomaton_ptr restricted_auto = nullptr, contains_auto = nullptr,
          not_contains_auto = nullptr, tmp_auto_1 = nullptr;
  StringAutomaton_ptr any_string = StringAutomaton::MakeAnyString();

  contains_auto = any_string->contains(search_auto);
  not_contains_auto = static_cast<StringAutomaton_ptr>(any_string->Difference(contains_auto));

  delete contains_auto; contains_auto = nullptr;
  delete any_string; any_string = nullptr;


  tmp_auto_1 = static_cast<StringAutomaton_ptr>(search_auto->Concat(not_contains_auto));
  delete not_contains_auto; not_contains_auto = nullptr;
  delete any_string; any_string = nullptr;

  restricted_auto = static_cast<StringAutomaton_ptr>(this->Intersect(tmp_auto_1));
  delete tmp_auto_1; tmp_auto_1 = nullptr;

  DVLOG(VLOG_LEVEL) << restricted_auto->id_ << " = [" << this->id_ << "]->restrictLastOccuranceTo(" << search_auto->id_ << ")";

  return restricted_auto;
}

StringAutomaton_ptr StringAutomaton::restrictFromIndexToEndTo(int index, StringAutomaton_ptr sub_string_auto) {
  StringAutomaton_ptr restricted_auto = nullptr;
  IntAutomaton_ptr index_auto = nullptr;
  index_auto = IntAutomaton::makeInt(index);
  restricted_auto = this->restrictFromIndexToEndTo(index_auto, sub_string_auto);
  delete index_auto; index_auto = nullptr;
  return restricted_auto;
}

StringAutomaton_ptr StringAutomaton::restrictFromIndexToEndTo(IntAutomaton_ptr index_auto, StringAutomaton_ptr sub_string_auto) {
  StringAutomaton_ptr restricted_auto = nullptr, tmp_auto_1 = nullptr, tmp_auto_2;
  StringAutomaton_ptr length_string_auto = new StringAutomaton(index_auto->getDFA());

  tmp_auto_1 = static_cast<StringAutomaton_ptr>(length_string_auto->Concat(sub_string_auto));
  length_string_auto->dfa_ = nullptr;
  delete length_string_auto; length_string_auto = nullptr;

  restricted_auto = static_cast<StringAutomaton_ptr>(this->Intersect(tmp_auto_1));
  delete tmp_auto_1; tmp_auto_1 = nullptr;

  DVLOG(VLOG_LEVEL) << restricted_auto->id_ << " = [" << this->id_ << "]->restrictFromIndexToEndTo(" << index_auto->getId() << ", " << sub_string_auto->id_ << ")";

  return restricted_auto;
}

StringAutomaton_ptr StringAutomaton::restrictAtIndexTo(int index, StringAutomaton_ptr sub_string_auto) {
  StringAutomaton_ptr restricted_auto = nullptr;
  IntAutomaton_ptr index_auto = nullptr;
  index_auto = IntAutomaton::makeInt(index);
  restricted_auto = this->restrictAtIndexTo(index_auto, sub_string_auto);
  delete index_auto; index_auto = nullptr;
  return restricted_auto;
}

StringAutomaton_ptr StringAutomaton::restrictAtIndexTo(IntAutomaton_ptr index_auto, StringAutomaton_ptr sub_string_auto) {
  StringAutomaton_ptr restricted_auto = nullptr, tmp_auto_1 = nullptr, tmp_auto_2;
  StringAutomaton_ptr length_string_auto = new StringAutomaton(index_auto->getDFA());
  StringAutomaton_ptr any_string = StringAutomaton::MakeAnyString();

  tmp_auto_1 = static_cast<StringAutomaton_ptr>(length_string_auto->Concat(sub_string_auto));
  if (tmp_auto_1->isEmptyString()) {
    // restricting string to be an empty string, a special case for index 0 and sub_string_auto is empty
    tmp_auto_2 = tmp_auto_1->clone();
  } else {
    tmp_auto_2 = static_cast<StringAutomaton_ptr>(tmp_auto_1->Concat(any_string));
  }
  length_string_auto->dfa_ = nullptr; // it is index_auto's dfa
  delete length_string_auto; length_string_auto = nullptr;
  delete tmp_auto_1; tmp_auto_1 = nullptr;
  delete any_string; any_string = nullptr;
  restricted_auto = static_cast<StringAutomaton_ptr>(this->Intersect(tmp_auto_2));
  delete tmp_auto_2; tmp_auto_2 = nullptr;

  DVLOG(VLOG_LEVEL) << restricted_auto->id_ << " = [" << this->id_ << "]->restrictIndexTo(" << index_auto->getId() << ", " << sub_string_auto->id_ << ")";
  return restricted_auto;
}

/**
 * TODO Efficiency of the pre image computations can be improved
 * if they are guided by the post image values
 */

StringAutomaton_ptr StringAutomaton::preToUpperCase(StringAutomaton_ptr rangeAuto) {
  DFA_ptr result_dfa = nullptr;
  StringAutomaton_ptr result_auto = nullptr;
  LOG(FATAL) << "implement me";
//  result_dfa = dfaPreToUpperCase(dfa,
//      StringAutomaton::DEFAULT_NUM_OF_VARIABLES, StringAutomaton::DEFAULT_VARIABLE_INDICES);
//  result_auto = new StringAutomaton(result_dfa, num_of_variables);

  if (rangeAuto not_eq nullptr) {
    StringAutomaton_ptr tmp_auto = result_auto;
    result_auto = static_cast<StringAutomaton_ptr>(tmp_auto->Intersect(rangeAuto));
    delete tmp_auto;
  }

  DVLOG(VLOG_LEVEL) << result_auto->id_ << " = [" << this->id_ << "]->preToUpperCase()";

  return result_auto;
}

StringAutomaton_ptr StringAutomaton::preToLowerCase(StringAutomaton_ptr rangeAuto) {
  DFA_ptr result_dfa = nullptr;
  StringAutomaton_ptr result_auto = nullptr;
  LOG(FATAL) << "implement me";
//  result_dfa = dfaPreToLowerCase(dfa,
//      StringAutomaton::DEFAULT_NUM_OF_VARIABLES, StringAutomaton::DEFAULT_VARIABLE_INDICES);
//  result_auto = new StringAutomaton(result_dfa, num_of_variables);

  if (rangeAuto not_eq nullptr) {
    StringAutomaton_ptr tmp_auto = result_auto;
    result_auto = static_cast<StringAutomaton_ptr>(tmp_auto->Intersect(rangeAuto));
    delete tmp_auto;
  }

  DVLOG(VLOG_LEVEL) << result_auto->id_ << " = [" << this->id_ << "]->preToLowerCase()";

  return result_auto;
}

// CHECK WITH BAKI
// may be wrong
StringAutomaton_ptr StringAutomaton::preTrim(StringAutomaton_ptr rangeAuto) {
  StringAutomaton_ptr result_auto = nullptr, trim_auto = nullptr, temp_auto = nullptr;

  std::string trim_regex = "' '*";
  trim_auto = StringAutomaton::MakeRegexAuto(trim_regex);

  result_auto = static_cast<StringAutomaton_ptr>(this->Concat(trim_auto));
  temp_auto = static_cast<StringAutomaton_ptr>(trim_auto->Concat(result_auto));
  delete result_auto;
  result_auto = static_cast<StringAutomaton_ptr>(temp_auto->Intersect(rangeAuto));
  delete trim_auto;
  delete temp_auto;

  DVLOG(VLOG_LEVEL) << result_auto->id_ << " = [" << this->id_ << "]->preTrim()";
  return result_auto;
}

StringAutomaton_ptr StringAutomaton::preConcatLeft(StringAutomaton_ptr right_auto) {

  DFA_ptr d1,d2,d3;
  d1 = this->dfa_;
  d2 = right_auto->getDFA();
  d3 = RelationalStringAutomaton::pre_concat_prefix(d1,d2,8);
  return new StringAutomaton(d3);
}

/**
 * Fix preconcat issue for rectangle start state representation in to dot
 */
StringAutomaton_ptr StringAutomaton::preConcatRight(StringAutomaton_ptr left_auto) {

  DFA_ptr d1,d2,d3;
  d1 = this->dfa_;
  d2 = left_auto->getDFA();
  d3 = RelationalStringAutomaton::pre_concat_suffix(d1,d2,8);
  return new StringAutomaton(d3);
}

StringAutomaton_ptr StringAutomaton::preReplace(StringAutomaton_ptr searchAuto, std::string replaceString, StringAutomaton_ptr rangeAuto) {
  DFA_ptr result_dfa = nullptr;
  StringAutomaton_ptr result_auto = nullptr;
  std::vector<char> replaceStringVector(replaceString.begin(), replaceString.end());
  replaceStringVector.push_back('\0');
  LOG(FATAL) << "implement me";
//  result_dfa = dfa_pre_replace_str(dfa, searchAuto->dfa, &replaceStringVector[0],
//      StringAutomaton::DEFAULT_NUM_OF_VARIABLES, StringAutomaton::DEFAULT_VARIABLE_INDICES);
//  result_auto = new StringAutomaton(result_dfa, num_of_variables);

  if (rangeAuto not_eq nullptr) {
    StringAutomaton_ptr tmp_auto = result_auto;
    result_auto = static_cast<StringAutomaton_ptr>(tmp_auto->Intersect(rangeAuto));
    delete tmp_auto;
  }

  DVLOG(VLOG_LEVEL) << result_auto->id_ << " = [" << this->id_ << "]->preReplace(" << searchAuto->id_ << ", " << replaceString << ")";

  return result_auto;
}

bool StringAutomaton::hasEmptyString() {
  return IsInitialStateAccepting();
}

bool StringAutomaton::isEmptyString() {
  return IsOnlyAcceptingEmptyInput();
}

bool StringAutomaton::isAcceptingSingleString() {
  return isAcceptingSingleWord();
}

std::string StringAutomaton::getAnAcceptingString() {
  std::stringstream ss;

  auto readable_ascii_heuristic = [](unsigned& index) -> bool {
    switch (index) {
      case 1:
      case 2:
      case 6:
        return true;
      default:
        return false;
    }
  };
  std::vector<bool>* example = getAnAcceptingWord(readable_ascii_heuristic);
  unsigned char c = 0;
  unsigned bit_range = num_of_bdd_variables_ - 1;
  unsigned read_count = 0;
  for (auto bit: *example) {
    if (bit) {
      c |= 1;
    } else {
      c |= 0;
    }

    if (read_count < (bit_range)) {
      c <<= 1;
    }
    if (read_count == bit_range) {
      ss << c;
      c = 0;
      read_count = 0;
    } else {
      read_count++;
    }
  }
  delete example;
  return ss.str();
}

StringFormula_ptr StringAutomaton::get_formula() {
  return formula_;
}

void StringAutomaton::set_formula(StringFormula_ptr formula) {
  formula_ = formula;
}

/**
 * @returns true if state has the given exception to a state that is not sink
 */
bool StringAutomaton::hasExceptionToValidStateFrom(int state, std::vector<char>& exception) {
  int sink_state = this->GetSinkState();
  return (sink_state != this->getNextState(state, exception));
}

/**
 * Returns final states
 */
std::vector<int> StringAutomaton::getAcceptingStates() {
  std::vector<int> final_states;
  for (int s = 0; s < this->dfa_->ns; s++) {
    if (this->IsAcceptingState(s)) {
      final_states.push_back(s);
    }
  }
  return final_states;
}

StringAutomaton_ptr StringAutomaton::getAnyStringNotContainsMe() {
  StringAutomaton_ptr not_contains_auto = nullptr, any_string_auto = nullptr,
          contains_auto = nullptr, tmp_auto_1 = nullptr;

  any_string_auto = StringAutomaton::MakeAnyString();
  tmp_auto_1 = static_cast<StringAutomaton_ptr>(any_string_auto->Concat(this));
  contains_auto = static_cast<StringAutomaton_ptr>(tmp_auto_1->Concat(any_string_auto));
  delete tmp_auto_1; tmp_auto_1 = nullptr;
  delete any_string_auto; any_string_auto = nullptr;
  not_contains_auto = static_cast<StringAutomaton_ptr>(contains_auto->Complement());
  delete contains_auto; contains_auto = nullptr;

  DVLOG(VLOG_LEVEL) << not_contains_auto->id_ << " = [" << this->id_ << "]->getAnyStringNotContainsMe()";

  return not_contains_auto;
}

/**
 * @param search automaton is an automaton that does not accept empty string
 * @this is an automaton that is known to be contains search automaton
 */
StringAutomaton_ptr StringAutomaton::indexOfHelper(StringAutomaton_ptr search_auto) {
  StringAutomaton_ptr index_of_auto = nullptr;
  index_of_auto = this->search(search_auto);
  int sink_state = index_of_auto->GetSinkState();
  int current_state = -1;
  int next_state = -1;
  std::vector<char> flag = {'1', '1', '1', '1', '1', '1', '1', '1', '1'}; // 255 (+1 extrabit)
  std::set<int> next_states;
  std::stack<int> state_work_list;
  std::map<int, bool> visited;

  for (int s = 0; s < index_of_auto->dfa_->ns; s++) {
    index_of_auto->dfa_->f[s] = -1;
  }
  visited[sink_state] = true;
  state_work_list.push(index_of_auto->dfa_->s);
  while (not state_work_list.empty()) {
    current_state = state_work_list.top(); state_work_list.pop();
    visited[current_state] = true;

    next_states = index_of_auto->getNextStates(current_state);

    if (sink_state != (next_state = index_of_auto->getNextState(current_state, flag))) {
      index_of_auto->dfa_->f[current_state] = 1; // mark final state for beginning of a match
      next_states.erase(next_state);
    }

    for (auto n : next_states) {
      if (not visited[n]) {
        state_work_list.push(n);
      }
    }
  }
  index_of_auto->Minimize();

  // remove extra bit used
  index_of_auto->ProjectAway((unsigned)(index_of_auto->num_of_bdd_variables_ - 1));
  index_of_auto->Minimize();

  DVLOG(VLOG_LEVEL) << index_of_auto->id_ << " = [" << this->id_ << "]->indexOfHelper(" << search_auto->id_  << ")";
  return index_of_auto;
}

/**
 * @param search automaton is an automaton that does not accept empty string
 * @this is an automaton that is known to be contains search automaton
 */
StringAutomaton_ptr StringAutomaton::lastIndexOfHelper(StringAutomaton_ptr search_auto) {
  StringAutomaton_ptr lastIndexOf_auto = nullptr, search_result_auto = nullptr;

  DFA_ptr lastIndexOf_dfa = nullptr, minimized_dfa = nullptr;

  search_result_auto = this->search(search_auto);

  Graph_ptr graph = search_result_auto->toGraph();
  // Mark start state of a match
  std::vector<char> flag_1_exception = {'1', '1', '1', '1', '1', '1', '1', '1', '1'}; // 255
  GraphNode_ptr node = nullptr;
  int sink_state = search_result_auto->GetSinkState();
  int next_state = -1;
  for (int s = 0; s < search_result_auto->dfa_->ns; s++) {
    node = graph->getNode(s);
    if (sink_state != (next_state = search_result_auto->getNextState(s, flag_1_exception))) {
      node->addEdgeFlag(1, graph->getNode(next_state)); // flag 1 is to mark for beginning of a match
    }
  }

  // BEGIN find new final states using reverse DFS traversal
  for (auto final_node : graph->getFinalNodes()) {
    std::stack<GraphNode_ptr> node_stack;
    std::map<GraphNode_ptr, bool> is_visited; // by default bool is initialized as false
    GraphNode_ptr curr_node = nullptr;
    node_stack.push(final_node);
    while (not node_stack.empty()) {
      curr_node = node_stack.top(); node_stack.pop();
      is_visited[curr_node] = true;
      for (auto& prev_node : curr_node->getPrevNodes()) {
        if (prev_node->hasEdgeFlag(1, curr_node)) { // a match state found
          prev_node->removeEdgeFlag(1, curr_node);
          prev_node->addEdgeFlag(3, curr_node); // 3 is for new final state
        } else {
          if (is_visited.find(prev_node) == is_visited.end()) {
            node_stack.push(prev_node);
          }
        }
      }
    }
  }

  // END find new final states using reverse DFS traversal
  graph->resetFinalNodesToFlag(3);

  // BEGIN generate automaton
  for (int s = 0; s < search_result_auto->dfa_->ns; s++) {
    GraphNode_ptr node = graph->getNode(s);
    if (graph->isFinalNode(node)) {
      search_result_auto->dfa_->f[s] = 1;
    } else {
      search_result_auto->dfa_->f[s] = -1;
    }
  }

  search_result_auto->Minimize();

  lastIndexOf_auto = search_result_auto->removeReservedWords();
  delete search_result_auto;

  // remove extra bit
  lastIndexOf_auto->ProjectAway((unsigned)(lastIndexOf_auto->num_of_bdd_variables_ - 1));
  lastIndexOf_auto->Minimize();

  DVLOG(VLOG_LEVEL) << lastIndexOf_auto->id_ << " = [" << this->id_ << "]->lastIndexOf(" << search_auto->id_ << ")";

  return lastIndexOf_auto;
}

/**
 * Duplicates each state in the automaton using extra bit,
 * Special words 1111 1111 1, 1111 11110 1 used for the transitions between duplicated states
 *
 * Output M so that L(M)={w|w=x0#1\bar{x1}#2.., where x0x1... \in L(M1)} (usage with extra bit)
 * @param use_extra_bit decides on whether to use extra bit or not.
 *
 */
StringAutomaton_ptr StringAutomaton::getDuplicateStateAutomaton() {
  StringAutomaton_ptr duplicated_auto = nullptr;
  DFA_ptr result_dfa = nullptr;
  paths state_paths = nullptr, pp = nullptr;
  trace_descr tp = nullptr;

  // sharp1: 1111 1111 1
  // sharp0: 1111 1110 1
  std::vector<char> sharp1 = Automaton::getReservedWord('1', num_of_bdd_variables_, true);
  std::vector<char> sharp0 = Automaton::getReservedWord('0', num_of_bdd_variables_, true);

  int number_of_variables = this->num_of_bdd_variables_ + 1,
          sink_state = this->GetSinkState(),
          to_state = 0,
          to_duplicate_state = 0,
          mapped_state_id = 0,
          duplicated_state_id = 0;

  bool has_sink = (sink_state != -1);
  // take precautions as there might not be a sink state...
  int original_num_states = this->dfa_->ns;
  if(sink_state < 0) {
    sink_state = this->dfa_->ns;
    original_num_states++;
  }
  int number_of_states = original_num_states * 2 - 1; // no duplicate sink state

  int* indices = GetBddVariableIndices(number_of_variables);
  std::map<std::vector<char>*, int> exceptions;
  std::vector<char>* current_exception = nullptr;
  char *statuses = new char[number_of_states + 1];
  bool sink_state_allocated = false;

  dfaSetup(number_of_states, number_of_variables, indices);
  for (int s = 0; s < original_num_states; s++) {
    if (s != sink_state) {
      state_paths = pp = make_paths(this->dfa_->bddm, this->dfa_->q[s]);
      while (pp) {
        if (pp->to != (unsigned)sink_state) {

          // figure out new state id and collect transitions to that state, (avoid modifying sink state)
          to_state = 2  * pp->to;
          if (pp->to > (unsigned)sink_state) {
            to_state--;
          } else if (to_state >= sink_state) {
            to_state++;
          }

          current_exception = new std::vector<char>();
          for (int j = 0; j < this->num_of_bdd_variables_; j++) {
            for (tp = pp->trace; tp && (tp->index != (unsigned)indices[j]); tp = tp->next);
            if (tp) {
              if (tp->value) {
                current_exception->push_back('1');
              } else {
                current_exception->push_back('0');
              }
            } else {
              current_exception->push_back('X');
            }
          }
          current_exception->push_back('0'); // sharpbit 0 for non-sharp uses
          current_exception->push_back('\0');
          exceptions[current_exception] = to_state;
        }
        current_exception = nullptr;
        tp = nullptr;
        pp = pp->next;
      }
      // figure out new state id for the current state, duplicate state will id will be new_state_id + 1 ( + 2 in case it is a sink)
      mapped_state_id = 2 * s;
      if (s > sink_state) {
        mapped_state_id--;
      } else if (mapped_state_id >= sink_state) {
        mapped_state_id++;
      }

      duplicated_state_id = mapped_state_id + 1;
      if (duplicated_state_id == sink_state) {
        duplicated_state_id++;
      }
      // do allocation for current states
      dfaAllocExceptions(exceptions.size() + 1);
      for (auto entry : exceptions) {
        dfaStoreException(entry.second, &*entry.first->begin());
      }
      dfaStoreException(duplicated_state_id, &*sharp1.begin()); // to duplicated state
      dfaStoreState(sink_state);
      // sink state id is between map_state_id and duplicate_state_id allocate sink state first;
      if ((not sink_state_allocated) and (duplicated_state_id - 1) == sink_state ) {
        dfaAllocExceptions(0);
        dfaStoreState(sink_state);
        statuses[sink_state] = '-';
        sink_state_allocated = true;
      }
      // do allocation for duplicated states
      dfaAllocExceptions(exceptions.size() + 1);
      for (auto it = exceptions.begin(); it != exceptions.end();) {
        to_duplicate_state = it->second + 1;
        if (to_duplicate_state == sink_state) {
          to_duplicate_state++;
        }
        dfaStoreException(to_duplicate_state, &*it->first->begin());
        current_exception = it->first;
        it = exceptions.erase(it);
        delete current_exception;
      }
      dfaStoreException(mapped_state_id, &*sharp0.begin()); // to original state
      dfaStoreState(sink_state);
      // update final states
      if (this->dfa_->f[s] == 1) {
        statuses[mapped_state_id] = '+';
//        statuses[duplicated_state_id] = '0';  // decide on don't care or reject
        statuses[duplicated_state_id] = '-';
      }
      else {
        statuses[mapped_state_id] = '-';
        statuses[duplicated_state_id] = '-';
      }

      kill_paths(state_paths);
      state_paths = pp = nullptr;
      exceptions.clear();
      current_exception = nullptr;
    } else if (not sink_state_allocated) {
      dfaAllocExceptions(0);
      dfaStoreState(sink_state);
      statuses[sink_state] = '-';
      sink_state_allocated = true;
    }
  }

  statuses[number_of_states] = '\0';
  result_dfa = dfaBuild(statuses);

  duplicated_auto = new StringAutomaton(result_dfa, number_of_variables);
  delete[] statuses;
  delete[] indices;

  DVLOG(VLOG_LEVEL) << duplicated_auto->id_ << " = [" << this->id_ << "]->getDuplicateStateAutomaton()";
  return duplicated_auto;
}

/**
 * TODO Discussion: Don't care states can be used for old final state
 * of search automaton, it will force to read one more word, however this time it
 * takes more space, we can use reject if we are sure that we have always have correct
 * ending in our search query automaton.
 * Generates a contains automaton an complements it,
 * Then connects complemented auto with self using
 * reserved keywords 1111 1111 1, 1111 1110 1.
 * Output M so that L(M)={w|w=x0#1\bar{x1}#2.., where \bar{x_i} \in L(M), x_i is \in the complement of L(S*MS*)} (usage with extrabit)
 * @param use_extra_bit decides on whether to use extra bit or not.
 */
StringAutomaton_ptr StringAutomaton::toQueryAutomaton() {
  StringAutomaton_ptr query_auto = nullptr, not_contains_auto = nullptr,
            empty_string_auto = nullptr, tmp_auto_1 = nullptr;

  DFA_ptr result_dfa = nullptr;
  paths state_paths = nullptr, pp = nullptr;
  trace_descr tp = nullptr;

  // sharp1: 1111 1111 1
  // sharp0: 1111 1110 1
  std::vector<char> sharp1 = Automaton::getReservedWord('1', num_of_bdd_variables_, true);
  std::vector<char> sharp0 = Automaton::getReservedWord('0', num_of_bdd_variables_, true);

  int number_of_variables = num_of_bdd_variables_ + 1,
          shift = 0,
          number_of_states = 0,
          sink_state = this->GetSinkState(),
          not_contains_sink_state = -1,
          to_state = 0;

  int* indices = GetBddVariableIndices(number_of_variables);

  std::map<std::vector<char>*, int> exceptions;
  std::vector<char>* current_exception = nullptr;
  char *statuses = nullptr;

  not_contains_auto = this->getAnyStringNotContainsMe();

  // TODO check union with empty works correct
  // union with empty string, so that initial state is accepting
  empty_string_auto = StringAutomaton::MakeEmptyString();
  tmp_auto_1 = not_contains_auto;
  not_contains_auto = static_cast<StringAutomaton_ptr>(tmp_auto_1->Union(empty_string_auto));
  delete empty_string_auto; empty_string_auto = nullptr;
  delete tmp_auto_1; tmp_auto_1 = nullptr;

  not_contains_sink_state = not_contains_auto->GetSinkState();
  if (not_contains_sink_state < 0) {
    shift = not_contains_auto->dfa_->ns;
  } else {
    shift = not_contains_auto->dfa_->ns - 1;
  }

  number_of_states = this->dfa_->ns + shift;
  sink_state += shift;
  statuses = new char[number_of_states + 1];

  dfaSetup(number_of_states, number_of_variables, indices);

  // Construct not contains automaton part
  for (int s = 0, new_state_id = 0; s < not_contains_auto->dfa_->ns; s++) {
    if (s != not_contains_sink_state) {
      state_paths = pp = make_paths(not_contains_auto->dfa_->bddm, not_contains_auto->dfa_->q[s]);
      while (pp) {
        if (pp->to != (unsigned)not_contains_sink_state) {
          if (pp->to > (unsigned)not_contains_sink_state) {
            to_state = pp->to - 1;
          } else {
            to_state = pp->to;
          }

          current_exception = new std::vector<char>();
          for (int j = 0; j < not_contains_auto->num_of_bdd_variables_; j++) {
            for (tp = pp->trace; tp && (tp->index != (unsigned)indices[j]); tp = tp->next);
            if (tp) {
              if (tp->value) {
                current_exception->push_back('1');
              } else {
                current_exception->push_back('0');
              }
            } else {
              current_exception->push_back('X');
            }
          }
          current_exception->push_back('0'); // sharpbit 0
          current_exception->push_back('\0');
          exceptions[current_exception] = to_state;
        }
        current_exception = nullptr;
        tp = nullptr;
        pp = pp->next;
      }

      if (not_contains_auto->dfa_->f[s] == 1) {
        dfaAllocExceptions(exceptions.size() + 1);
        for (auto it = exceptions.begin(); it != exceptions.end();) {
          dfaStoreException(it->second, &*it->first->begin());
          current_exception = it->first;
          it = exceptions.erase(it);
          delete current_exception;
        }
        dfaStoreException(shift, &*sharp1.begin());
        dfaStoreState(sink_state);
        statuses[new_state_id] = '+';
      } else {
        dfaAllocExceptions(exceptions.size());
        for (auto it = exceptions.begin(); it != exceptions.end();) {
          dfaStoreException(it->second, &*it->first->begin());
          current_exception = it->first;
          it = exceptions.erase(it);
          delete current_exception;
        }
        dfaStoreState(sink_state);
        statuses[new_state_id] = '-';
      }

      kill_paths(state_paths);
      state_paths = pp = nullptr;
      current_exception = nullptr;
      exceptions.clear();
      new_state_id++;
    }
  }

  delete not_contains_auto; not_contains_auto = nullptr;

  // construct search automaton part (this)
  for (int s = 0; s < this->dfa_->ns; s++) {
    if (s != sink_state - shift) {
      state_paths = pp = make_paths(this->dfa_->bddm, this->dfa_->q[s]);
      while (pp) {
        if (pp->to != (unsigned)(sink_state - shift)) {
          to_state = pp->to + shift;
          current_exception = new std::vector<char>();
          for (int j = 0; j < number_of_variables-1; j++) {
            for (tp = pp->trace; tp && (tp->index != (unsigned)indices[j]); tp = tp->next);
            if (tp) {
              if (tp->value) {
                current_exception->push_back('1');
              } else {
                current_exception->push_back('0');
              }
            } else {
              current_exception->push_back('X');
            }
          }
          current_exception->push_back('0'); // sharpbit 0
          current_exception->push_back('\0');
          exceptions[current_exception] = to_state;
        }
        tp = nullptr;
        pp = pp->next;
      }

      if (this->dfa_->f[s] == 1) {
        dfaAllocExceptions(exceptions.size() + 1);
        for (auto it = exceptions.begin(); it != exceptions.end();) {
          dfaStoreException(it->second, &*it->first->begin());
          current_exception = it->first;
          it = exceptions.erase(it);
          delete current_exception;
        }
        dfaStoreException(0, &*sharp0.begin()); // add sharp0 to the initial state of not_contains auto
        dfaStoreState(sink_state);
//        statuses[s + shift] = '0'; // TODO decide on don't care or reject
        statuses[s + shift] = '-';
      } else {
        dfaAllocExceptions(exceptions.size());
        for (auto it = exceptions.begin(); it != exceptions.end();) {
          dfaStoreException(it->second, &*it->first->begin());
          current_exception = it->first;
          it = exceptions.erase(it);
          delete current_exception;
        }
        dfaStoreState(sink_state);
        statuses[s + shift] = '-';
      }
      kill_paths(state_paths);
      state_paths = pp = nullptr;
      exceptions.clear();
    } else {
      dfaAllocExceptions(0);
      dfaStoreState(sink_state);
      statuses[sink_state] = '-';
    }
  }

  statuses[number_of_states] = '\0';
  result_dfa = dfaBuild(statuses);
  delete[] statuses;
  delete[] indices;

  query_auto = new StringAutomaton(result_dfa, number_of_variables);
  DVLOG(VLOG_LEVEL) << query_auto->id_ << " = [" << this->id_ << "]->toQueryAutomaton()";

  return query_auto;
}

/**
 * TODO fix the issue when there is empty string accepted by search auto,
 * handle empty string on the caller site
 */
StringAutomaton_ptr StringAutomaton::search(StringAutomaton_ptr search_auto) {
  StringAutomaton_ptr search_result_auto = nullptr, duplicate_auto = nullptr,
          search_query_auto = nullptr;

  duplicate_auto = this->getDuplicateStateAutomaton();
  search_query_auto = search_auto->toQueryAutomaton();
  search_result_auto = static_cast<StringAutomaton_ptr>(duplicate_auto->Intersect(search_query_auto));
  delete duplicate_auto; duplicate_auto = nullptr;
  delete search_query_auto; search_query_auto = nullptr;
  DVLOG(VLOG_LEVEL) << search_result_auto->id_ << " = [" << this->id_ << "]->search(" << search_auto->id_ << ")";
  return search_result_auto;
}

/**
 * Removes special transitions from automaton
 * Can be generalize to general replace algorithm
 */
StringAutomaton_ptr StringAutomaton::removeReservedWords() {
  if(this->num_of_bdd_variables_ < 9) {
    LOG(FATAL) << "can't remove reserved words without first having extra bit";
  }
  StringAutomaton_ptr string_auto = nullptr;
  DFA_ptr result_dfa = nullptr;
  paths state_paths = nullptr, pp = nullptr;
  trace_descr tp = nullptr;

  std::vector<char> flag_1 = {'1', '1', '1', '1', '1', '1', '1', '1', '1'}; // 255
  std::vector<char> flag_2 = {'1', '1', '1', '1', '1', '1', '1', '0', '1'}; // 254

  std::map<int, std::set<int>> merged_states_via_reserved_words;
  std::map<int, int> state_id_map;
  std::map<std::vector<char>*, int> exceptions;

  int number_of_variables = this->num_of_bdd_variables_,
          number_of_states = this->dfa_->ns,
          sink_state = this->GetSinkState(),
          next_state = -1;

  // collect information about automaton
  for (int s = 0; s < this->dfa_->ns; s++) {
    if ( (sink_state != (next_state = this->getNextState(s, flag_1))) or
            (sink_state != (next_state = this->getNextState(s, flag_2))) ) {

      auto it_1 = state_id_map.find(next_state);
      if (it_1 != state_id_map.end()) {
        state_id_map[s] = it_1->second;
      } else {
        state_id_map[s] = next_state;
      }

      merged_states_via_reserved_words[state_id_map[s]].insert(s);

      // update old mappings
      auto it_2 = merged_states_via_reserved_words.find(s);
      if (it_2 != merged_states_via_reserved_words.end()) {
        merged_states_via_reserved_words[state_id_map[s]].insert(it_2->second.begin(), it_2->second.end());
        for (auto merged_state : it_2->second) {
          state_id_map[merged_state] = state_id_map[s];
        }
        merged_states_via_reserved_words.erase(it_2);
      }

    } else {
      state_id_map[s] = s;
      merged_states_via_reserved_words[s].insert(s); // mapped to itself or adds itself
    }
  }

  // keep initial state same
  if (state_id_map[this->dfa_->s] != this->dfa_->s) {
    int old_mapping = state_id_map[this->dfa_->s];
    auto it_2 = merged_states_via_reserved_words.find(old_mapping);
    if (it_2 != merged_states_via_reserved_words.end()) {
      merged_states_via_reserved_words[this->dfa_->s].insert(it_2->second.begin(), it_2->second.end());
      for (auto merged_state : it_2->second) {
        state_id_map[merged_state] = this->dfa_->s;
      }
      merged_states_via_reserved_words.erase(it_2);
    }
  }

  // decide on required number of variables to handle non-determinism
  unsigned max = 0;
  for (auto entry : merged_states_via_reserved_words) {
    if (entry.second.size() > max) {
      max = entry.second.size();
    }
  }

  CHECK_NE(0, max) << "Automaton [" << this->id_ << "] does not include reserved keywords";

  number_of_variables = this->num_of_bdd_variables_ + std::ceil(std::log2(max)); // number of variables required
  int* indices = GetBddVariableIndices(number_of_variables);
  char* statuses = new char[number_of_states + 1];
  unsigned extra_bits_value = 0;
  int number_of_extra_bits_needed = number_of_variables - this->num_of_bdd_variables_;
  std::vector<char>* current_exception = nullptr;

  dfaSetup(number_of_states, number_of_variables, indices);
  for (int s = 0; s < number_of_states; s++) {
    if (merged_states_via_reserved_words.find(s) != merged_states_via_reserved_words.end()) {
      statuses[s] = '-'; // initially
      for(auto merge_state : merged_states_via_reserved_words[s]) {
        auto extra_bit_binary_format = GetBinaryFormat(extra_bits_value, number_of_extra_bits_needed);
        state_paths = pp = make_paths(this->dfa_->bddm, this->dfa_->q[merge_state]);
        while (pp) {
          if (pp->to != (unsigned)sink_state) {
            current_exception = new std::vector<char>();
            for (int j = 0; j < this->num_of_bdd_variables_; j++) {
              for (tp = pp->trace; tp && (tp->index != (unsigned)indices[j]); tp = tp->next);
              if (tp) {
                if (tp->value) {
                  current_exception->push_back('1');
                } else {
                  current_exception->push_back('0');
                }
              } else {
                current_exception->push_back('X');
              }
            }

            // do not add reserved transition, it will be only transition between states if it exists
            if (*current_exception == flag_1 or *current_exception == flag_2) {
              delete current_exception;
            } else {
              for (int i = 0; i < number_of_extra_bits_needed; i++) {
                current_exception->push_back(extra_bit_binary_format[i]);
              }
              current_exception->push_back('\0');
              exceptions[current_exception] = state_id_map[pp->to];
            }
            current_exception = nullptr;
          }

          tp = nullptr;
          pp = pp->next;
        }

        if (this->IsAcceptingState(merge_state)) {
          statuses[s] = '+';
        }

        kill_paths(state_paths);
        state_paths = pp = nullptr;
        extra_bits_value++;
      }
//       do allocation for merged states
      dfaAllocExceptions(exceptions.size());
      for (auto it = exceptions.begin(); it != exceptions.end();) {
        dfaStoreException(it->second, &*it->first->begin());
        current_exception = it->first;
        it = exceptions.erase(it);
        delete current_exception;
      }
      current_exception = nullptr;
      dfaStoreState(sink_state);
      current_exception = nullptr;
      extra_bits_value = 0;
    } else {
      // a state to remove
      dfaAllocExceptions(0);
      dfaStoreState(s);
      statuses[s] = '-';
    }
  }

  statuses[number_of_states] = '\0';
  result_dfa = dfaBuild(statuses);
  delete[] indices;
  delete[] statuses;
  string_auto = new StringAutomaton(dfaMinimize(result_dfa), number_of_variables);
  dfaFree(result_dfa); result_dfa = nullptr;

  while (number_of_extra_bits_needed > 0) {
    string_auto->ProjectAway((unsigned)(string_auto->num_of_bdd_variables_ - 1));
    string_auto->Minimize();
    number_of_extra_bits_needed--;
  }

  DVLOG(VLOG_LEVEL) << string_auto->id_ << " = [" << this->id_ << "]->removeReservedWords()";
  return string_auto;
}

void StringAutomaton::add_print_label(std::ostream& out) {
  out << " subgraph cluster_0 {\n";
  out << "  style = invis;\n  center = true;\n  margin = 0;\n";
  out << "  node[shape=plaintext];\n";
  out << " \"\"[label=\"";
  if (formula_) {
    out << formula_->get_variable_coefficient_map().begin()->first << "\n";
  } else {
    out << "str term" << "\n";
  }
  out << "\"]\n";
  out << " }";
}

} /* namespace Theory */
} /* namespace Vlab */

