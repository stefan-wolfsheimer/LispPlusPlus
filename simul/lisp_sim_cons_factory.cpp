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
#include <exception>
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
using SimConsFactory = Lisp::SimConsFactory;
using SimConsFactoryRecord = Lisp::SimConsFactoryRecord;
const unsigned short SimConsFactory::carIndex = 1;
const unsigned short SimConsFactory::cdrIndex = 2;

SimConsFactory::SimConsFactory()
{
}

void SimConsFactory::setTargetNumRootConses(std::size_t v)
{
  targetNumRootConses = v;
}

std::size_t SimConsFactory::getTargetNumRootConses() const
{
  return targetNumRootConses;
}

void SimConsFactory::setTargetNumBulkConses(std::size_t v)
{
  targetNumBulkConses = v;
}
    
std::size_t SimConsFactory::getTargetNumBulkConses() const
{
  return targetNumBulkConses;
}

void SimConsFactory::setTargetEdgeFraction(double f)
{
  targetEdgeFraction = f;
}

double SimConsFactory::getTargetEdgeFraction() const
{
  return targetEdgeFraction;
}

void SimConsFactory::setNumSteps(std::size_t n)
{
  numSteps = n;
}

std::size_t SimConsFactory::getNumSteps() const
{
  return numSteps;
}

void SimConsFactory::setNumEdgeRewireSteps(std::size_t _numEdgeRewireSteps)
{
  numEdgeRewireSteps = _numEdgeRewireSteps;
}

std::size_t SimConsFactory::getNumEdgeRewireSteps() const
{
  return numEdgeRewireSteps;
}

void SimConsFactory::setNumBulkConsesSteps(std::size_t _numBulkConsesSteps)
{
  numBulkConsesSteps = _numBulkConsesSteps;
}

std::size_t SimConsFactory::getNumBulkConsesSteps() const
{
  return numBulkConsesSteps;
}

void SimConsFactory::setGarbageSteps(std::size_t _garbageSteps)
{
  garbageSteps = _garbageSteps;
}

std::size_t SimConsFactory::getGarbageSteps() const
{
  return garbageSteps;
}

void SimConsFactory::setRecycleSteps(std::size_t _recycleSteps)
{
  recycleSteps = _recycleSteps;
}

std::size_t SimConsFactory::getRecycleSteps() const
{
  return recycleSteps;
}


double SimConsFactory::getTargetNumEdges(std::shared_ptr<ConsFactory> factory,
                                         const ConsGraph & graph) const
{
  std::size_t numRootConses = factory->numRootConses();
  std::size_t numConses = graph.numNodes()-1;
  return getTargetEdgeFraction() *
    ( 2.0 * numRootConses + numConses ) +
    numRootConses + numConses;
}

double SimConsFactory::getTargetNumEdges(std::shared_ptr<ConsFactory> factory) const
{
  ConsGraph graph(*factory);
  return getTargetNumEdges(factory, graph);
}

double SimConsFactory::getEdgeFraction(std::shared_ptr<ConsFactory> factory,
                                       const Lisp::ConsGraph & graph) const
{
  std::size_t maxEdges = factory->numRootConses() + 2 * (graph.numNodes()-1);
  std::size_t minEdges = graph.numNodes()-1;
  if(maxEdges - minEdges)
  {
    return (double)(graph.numEdges() - minEdges) / (maxEdges - minEdges);
  }
  else
  {
    return 0;
  }
}

double SimConsFactory::getEdgeFraction(std::shared_ptr<ConsFactory> factory) const
{
  ConsGraph graph(*factory);
  return getEdgeFraction(factory, graph);
}

std::vector<SimConsFactoryRecord> SimConsFactory::run()
{
  std::vector<SimConsFactoryRecord> ret;
  auto factory = std::make_shared<ConsFactory>(100,
                                               garbageSteps,
                                               recycleSteps);
  {
    std::list<SharedObject> rootConses;
    for(std::size_t i = 0; i < numSteps; i++)
    {
      SimConsFactoryRecord rec;
      stepRootConses(factory, rootConses);
      for(std::size_t j = 0; j < numBulkConsesSteps; j++)
      {
        stepConses(factory, rootConses);
      }
      for(std::size_t j = 0; j < numEdgeRewireSteps; j++)
      {
        stepEdge(factory);
      }
      ConsGraph graph(*factory);
      rec.step = i;
      rec.numRootConses = factory->numRootConses();
      rec.numBulkConses = (graph.numNodes()-1) - factory->numRootConses();
      rec.numReachableConses = (graph.numNodes()-1);
      rec.numVoidConses = factory->numConses(Color::Void);
      rec.numFreeConses = factory->numConses(Color::Free);
      rec.numEdges = graph.numEdges();
      rec.expectedNumEdges = getTargetNumEdges(factory, graph);
      rec.edgeFraction = getEdgeFraction(factory, graph);
      ret.push_back(rec);
    }
  }
  return ret;
}

void Lisp::SimConsFactory::stepRootConses(std::shared_ptr<ConsFactory> factory,
                                          std::list<SharedObject> & rootConses)
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
Lisp::SimConsFactory::getFreeEdges(std::shared_ptr<ConsFactory> factory)
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

void Lisp::SimConsFactory::stepConses(std::shared_ptr<ConsFactory> factory,
                                      std::list<SharedObject> & rootConses)
{
  using Cons = Lisp::Cons;
  using Nil = Lisp::Nil;
  using ConsPair = std::pair<Cons*, unsigned short>;
  auto freeEdges = getFreeEdges(factory);
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

void SimConsFactory::stepEdge(std::shared_ptr<ConsFactory> factory)
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
      auto freeEdges = getFreeEdges(factory);
      if(!freeEdges.empty() && graph.numNodes())
      {
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
  }
  if(num_edges > min_edges)
  {
    if( (num_edges > target_num_edges && (float) rand() / (float) RAND_MAX < acceptanceRate) ||
        (float) rand() / (float) RAND_MAX < errorAcceptanceRate )
    {
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
