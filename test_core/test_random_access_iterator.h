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
#pragma once
#include <catch.hpp>
#include <vector>
#include <iostream>

template<typename ITR>
bool testRandomAccessIterator(const std::vector<typename ITR::value_type> & data,
                              ITR begin,
                              ITR end)
{
  using iterator = ITR;
  if(data.empty())
  {
    REQUIRE(begin == end);
    return true;
  }
  else
  {
    REQUIRE(*begin == *data.begin());
    {
      iterator itr = begin;
      for(std::size_t i = 1; i < data.size(); ++i)
      {
        REQUIRE(*++itr == data[i]);
      }
      REQUIRE(++itr == end);
    }
    {
      iterator itr = begin;
      for(std::size_t i = 0; i < data.size(); ++i)
      {
        REQUIRE(*itr++ == data[i]);
      }
      REQUIRE(itr == end);
    }
    {
      iterator itr = end;
      for(std::size_t i = 1; i < data.size(); ++i)
      {
        REQUIRE(*(--itr) == data[data.size()-i]);
      }
      REQUIRE(--itr == begin);
    }
    {
      iterator itr = end;
      REQUIRE(itr-- == end);
      for(std::size_t i = 1; i < data.size(); ++i)
      {
        REQUIRE(*(itr--) == data[data.size()-i]);
      }
      REQUIRE(itr == begin);
    }
    {
      iterator itr = begin;
      for(std::size_t i = 0; i < data.size(); i++)
      {
        iterator jtr = itr;
        REQUIRE(itr == jtr);
        REQUIRE(itr <= jtr);
        REQUIRE(itr >= jtr);
        REQUIRE(begin[i] == *itr);
        ++jtr;
        for(std::size_t j = i + 1; j < data.size(); j++)
        {
          REQUIRE(itr != jtr);
          REQUIRE(itr <= jtr);
          REQUIRE(itr <  jtr);
          REQUIRE(jtr >= itr);
          REQUIRE(jtr >  itr);
          REQUIRE(itr + (j-i) == jtr);
          REQUIRE(jtr - (j-i) == itr);
          REQUIRE(itr[j - i] == *jtr);
          REQUIRE(jtr[i - j] == *itr);
          REQUIRE((jtr - itr) == (j-i));
          REQUIRE((itr - jtr) == (i-j));
          ++jtr;
        }
        ++itr;
      }
      REQUIRE(itr == end);
    }
  }
  return true;
}

