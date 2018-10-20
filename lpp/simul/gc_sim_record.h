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

namespace Lisp
{
  struct GcSimRecord
  {
    std::size_t step;
    std::size_t numRoot;
    std::size_t numBulk;
    std::size_t numTotal;
    std::size_t numFreeChildren;
    std::size_t numVoid;
    std::size_t numDisposed;
    std::size_t numEdges;
    double expectedNumEdges;
    double edgeFraction;
  };

  class GcSimFieldBase
  {
  public:
    GcSimFieldBase(const std::string & _name) : name(_name) {}
    virtual ~GcSimFieldBase() {}

    const std::string& getName() const
    {
      return name;
    }
  private:
    std::string name;
  };

  template<typename T>
  struct GcSimField : public GcSimFieldBase
  {
    typedef T member_type;
    typedef member_type GcSimRecord::*pointer_to_member_type;
    GcSimField(pointer_to_member_type member, const std::string & name) : GcSimFieldBase(name)
    {}
  };

  struct GcSimRecordFields
  {
    std::vector<GcSimFieldBase*> fields;

    GcSimRecordFields()
    {
      fields.push_back(new GcSimField<std::size_t>(&GcSimRecord::step, "step"));
      fields.push_back(new GcSimField<std::size_t>(&GcSimRecord::numRoot, "numRoot"));
      fields.push_back(new GcSimField<std::size_t>(&GcSimRecord::numBulk, "numBulk"));
      fields.push_back(new GcSimField<std::size_t>(&GcSimRecord::numTotal, "numTotal"));
      fields.push_back(new GcSimField<std::size_t>(&GcSimRecord::numFreeChildren, "numFreeChildren"));
      fields.push_back(new GcSimField<std::size_t>(&GcSimRecord::numVoid, "numVoid"));
      fields.push_back(new GcSimField<std::size_t>(&GcSimRecord::numDisposed, "numDisposed"));
      fields.push_back(new GcSimField<std::size_t>(&GcSimRecord::numEdges, "numEdges"));
      fields.push_back(new GcSimField<double>(&GcSimRecord::expectedNumEdges, "expectedNumEdges"));
      fields.push_back(new GcSimField<double>(&GcSimRecord::edgeFraction, "edgeFraction"));
    }

    ~GcSimRecordFields()
    {
      for(auto f : fields)
      {
        delete f;
      }
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
        ost << "f";
      }
    }
  };
}

inline std::ostream & operator<<(std::ostream & ost, const std::vector<Lisp::GcSimRecord> & series)
{
  Lisp::GcSimRecordFields fields;
  fields.streamHeader(ost);
  for(auto rec : series)
  {
    ost << std::endl;
    fields.streamRecord(ost, rec);
  }
  return ost;
}
