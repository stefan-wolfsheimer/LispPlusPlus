#include <lpp/core/types/function.h>
#include <lpp/core/opcode.h>

using Function = Lisp::Function;

void Function::disassemble(std::ostream & ost) const
{
  //@todo human readable
  std::size_t i = 0;
  ost << "[Function " << this << "]" << std::endl;
  for(auto instr = cbegin(); instr != cend(); ++instr)
  {
    std::cout << i << "\t";
    switch(*instr)
    {
    case RETURNV:
      ost << "RETURNV " << data.atCell(instr[1]);
      instr++;
      break;
    case RETURNS:
      ost << "RETURNS " << *instr;
      instr++;
      break;
    case RETURNL:
      ost << "RETURNL " << data.atCell(instr[1]);
      instr++;
      break;
    case INCRET:
      ost << "INCRET";
      break;
    case FUNCALL:
      ost << "FUNCALL " << instr[1];
      instr++;
      break;
    case DEFINES:
      ost << "DEFINES " << data.atCell(instr[1]);
      break;
    default:
      ost << "instr " << *instr;
    }
    i++;
    std::cout << std::endl;
  }
  ost << "[End " << this << "]" << std::endl;
}