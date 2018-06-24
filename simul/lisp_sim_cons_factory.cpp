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
#include <lpp/core/gc/garbage_collector.h>
#include <lpp/core/object.h>
#include <lpp/core/types/cons.h>

#include <lpp/core/gc/collectible_graph.h>
#include <lpp/core/gc/collectible_node.h>
#include <lpp/core/gc/collectible_edge.h>


using GarbageCollector = Lisp::GarbageCollector;
using Color = Lisp::Color;
using CollectibleGraph = Lisp::CollectibleGraph;
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


double SimConsFactory::getTargetNumEdges(std::shared_ptr<GarbageCollector> factory,
                                         const CollectibleGraph & graph) const
{
  // f = (n - min_edges) / (max_edges - min_edges)
  // f * (max_edges - min_edges) = n - min_edges
  // n = f * (max_edges - min_edges) + min_edges
  std::size_t maxEdges = factory->numRootCollectible() + 2 * (graph.numNodes()-1);
  std::size_t minEdges = graph.numNodes()-1;
  return getTargetEdgeFraction() * (maxEdges - minEdges) + minEdges;
}

double SimConsFactory::getTargetNumEdges(std::shared_ptr<GarbageCollector> factory) const
{
  CollectibleGraph graph(*factory);
  return getTargetNumEdges(factory, graph);
}

double SimConsFactory::getEdgeFraction(std::shared_ptr<GarbageCollector> factory,
                                       const Lisp::CollectibleGraph & graph) const
{
  // min_edges = n_root_conses + n_conses
  // max_edges = n_root_conses + 2 * (n_conses + n_root_conses)
  // f = (n - min_edges) / (max_edges - min_edges)
  std::size_t maxEdges = factory->numRootCollectible() + 2 * (graph.numNodes()-1);
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

double SimConsFactory::getEdgeFraction(std::shared_ptr<GarbageCollector> factory) const
{
  CollectibleGraph graph(*factory);
  return getEdgeFraction(factory, graph);
}

std::vector<SimConsFactoryRecord> SimConsFactory::run()
{
  std::vector<SimConsFactoryRecord> ret;
  auto factory = std::make_shared<GarbageCollector>(100,
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
      CollectibleGraph graph(*factory);
      rec.step = i;
      rec.numRootConses = factory->numRootCollectible();
      rec.numBulkConses = (graph.numNodes()-1) - factory->numRootCollectible();
      rec.numReachableConses = (graph.numNodes()-1);
      rec.numVoidConses = factory->numVoidCollectible();
      rec.numFreeConses = factory->numDisposedCollectible();
      rec.numEdges = graph.numEdges();
      rec.expectedNumEdges = getTargetNumEdges(factory, graph);
      rec.edgeFraction = getEdgeFraction(factory, graph);
      ret.push_back(rec);
    }
  }
  return ret;
}

void Lisp::SimConsFactory::stepRootConses(std::shared_ptr<GarbageCollector> factory,
                                          std::list<SharedObject> & rootConses)
{
  if(selectAddCons(factory->numRootCollectible(), targetNumRootConses))
  {
    rootConses.push_back(std::make_shared<Object>(factory->makeCons(Lisp::nil, Lisp::nil)));
  }
  else if(selectRemoveCons(factory->numRootCollectible(), targetNumRootConses))
  {
    assert(factory->numRootCollectible() == rootConses.size());
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
Lisp::SimConsFactory::getFreeEdges(std::shared_ptr<GarbageCollector> coll)
{
  using ConsPair = std::pair<Cons*, unsigned short>;
  using Cons = Lisp::Cons;
  using Nil = Lisp::Nil;
  std::vector<ConsPair> freeEdges;
  coll->forEachReachable([&freeEdges](const Cell & cell) {
      if(cell.isA<Cons>())
      {
        if(cell.as<Cons>()->getCarCell().isA<Nil>())
        {
          freeEdges.push_back(ConsPair(cell.as<Cons>(), carIndex));
        }
        if(cell.as<Cons>()->getCdrCell().isA<Nil>())
        {
          freeEdges.push_back(ConsPair(cell.as<Cons>(), cdrIndex));
        }
      }
  });
  return freeEdges;
}

void Lisp::SimConsFactory::stepConses(std::shared_ptr<GarbageCollector> coll,
                                      std::list<SharedObject> & rootConses)
{
  using Cons = Lisp::Cons;
  using Nil = Lisp::Nil;
  using ConsPair = std::pair<Cons*, unsigned short>;
  auto freeEdges = getFreeEdges(coll);
  std::vector<Cons *> nonRootConses;
  coll->forEachReachable([&nonRootConses](const Cell & cell) {
      if(cell.isA<Cons>() && !cell.as<Cons>()->isRoot())
      {
        nonRootConses.push_back(cell.as<Cons>());
      }
    });
  if(freeEdges.size() > 0 && selectAddCons(nonRootConses.size(),
                                           targetNumBulkConses))
  {
    std::size_t i = rand() % freeEdges.size();
    if(freeEdges[i].second == carIndex)
    {
      freeEdges[i].first->setCar(Object(coll->makeCons(Lisp::nil, Lisp::nil)));
    }
    else
    {
      freeEdges[i].first->setCdr(Object(coll->makeCons(Lisp::nil, Lisp::nil)));
    }
  }
  else if(selectRemoveCons(nonRootConses.size(), targetNumBulkConses))
  {
    CollectibleGraph graph(*coll);
    std::size_t i = rand() % nonRootConses.size();
    auto cons = nonRootConses[i];
    auto node = graph.findNode(cons);
    for(auto parent : node->getParents())
    {
      auto pcons = parent.as<Cons>();
      if(pcons->getCarCell().as<Cons>() == cons)
      {
        pcons->unsetCar();
      }
      if(pcons->getCdrCell().as<Cons>() == cons)
      {
        pcons->unsetCdr();
      }
    }
  }
}

void SimConsFactory::stepEdge(std::shared_ptr<GarbageCollector> factory)
{
  static float acceptanceRate = 0.75f;
  static float errorAcceptanceRate = 0.25f;
  CollectibleGraph graph(*factory);
  std::size_t n_root_conses = factory->numRootCollectible();
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
        auto cell = node->getCell();
        if(cell.isA<Cons>())
        {
          if(freeEdges[i].second == carIndex)
          {
            freeEdges[i].first->setCar(cell.as<Cons>());
          }
          else
          {
            freeEdges[i].first->setCdr(cell.as<Cons>());
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
        auto parent = edge->getParent();
        auto child = edge->getChild();
        if(parent.isA<Cons>() && child.isA<Cons>())
        {
          if(parent.as<Cons>()->getCarCell() == child)
          {
            parent.as<Cons>()->unsetCar();
          }
          else if(parent.as<Cons>()->getCdrCell() == child)
          {
            parent.as<Cons>()->unsetCdr();
          }
        }
      }
    }
  }
}
