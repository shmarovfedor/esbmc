/*******************************************************************\

   Module: Symbolic Execution

   Author: Daniel Kroening, kroening@kroening.com Lucas Cordeiro,
     lcc08r@ecs.soton.ac.uk

\*******************************************************************/

#ifndef CPROVER_GOTO_SYMEX_GOTO_SYMEX_STATE_H
#define CPROVER_GOTO_SYMEX_GOTO_SYMEX_STATE_H

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <guard.h>
#include <pointer-analysis/value_set.h>
#include <goto-programs/goto_functions.h>
#include <string>
#include <stack>
#include <vector>

#include "symex_target.h"
#include "crypto_hash.h"
#include "renaming.h"

#include <i2string.h>

class execution_statet; // foward dec

/**
 *  Class for storing a particular threads state.
 *  This means storing information about its program counter, its call stack,
 *  the locality of all the variables in that stack, its execution guard, the
 *  number of jumps and states that are hanging around... the everything,
 *  really. Notably, we're storing all that stuff here, we're not mainpulating
 *  it. Instead, that all occurs in the goto_symext class.
 *
 *  Perhaps in the future code will start moving over from one to the other;
 *  however that'd end up with an even larger class containing everything.
 *  Ideally everything would become more segregated.
 */

class goto_symex_statet
{
public:
  class goto_statet; // forward dec
  class framet; // forward dec

  /**
   *  Default constructor.
   *  Sets up blank contents for the call stack, a dummy guard, no data of
   *  interest. Takes references to pieces of global state, the l2 renaming
   *  and value set / pointer tracking situations.
   *  @param l2 Global L2 state reference.
   *  @param vs Global value set reference.
   */
  goto_symex_statet(renaming::level2t &l2, value_sett &vs,
                    const namespacet &_ns);

  /**
   *  Copy constructor.
   *  Performs your normal copy constructor activity; however requires a new
   *  l2 state, because in the majority of circumstances where copy
   *  constructors are needed, it's because a higher up object is getting cloned
   *  and we need to change global state references.
   *  @param state State to copy from
   *  @param l2 New L2 state to refer to.
   *  @param vs New value set to refer to.
   */
  goto_symex_statet(const goto_symex_statet &state, renaming::level2t &l2,
                    value_sett &vs);

  goto_symex_statet &
  operator=(const goto_symex_statet &state);

  // Types

  typedef std::list<goto_statet> goto_state_listt;
  typedef std::map<goto_programt::const_targett,
                   goto_state_listt> goto_state_mapt;
  typedef std::vector<framet> call_stackt;

  /**
   *  Class recording the result of a portion of symex.
   *  A goto_statet records the state of a program having run up to some form
   *  of jump instruction, that needs to be merged with the main state in the
   *  future at some time. To that extent, it has its own level2 copy of gloal
   *  state, its own value set copy, its own depth, and guard. It's primarily
   *  just a container for these values.
   */

  class goto_statet
  {
  public:
    unsigned depth;
    renaming::level2t *level2_ptr;
    renaming::level2t &level2;
    value_sett value_set;
    guardt guard;
    unsigned int thread_id;

    explicit
    goto_statet(const goto_symex_statet &s) :
      depth(s.depth),
      level2_ptr(s.level2.clone()),
      level2(*level2_ptr),
      value_set(s.value_set),
      guard(s.guard),
      thread_id(s.source.thread_nr)
    {
    }

    goto_statet(const goto_statet &s) :
      depth(s.depth),
      level2_ptr(s.level2_ptr->clone()),
      level2(*level2_ptr),
      value_set(s.value_set),
      guard(s.guard),
      thread_id(s.thread_id) {}

  // Deny the use of goto_statet copy constructors
  private:
  goto_statet &operator=(const goto_statet &ref __attribute__((unused)))
  {
    assert(0);
  }

  public:
    ~goto_statet() {
      delete level2_ptr;
      return;
    }
  };

  /**
   *  Stack frame tracking class.
   *  Records all information relevant to a particular stack frame created by
   *  the invocation of a function call. Has a renaming context and level 1
   *  "activation record" ID number to stop recursive calls aliasing. Also
   *  contains function-return data, and the set of states that need to be
   *  merged as the result of jumps in the past. Primarily a data container.
   */
  class framet
  {
  public:
    /** Name of function called to make this stack frame. */
    irep_idt function_identifier;
    /** Map of states to merge in the future. Each state in this map represents
     *  a particular goto_statet that jumps to a particular location in the
     *  function, and that has to have its state joined in a phi function. */
    goto_state_mapt goto_state_map;
    /** Renaming context for L1 names */
    renaming::level1t level1;
    /** Record of source of function call. Used when returning from the function
     *  to the caller. */
    symex_targett::sourcet calling_location;

    /** End of function instruction location. Jumped to after an in-body return
     * instruction. */
    goto_programt::const_targett end_of_function;
    /** Expression to assign return values to. The lvalue that the calller
     *  assigns the result of this function call to at a higher level. */
    exprt return_value;

    typedef std::set<irep_idt> local_variablest;
    /** Set of local variable l1 names. */
    local_variablest local_variables;

    /** List of function pointer targets. During the invocation of a function
     *  pointer call, this contains a list of targets that the function pointer
     *  can point at, and that need to have calls set up to and executed. This
     *  member contains an iterator to the first goto instruction in the target
     *  and the target symbol name. */
    std::list<std::pair<goto_programt::const_targett,exprt> >
      cur_function_ptr_targets;
    /** Instruction where function pointer calls should seem to originate
     *  from. */
    goto_programt::const_targett function_ptr_call_loc;
    /** Function pointer call merge point. Instruction where the resulting
     *  states from particular funtion calls originating from a function pointer
     *  dereference should be merged into main state. */
    goto_programt::const_targett function_ptr_combine_target;
    /** Original function pointer call code. Contains arguments to setup
     *  resulting function invocations with. */
    const code_function_callt *orig_func_ptr_call;

    typedef std::set<std::string> declaration_historyt;
    /** List of variables names that have been declared. Used to detect when we
     *  are in some kind of block that is entered then exited repeatedly -
     *  whenever that happens, a new l1 name is required. This caches the
     *  already seen names in a function for making that decision. */
    declaration_historyt declaration_history;

    framet(unsigned int thread_id) :
      return_value(static_cast<const exprt &>(get_nil_irep())),
      has_throw_decl(false)
    {
      level1._thread_id = thread_id;
    }

    // exceptions
    typedef std::map<irep_idt, goto_programt::targett> catch_mapt;
    catch_mapt catch_map;

    typedef std::set<irep_idt> throw_list_sett;
    throw_list_sett throw_list_set;
    bool has_throw_decl;
  };

  // Macros

  /**
   *  Perform both levels of renaming.
   *  @param identifier Identifier to rename.
   *  @return Renamed identifier.
   */
  std::string
  current_name(const irep_idt &identifier) const
  {
    return current_name(level2, identifier);
  }

  /**
   *  Perform both levels of renaming.
   *  @param plevel2 L2 renaming context to rename with.
   *  @param identifier Identifier to rename.
   *  @return Renamed identifier.
   */
  std::string
  current_name(
    const renaming::level2t &plevel2, const irep_idt &identifier) const
  {
    irep_idt temp = top().level1.get_ident_name(identifier);
    return plevel2.get_ident_name(temp);
  }

  /**
   *  Perform both levels of renaming.
   *  @param goto_state Detatched state containing L2 state to rename with.
   *  @param identifier Identifier to rename.
   *  @return Renamed identifier.
   */
  std::string
  current_name(
    const goto_statet &goto_state, const irep_idt &identifier) const
  {
    return current_name(goto_state.level2, identifier);
  }

  /**
   *  Fetch topmost stack frame.
   *  I.E., give us the stack frame of the function call currently being
   *  interpreted.
   *  @return Currently executing stack frame.
   */
  inline framet &
  top()
  {
    assert(!call_stack.empty());
    return call_stack.back();
  }

  inline const framet &
  top() const
  {
    assert(!call_stack.empty());
    return call_stack.back();
  }

  /**
   *  Push a new fresh stack frame on the stack.
   *  @param thread_id Thread identifier of current state.
   *  @return New stack frame.
   */
  inline framet &
  new_frame(unsigned int thread_id) {
    call_stack.push_back(framet(thread_id));
    return call_stack.back();
  }

  /**
   *  Clear topmost stackframe from the stack.
   */
  inline void
  pop_frame() {
    call_stack.pop_back();
  }

  /**
   *  Return stack frame of previous function call.
   */
  inline const framet &
  previous_frame() {
    return *(--(--call_stack.end()));
  }

  // Methods

  /**
   *  Initialize state with a function call.
   *  Sets up a function call on the stack of the goto program passed in,
   *  between the two iterators passed in.
   *  @param start GOTO instruction to start at.
   *  @param end GOTO instruction to end function at.
   *  @param prog Goto program we're operating over.
   *  @param thread_id Thread identifier of this state.
   */
  void initialize(const goto_programt::const_targett & start,
                  const goto_programt::const_targett & end,
                  const goto_programt *prog,
                  unsigned int thread_id);

  /**
   *  Perform both levels of renaming on an expression.
   *  @param expr Expression to rename contents of.
   */
  void rename(exprt &expr);

  /**
   *  Perform renaming of contents of an address_of operation.
   *  This requires different behaviour, because what we really want is a
   *  pointer to a global variable or l1 variable on the stack, or a heap
   *  object. So, don't perform second level of renaming in this function.
   *  @param expr Expression to rename contents of.
   */
  void rename_address(exprt &expr);

  /**
   *  Make an L2 and value set assignment.
   *  Records an assignment with L2 renaming context and the value set pointer
   *  tracking state. Potentially enables constant propagation if record_value
   *  is true.
   *  @param lhs Symbol being assigned to.
   *  @param rhs Value being assigned to symbol.
   *  @param record_value Whether to enable constant propagation.
   */
  void assignment(exprt &lhs, const exprt &rhs, bool record_value);

  /**
   *  Determine whether to constant propagate the value of an expression.
   *  These obey a few efficiency rules regarding whether or not its efficient
   *  to fully constant propagate away things like "WITH" updates to structs.
   *  Generally any update of an integer that can be simplified, is.
   *  @param expr Expression to decide whether to const propagate.
   *  @return True if constant propagation should be enabled.
   */
  bool constant_propagation(const exprt &expr) const;

  /**
   *  Decide whether to constant_propagate an address_of
   *  @see constant_propagation.
   *  @param expr Expression to decide whether to const propagate.
   *  @return True if constant propagation should be enabled.
   */
  bool constant_propagation_reference(const exprt &expr) const;

  /**
   *  Fetch an original l0 identifer.
   *  Revokes both levels of renaming on an identifer, leaves us with the
   *  original c-level identifier for a symbol.
   *  @param identifier The identifier to reverse renaming on.
   *  @return Renamed to l0 identifier.
   */
  const irep_idt get_original_name(const irep_idt &identifier) const;

  /**
   *  Fetch an original l0 identifer.
   *  Revokes both levels of renaming on an identifer, leaves us with the
   *  original c-level identifier for a symbol. This method applies this to
   *  all contents of an expression.
   *  @param expr The expression to un-rename in place.
   */
  void get_original_name(exprt &expr) const;

  /**
   *  Print stack trace of state to stdout.
   *  Takes all the current function calls and produces an indented stack
   *  trace, then prints it to stdout.
   *  @param indent Number of spaces to indent contents by.
   */
  void print_stack_trace(unsigned int indent) const;

  /**
   *  Generate set of strings making up a stack trace.
   *  From the current thread state, produces a set of strings recording the
   *  current function invocations on the stack, and returns them.
   *  @return Vector of strings describing the current function calls in state.
   */
  std::vector<dstring> gen_stack_trace(void) const;

  // Members

  /** Number of instructions executed in this thread. */
  unsigned depth;

  /** Flag indicating this thread has stopped executing. */
  bool thread_ended;

  /** Current state guard of this thread. */
  guardt guard;
  /** Current program location of this thread. */
  symex_targett::sourcet source;
  /** Invocation count for each function name. Tracks how many times a function
   *  has been called, used by l1 renaming as an activation record. */
  std::map<irep_idt, unsigned> function_frame;
  /** Record of how many loop unwinds we've performed. For each target in the
   *  program that contains a loop, record how many times we've unwound round
   *  it. */
  std::map<symex_targett::sourcet, unsigned> unwind_map;
  /** Record of how many times we've unwound function recursion. */
  std::map<irep_idt, unsigned> function_unwind;

  /** Flag saying whether to maintain pointer value set tracking. */
  bool use_value_set;
  /** Reference to global l2 state. */
  renaming::level2t &level2;
  /** Reference to global pointer tracking state. */
  value_sett &value_set;

  /** Stack of framet's recording current function call stack */
  call_stackt call_stack;

  /** Namespace to work with. */
  const namespacet &ns;
};

#endif
