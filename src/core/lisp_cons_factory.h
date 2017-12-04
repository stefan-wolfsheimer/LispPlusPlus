/******************************************************************************
Copyright (c) 2017, Stefan Wolfsheimer

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
#include <vector>
#include "lisp_i_cons_factory.h"

// @todo move to config.h
#define CONS_PAGE_SIZE 512

namespace Lisp
{
  class Object;
  class ConsFactory : public IConsFactory
  {
  public:
    using Color = IConsFactory::Color;
    ConsFactory(std::size_t _pageSize=CONS_PAGE_SIZE);
    ~ConsFactory();
    Cons * make(const Object & car,
                const Object & cdr) override;
    void unroot(Cons * cons) override;
    std::size_t numConses(Color color) const override;
    Color encodeColor(unsigned char code) const override;
    unsigned char decodeColor(Color color) const override;
  private:
    inline void initChild(Object & obj, const Object & rhs);
    Color code2color[5];
    unsigned int color2code[5];

    std::size_t pageSize;
    std::vector<Cons*> pages;
    std::vector<Cons*> freeConses;

    /*******************
     +---------------+
     | white conses  |
     +---------------+ whiteTop
     | grey conses   |
     +---------------+ greyTop
     | black conses  |
     +---------------+ size()
     *******************/
    std::size_t whiteTop;
    std::size_t greyTop;
    std::vector<Cons*> conses;
  };
}

