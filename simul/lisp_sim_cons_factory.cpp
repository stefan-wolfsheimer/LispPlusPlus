/******************************************************************************
Copyright (c) 2017-2018, Stefan Wolfsheimer

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
#include "lisp_sim_cons_factory.h"
#include <list>
#include <cstdlib>
#include <iostream>
#include <core/lisp_cons_factory.h>
#include <core/lisp_object.h>
#include <core/lisp_nil.h>
#include <core/lisp_cons.h>

Lisp::SimConsFactory::SimConsFactory()
  : factory(std::make_shared<ConsFactory>(100, 0, 0))
{
}

void Lisp::SimConsFactory::run()
{
  std::list<SharedObject> rootConses;
  for(std::size_t i = 0; i < num_steps; i++)
  {
    stepRootConses(rootConses);
    stepConses(rootConses);
    std::cout << i << " " << factory->numRootConses() << std::endl;
  }
}

void Lisp::SimConsFactory::stepRootConses(std::list<SharedObject> & rootConses)
{
  if(selectAddCons(factory->numRootConses()))
  {
    rootConses.push_back(std::make_shared<Object>(factory->make(Lisp::nil, Lisp::nil)));
  }
  else if(selectRemoveCons(factory->numRootConses()))
  {
    std::size_t i = rand() % rootConses.size();
    auto itr = rootConses.begin();
    while(i)
    {
      ++itr;
      --i;
    }
    rootConses.erase(itr);
  }
}

bool Lisp::SimConsFactory::selectAddCons(std::size_t numberOfConses)
{
  static float acceptanceRate = 0.75f;
  static float errorAcceptanceRate = 0.25f;
  if(numberOfConses == 0)
  {
    return true;
  }
  else if(numberOfConses < target_num_root_conses)
  {
    return ((float) rand() / (float) RAND_MAX < acceptanceRate);
  }
  else
  {
    return ((float) rand() / (float) RAND_MAX < errorAcceptanceRate);
  }
}

bool Lisp::SimConsFactory::selectRemoveCons(std::size_t numberOfConses)
{
  static float acceptanceRate = 0.75f;
  static float errorAcceptanceRate = 0.25f;
  if(numberOfConses == 0)
  {
    return false;
  }
  else if(numberOfConses >= target_num_root_conses)
  {
    return ((float) rand() / (float) RAND_MAX < acceptanceRate);
  }
  else
  {
    return ((float) rand() / (float) RAND_MAX < errorAcceptanceRate);
  }
}



void Lisp::SimConsFactory::stepConses(std::list<SharedObject> & rootConses)
{
  using Cons = Lisp::Cons;
  using Nil = Lisp::Nil;
  using ConsPair = std::pair<Cons*, unsigned short>;
  static const unsigned short carIndex = 1;
  static const unsigned short cdrIndex = 2;
  std::vector<ConsPair> freeEdges;
  std::size_t nNonRootConses = 0;
  for(Cons * cons : factory->getReachableConsesAsSet())
  {
    if(cons->getCarCell().isA<Nil>())
    {
      freeEdges.push_back(ConsPair(cons, carIndex));
    }
    if(cons->getCdrCell().isA<Nil>())
    {
      freeEdges.push_back(ConsPair(cons, cdrIndex));
    }
    if(!cons->isRoot())
    {
      nNonRootConses++;
    }
  }
  if(freeEdges.size() > 0)
  {
    if(selectAddCons(nNonRootConses))
    {
      std::size_t i = rand() % freeEdges.size();
      if(freeEdges[i].second == carIndex)
      {
        freeEdges[i].first->setCar(Object(factory->make(Lisp::nil, Lisp::nil)));
      }
      else
      {
        freeEdges[i].first->setCdr(Object(factory->make(Lisp::nil, Lisp::nil)));
      }
    }
    else if(selectRemoveCons(nNonRootConses))
    {
    }
  }
}

int main(int argc, const char ** argv)
{
  Lisp::SimConsFactory simConsFactory;
  srand (1);
  simConsFactory.run();
  return 0;
}
