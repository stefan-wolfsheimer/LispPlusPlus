/******************************************************************************
Copyright (c) 2018, Stefan Wolfsheimer

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.
******************************************************************************/

#include <lpp/core/memory/cons_pages.h>
#include <lpp/core/memory/collectible_container.h>

using ConsPages = Lisp::ConsPages;

ConsPages::~ConsPages()
{
  for(std::size_t p = 0; p < pages.size(); p++)
  {
    std::size_t s = (p + 1) == pages.size() ? pos : pageSize;
    auto & page(pages[p]);
    for(std::size_t i = 0; i < s; i++)
    {
      page[i].unsetCar();
      page[i].unsetCdr();
    }
    delete [] pages[p];
  }
}

void ConsPages::recycleAll(const std::unordered_set<BasicCons*> & reachable,
                           CollectibleContainer<BasicCons> & target,
                           Color ignoreColor)
{
  recycled.clear();
  for(std::size_t p = 0; p < pages.size(); p++)
  {
    // todo use iterators when friendship is removed
    std::size_t s = (p + 1) == pages.size() ? pos : pageSize;
    auto & page(pages[p]);
    for(std::size_t i = 0; i < s; i++)
    {
      if(reachable.find(&page[i]) == reachable.end())
      {
        recycle(&page[i]);
      }
      else
      {
        if(page[i].getColor() != ignoreColor)
        {
          target.add(&page[i]);
        }
      }
    }
  }
}

