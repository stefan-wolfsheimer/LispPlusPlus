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
#include <assert.h>
#include <core/lisp_cons_factory.h>
#include <core/lisp_object.h>
#include <core/lisp_nil.h>
#include <core/lisp_cons.h>
#include "lisp_cons_graph.h"
#include "lisp_cons_graph_node.h"
#include "lisp_cons_graph_edge.h"

using ConsFactory = Lisp::ConsFactory;
using Color = ConsFactory::Color;
using ConsGraph = Lisp::ConsGraph;

const unsigned short Lisp::SimConsFactory::carIndex = 1;
const unsigned short Lisp::SimConsFactory::cdrIndex = 2;

Lisp::SimConsFactoryRecord::SimConsFactoryRecord()
{
  step = 0;
  numRootConses = 0;
  numReachableConses = 0;
  numVoidConses = 0;
  numFreeConses = 0;
  numEdges = 0;
  edgeFraction = 0;
}

Lisp::SimConsFactory::SimConsFactory()
  : factory(std::make_shared<ConsFactory>(100))
{
}

void Lisp::SimConsFactory::setTargetNumRootConses(std::size_t v)
{
  targetNumRootConses = v;
}

std::size_t Lisp::SimConsFactory::getTargetNumRootConses() const
{
  return targetNumRootConses;
}

void Lisp::SimConsFactory::setTargetNumBulkConses(std::size_t v)
{
  targetNumBulkConses = v;
}
    
std::size_t Lisp::SimConsFactory::getTargetNumBulkConses() const
{
  return targetNumBulkConses;
}

void Lisp::SimConsFactory::setTargetEdgeFraction(double f)
{
  targetEdgeFraction = f;
}

double Lisp::SimConsFactory::getTargetEdgeFraction() const
{
  return targetEdgeFraction;
}

void Lisp::SimConsFactory::setNumSteps(std::size_t n)
{
  numSteps = n;
}

std::size_t Lisp::SimConsFactory::getNumSteps() const
{
  return numSteps;
}

std::vector<Lisp::SimConsFactoryRecord> Lisp::SimConsFactory::run()
{
  std::vector<SimConsFactoryRecord> ret;
  std::list<SharedObject> rootConses;
  std::cout << "step,numRootConses,numReachableConses,voidConses,freeConses,numEdges,edgeFraction" << std::endl;
  for(std::size_t i = 0; i < numSteps; i++)
  {
    SimConsFactoryRecord rec;
    stepRootConses(rootConses);
    stepConses(rootConses);
    stepEdge();
    ConsGraph graph(*factory);
    std::size_t max_edges = factory->numRootConses() + 2 * (graph.numNodes()-1);
    std::size_t min_edges = graph.numNodes()-1;
    double edge_fraction = 0;
    if(max_edges - min_edges)
    {
      edge_fraction = (double)(graph.numEdges() - min_edges) / (max_edges - min_edges);
    }
    assert((graph.numNodes()-1) == factory->getReachableConsesAsSet().size());
    rec.step = i;
    rec.numRootConses = factory->numRootConses();
    rec.numReachableConses = (graph.numNodes()-1);
    rec.numVoidConses = factory->numConses(Color::Void);
    rec.numFreeConses = factory->numConses(Color::Free);
    rec.numEdges = graph.numEdges();
    rec.edgeFraction = edge_fraction;

    std::cout << i << ","
              << factory->numRootConses() << ","
              << (graph.numNodes()-1) << ","
              << factory->numConses(Color::Void) << ","
              << factory->numConses(Color::Free) << ","
              << graph.numEdges() << ","
              << edge_fraction
              << std::endl;
    ret.push_back(rec);
  }
  return ret;
}

void Lisp::SimConsFactory::stepRootConses(std::list<SharedObject> & rootConses)
{
  if(selectAddCons(factory->numRootConses(), targetNumRootConses))
  {
    rootConses.push_back(std::make_shared<Object>(factory->make(Lisp::nil, Lisp::nil)));
  }
  else if(selectRemoveCons(factory->numRootConses(), targetNumRootConses))
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

bool Lisp::SimConsFactory::selectAddCons(std::size_t numberOfConses,
                                         std::size_t target)
{
  static float acceptanceRate = 0.75f;
  static float errorAcceptanceRate = 0.25f;
  if(numberOfConses == 0)
  {
    return true;
  }
  else if(numberOfConses < target)
  {
    return ((float) rand() / (float) RAND_MAX < acceptanceRate);
  }
  else
  {
    return ((float) rand() / (float) RAND_MAX < errorAcceptanceRate);
  }
}

bool Lisp::SimConsFactory::selectRemoveCons(std::size_t numberOfConses,
                                            std::size_t target)
{
  static float acceptanceRate = 0.75f;
  static float errorAcceptanceRate = 0.25f;
  if(numberOfConses == 0)
  {
    return false;
  }
  else if(numberOfConses >= target)
  {
    return ((float) rand() / (float) RAND_MAX < acceptanceRate);
  }
  else
  {
    return ((float) rand() / (float) RAND_MAX < errorAcceptanceRate);
  }
}

std::vector<std::pair<Lisp::Cons*, unsigned short> >
Lisp::SimConsFactory::getFreeEdges()
{
  using ConsPair = std::pair<Cons*, unsigned short>;
  using Cons = Lisp::Cons;
  using Nil = Lisp::Nil;

  std::vector<ConsPair> freeEdges;
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
  }
  return freeEdges;
}

void Lisp::SimConsFactory::stepConses(std::list<SharedObject> & rootConses)
{
  using Cons = Lisp::Cons;
  using Nil = Lisp::Nil;
  using ConsPair = std::pair<Cons*, unsigned short>;
  auto freeEdges = getFreeEdges();
  std::vector<Cons *> nonRootConses;
  for(Cons * cons : factory->getReachableConsesAsSet())
  {
    if(!cons->isRoot())
    {
      nonRootConses.push_back(cons);
    }
  }
  if(freeEdges.size() > 0 && selectAddCons(nonRootConses.size(),
                                           targetNumBulkConses))
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
  else if(selectRemoveCons(nonRootConses.size(), targetNumBulkConses))
  {
    ConsGraph graph(*factory);
    std::size_t i = rand() % nonRootConses.size();
    auto cons = nonRootConses[i];
    auto node = graph.findNode(cons);
    for(auto parent : node->getParents())
    {
      if(parent->getCarCell().as<Cons>() == cons)
      {
        parent->unsetCar();
      }
      if(parent->getCdrCell().as<Cons>() == cons)
      {
        parent->unsetCdr();
      }
    }
  }
}

void Lisp::SimConsFactory::stepEdge()
{
  static float acceptanceRate = 0.75f;
  static float errorAcceptanceRate = 0.25f;
  ConsGraph graph(*factory);
  std::size_t n_root_conses = factory->numRootConses();
  std::size_t n_conses = (graph.numNodes()-1) - n_root_conses;
  double target_num_edges = targetEdgeFraction * ( 2.0 * n_root_conses + n_conses) + n_root_conses + n_conses;
  std::size_t num_edges = graph.numEdges();
  std::size_t min_edges = n_root_conses + n_conses;
  std::size_t max_edges = n_root_conses + 2 * (n_root_conses + n_conses );
  if(num_edges  < max_edges)
  {
    if( (num_edges < target_num_edges && (float) rand() / (float) RAND_MAX < acceptanceRate) ||
        (float) rand() / (float) RAND_MAX < errorAcceptanceRate )
    {
      auto freeEdges = getFreeEdges();
      std::size_t i = rand() % freeEdges.size();
      std::size_t j = rand() % graph.numNodes();
      auto node = graph.getNode(j);
      auto cons = node->getCons();
      if(cons)
      {
        if(freeEdges[i].second == carIndex)
        {
          freeEdges[i].first->setCar(cons);
        }
        else
        {
          freeEdges[i].first->setCdr(cons);
        }
      }
    }
  }
  if(num_edges > min_edges)
  {
    if( (num_edges > target_num_edges && (float) rand() / (float) RAND_MAX < acceptanceRate) ||
        (float) rand() / (float) RAND_MAX < errorAcceptanceRate )
    {
      //Todo remove edge
      std::size_t i = rand() % graph.numEdges();
      auto edge = graph.getEdge(i);
      if(edge)
      {
        Cons * parent = edge->getParent();
        Cons * child = edge->getChild();
        if(parent && child)
        {
          if(parent->getCarCell().as<Cons>() == child)
          {
            parent->unsetCar();
          }
          else if(parent->getCdrCell().as<Cons>() == child)
          {
            parent->unsetCdr();
          }
        }
      }
    }
  }
}
