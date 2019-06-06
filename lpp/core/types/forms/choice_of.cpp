#include <lpp/core/types/forms/choice_of.h>

using ChoiceOf = Lisp::ChoiceOf<void>;

ChoiceOf::ChoiceOf(const std::vector<Form*> & _member)
{
  for(auto f : _member)
  {
    cells.push_back(f);
  }
}

void ChoiceOf::add(Form * f)
{
  cells.push_back(f);
}

bool ChoiceOf::isInstance(const Cell & cell) const
{
  for(const Cell & c : cells)
  {
    assert(c.isA<Form>());
    if(c.as<Form>()->isInstance(cell))
    {
      return true;
    }
  }
  return false;
}
