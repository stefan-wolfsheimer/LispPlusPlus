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
#include <lpp/simul/sim_member.h>

namespace Lisp
{
  struct GcSimRecord
  {
    std::size_t step;
    std::size_t numRoot;
    std::size_t numBulk;
    std::size_t numTotal;
    std::size_t numLeaves;
    std::size_t numVoid;
    std::size_t numDisposed;
    std::size_t numEdges;
    double edgeFraction;
  };

  struct GcSimRecordMembers
  {
    std::vector<std::shared_ptr<SimulMemberBase<GcSimRecord>>> fields;

    GcSimRecordMembers()
    {
      addField(&GcSimRecord::step, "step");
      addField(&GcSimRecord::numRoot, "numRoot");
      addField(&GcSimRecord::numBulk, "numBulk");
      addField(&GcSimRecord::numTotal, "numTotal");
      addField(&GcSimRecord::numLeaves, "numLeaves");
      addField(&GcSimRecord::numVoid, "numVoid");
      addField(&GcSimRecord::numDisposed, "numDisposed");
      addField(&GcSimRecord::numEdges, "numEdges");
      addField(&GcSimRecord::edgeFraction, "edgeFraction");
    }

    template<typename T>
    inline void addField(T GcSimRecord::*member, const std::string & name)
    {
      fields.push_back(std::make_shared<SimulMember<GcSimRecord, T>>(member, name));
    }

    void streamHeader(std::ostream & ost)
    {
      bool first = true;
      for(auto f : fields)
      {
        if(first)
        {
          first = false;
        }
        else
        {
          ost << ",";
        }
        ost << f->getName();
      }
    }

    void streamRecord(std::ostream & ost, const GcSimRecord & rec)
    {
      bool first = true;
      for(auto f : fields)
      {
        if(first)
        {
          first = false;
        }
        else
        {
          ost << ",";
        }
        f->stream(ost, &rec);
      }
    }
  };
}

inline std::ostream & operator<<(std::ostream & ost, const std::vector<Lisp::GcSimRecord> & series)
{
  Lisp::GcSimRecordMembers fields;
  fields.streamHeader(ost);
  for(auto rec : series)
  {
    ost << std::endl;
    fields.streamRecord(ost, rec);
  }
  return ost;
}
