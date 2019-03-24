#include <lpp/core/types/form.h>
#include <lpp/core/cell.h>
#include <lpp/core/object.h>

using Form = Lisp::Form::Form;

void Form::forEachChild(std::function<void(const Cell&)> func) const
{
  for(const Cell & cell : cells)
  {
    func(cell);
  }
}

bool Form::greyChildren()
{
  for(Cell & cell : cells)
  {
    cell.grey();
  }
  return true;
}

void Form::resetGcPosition()
{
}

bool Form::recycleNextChild()
{
  for(Cell & cell : cells)
  {
    cell = Lisp::nil;
  }
  return true;
}

