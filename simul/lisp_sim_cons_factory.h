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
//@todo rename to garbagbe collector
#pragma once
#include "lisp_sim_cons_factory_record.h"
#include <cstddef>
#include <memory>
#include <list>
#include <vector>

namespace Lisp
{
  class CollectibleGraph;
  class GarbageCollector;

  class SimConsFactory
  {
  public:
    SimConsFactory();
    std::vector<SimConsFactoryRecord> run();

    void setTargetNumRootConses(std::size_t v);
    std::size_t getTargetNumRootConses() const;

    void setTargetNumBulkConses(std::size_t v);
    std::size_t getTargetNumBulkConses() const;

    void setTargetEdgeFraction(double targetEdgeFraction);
    double getTargetEdgeFraction() const;

    void setNumSteps(std::size_t numSteps);
    std::size_t getNumSteps() const;

    void setNumEdgeRewireSteps(std::size_t numEdgeRewire);
    std::size_t getNumEdgeRewireSteps() const;

    void setNumBulkConsesSteps(std::size_t numBulkConsesSteps);
    std::size_t getNumBulkConsesSteps() const;

    void setGarbageSteps(std::size_t garbageSteps);
    std::size_t getGarbageSteps() const;

    void setRecycleSteps(std::size_t recycleSteps);
    std::size_t getRecycleSteps() const;

    double getTargetNumEdges(std::shared_ptr<GarbageCollector> factory,
                             const Lisp::CollectibleGraph & graph) const;
    double getTargetNumEdges(std::shared_ptr<GarbageCollector> factory) const;
    double getEdgeFraction(std::shared_ptr<GarbageCollector> factory,
                           const Lisp::CollectibleGraph & graph) const;
    double getEdgeFraction(std::shared_ptr<GarbageCollector> factory) const;
  private:
    using SharedObject = std::shared_ptr<Object>;

    std::size_t targetNumRootConses = 10;
    std::size_t targetNumBulkConses = 100;
    double targetEdgeFraction = 0.5;
    std::size_t numEdgeRewireSteps = 1;
    std::size_t numBulkConsesSteps = 3;
    std::size_t garbageSteps = 1; // parameters for ConsFactory
    std::size_t recycleSteps = 1;

    // min_edges = n_root_conses + n_conses = 3
    // max_edges = n_root_conses + 2 * (n_root_conses + n_conses ) = 7
    // edge_fraction = (edges - min_edges) / (max_edges - min_edges)
    //               = (edges - n_root_conses - n_conses) / (2 * n_root_conses + n_conses)
    // edges = f * (2 * n_root_conses + n_conses) + n_root_conses + n_conses
    /*
             |
             o
            / \
           o   o

     */
    std::size_t numSteps = 100;
    static const unsigned short carIndex;
    static const unsigned short cdrIndex;

    std::vector<std::pair<Cons*, unsigned short> > getFreeEdges(std::shared_ptr<GarbageCollector> factory);
    void stepRootConses(std::shared_ptr<GarbageCollector> factory,
                        std::list<SharedObject> & rootConses);
    void stepConses(std::shared_ptr<GarbageCollector> factory,
                    std::list<SharedObject> & rootConses);
    void stepEdge(std::shared_ptr<GarbageCollector> factory);
    bool selectAddCons(std::size_t numberOfConses, std::size_t target);
    bool selectRemoveCons(std::size_t numberOfConses, std::size_t target);
  };
}
