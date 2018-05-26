#include "lisp_vm.h"
#include "types/lisp_function.h"
#include "lisp_opcode.h"

using Object = Lisp::Object;
using Function = Lisp::Function;
using Vm = Lisp::Vm;

Object Vm::compile(const Object & obj) const
{
  /*
  if(obj.isA<AtomType>())
  {
    // SETV &1
    // &1 object
    Function * func = new Function(1, 1);
    func->instr.push_back(std::make_pair(Lisp::SETV, 0));
    func->data.push_back(obj);
    return Object(func);
    }*/
  return Lisp::nil;
}
