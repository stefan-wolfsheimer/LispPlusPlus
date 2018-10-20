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
#pragma once
#include <lpp/simul/gc_sim.h>
#include <lpp/simul/gc_sim_record.h>
#include <vector>

namespace Lisp
{
  template<typename RNGE>
  class GcSimRunner
  {
  public:
    using random_engine = RNGE;
    GcSimRunner(const random_engine & _generator);
    std::vector<Lisp::GcSimRecord> run();

  private:
    random_engine generator;
    Lisp::GcSim sim;

    double meanNumberOfCells = 4;

    std::size_t targetNumRoot = 10;
    std::size_t targetNumBulk = 10;
    double targetChildFraction = 0.5;
    std::size_t numEdgeRewireSteps = 5;
    std::size_t numBulkSteps = 3;
    std::size_t garbageSteps = 1; // parameters for GarbageCollector
    std::size_t recycleSteps = 1;
    std::size_t numSteps = 100;

    void stepRoot();
    void stepBulk();
    void stepRewire();
    bool selectAdd(std::size_t n, std::size_t target);
    bool selectRemove(std::size_t n, std::size_t target);
    std::size_t getTargetNumChildren();
  };
}

template<typename RNGE>
Lisp::GcSimRunner<RNGE>::GcSimRunner(const random_engine & _generator)
  : generator(_generator)
{
}

template<typename RNGE>
std::vector<Lisp::GcSimRecord> Lisp::GcSimRunner<RNGE>::run()
{
  std::vector<Lisp::GcSimRecord> ret;

  for(std::size_t i = 0; i < numSteps; i++)
  {
    stepRoot();
    for(std::size_t i = 0; i < numBulkSteps; i++)
    {
      stepBulk();
    }
    for(std::size_t i = 0; i < numEdgeRewireSteps; i++)
    {
      stepRewire();
    }
    GcSimRecord rec;
    rec.step = i;
    rec.numRoot = sim.numRoot();
    rec.numBulk = sim.numBulk();
    rec.numTotal = sim.numTotal();
    rec.numLeaves = sim.numLeaves();
    rec.numVoid = sim.numVoid();
    rec.numDisposed = sim.numDisposed();
    rec.numEdges = sim.numEdges();
    std::size_t nchildren = sim.numEdges() + sim.numLeaves();
    if(nchildren)
    {
      rec.edgeFraction = (double)sim.numEdges() / nchildren;
    }
    else
    {
      rec.edgeFraction = 0.0;
    }
    ret.push_back(rec);
  }
  return ret;
}

template<typename RNGE>
void Lisp::GcSimRunner<RNGE>::stepRoot()
{
  std::poisson_distribution<int> cellDistribution(meanNumberOfCells);
  if(selectAdd(sim.numRoot(), targetNumRoot))
  {
    std::size_t ncells = cellDistribution(generator);
    sim.addRoot(ncells);
  }
  else if(selectRemove(sim.numRoot(), targetNumRoot))
  {
    assert(sim.numRoot() > 0);
    std::uniform_int_distribution<int> distribution(0, sim.numRoot()-1);
    sim.unroot(distribution(generator));
  }
}

template<typename RNGE>
void Lisp::GcSimRunner<RNGE>::stepBulk()
{
  std::poisson_distribution<int> cellDistribution(meanNumberOfCells);
  if(sim.numLeaves() > 0 && selectAdd(sim.numBulk(), targetNumBulk))
  {
    std::size_t ncells = cellDistribution(generator);
    std::uniform_int_distribution<int> distribution(0, sim.numLeaves() - 1);
    sim.addBulk(distribution(generator), ncells);
  }
  else if(selectRemove(sim.numBulk(), targetNumBulk))
  {
    assert(sim.numBulk() > 0);
    std::uniform_int_distribution<int> distribution(0, sim.numBulk()-1);
    auto parents = sim.getParentIndices(distribution(generator));
    for(auto & p: parents)
    {
      sim.removeEdge(p.first, p.second);
    }
  }
}

template<typename RNGE>
std::size_t Lisp::GcSimRunner<RNGE>::getTargetNumChildren()
{
  std::size_t maxChildren = sim.numEdges() + sim.numLeaves();
  std::size_t minChildren = sim.numBulk();
  // f = (n - min_children) / (max_children - min_children)
  // f * (max_children -  min_children) = n - min_children
  // n = f * (max_children - min_children) + min_children
  return targetChildFraction * (maxChildren - minChildren) + minChildren;
}

template<typename RNGE>
void Lisp::GcSimRunner<RNGE>::stepRewire()
{
  // return true with p=0.75 if n < target
  static std::bernoulli_distribution accDistribution(0.75f);
  // return true with p=0.25 if n >= target
  static std::bernoulli_distribution rejDistribution(0.25f);
  if(sim.numLeaves() && sim.numTotal())
  {
    if((getTargetNumChildren() > sim.numEdges() && accDistribution(generator)) || rejDistribution(generator))
    {
      sim.addEdge(std::uniform_int_distribution<int>(0, sim.numLeaves() - 1)(generator),
                  std::uniform_int_distribution<int>(0, sim.numTotal() - 1)(generator));
    }
  }
  if(sim.numEdges())
  {
    if((getTargetNumChildren() < sim.numEdges() && accDistribution(generator)) || rejDistribution(generator))
    {
      sim.removeEdge(std::uniform_int_distribution<int>(0, sim.numEdges() - 1)(generator));
    }
  }
}

template<typename RNGE>
bool Lisp::GcSimRunner<RNGE>::selectAdd(std::size_t n, std::size_t target)
{
  // return true with p=0.75 if n < target
  static std::bernoulli_distribution accDistribution(0.75f);
  // return true with p=0.25 if n >= target
  static std::bernoulli_distribution rejDistribution(0.25f);
  if(n == 0)
  {
    return true;
  }
  else if(n < target)
  {
    return accDistribution(generator);
  }
  else
  {
    return rejDistribution(generator);
  }
}

template<typename RNGE>
bool Lisp::GcSimRunner<RNGE>::selectRemove(std::size_t n, std::size_t target)
{
  // return true with p=0.75 if n >= target
  static std::bernoulli_distribution accDistribution(0.75f);
  // return true with p=0.25 if n < target
  static std::bernoulli_distribution rejDistribution(0.25f);
  if(n == 0)
  {
    return false;
  }
  else if(n >= target)
  {
    return accDistribution(generator);
  }
  else
  {
    return rejDistribution(generator);
  }
}
