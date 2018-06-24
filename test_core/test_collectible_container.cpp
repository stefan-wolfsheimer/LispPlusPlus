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
#include <catch.hpp>
#include <lpp/core/gc/collectible_container.h>
#include <lpp/core/gc/unmanaged_collectible_container.h>
#include <lpp/core/types/collectible.h>

using Color = Lisp::Color;
using Collectible = Lisp::Collectible;

struct Entity : public Collectible
{
};

using CollectibleContainer = Lisp::CollectibleContainer<Entity>;
using UnmanagedCollectibleContainer = Lisp::UnmanagedCollectibleContainer<Entity>;


TEST_CASE("collectible_container_life_cycle", "[CollectibleContainer]")
{
  CollectibleContainer container(Color::Black, true, nullptr);
  REQUIRE(container.empty());
  REQUIRE(container.size() == 0);
  REQUIRE(container.getColor() == Color::Black);
  Entity ent1;
  Entity ent2;
  Entity ent3;
  container.add(&ent1);
  container.add(&ent2);
  container.add(&ent3);
  REQUIRE(container.size() == 3);
  REQUIRE(ent1.getColor() == Color::Black);
  REQUIRE(ent1.getIndex() == 0u);
  REQUIRE(ent2.getColor() == Color::Black);
  REQUIRE(ent2.getIndex() == 1u);
  REQUIRE(ent3.getColor() == Color::Black);
  REQUIRE(ent3.getIndex() == 2u);
  container.remove(&ent1);
  REQUIRE(container.size() == 2);
  REQUIRE(ent2.getColor() == Color::Black);
  REQUIRE(ent2.getIndex() == 1u);
  REQUIRE(ent3.getColor() == Color::Black);
  REQUIRE(ent3.getIndex() == 0u);
  REQUIRE(container.popBack() == &ent2);
  REQUIRE(container.size() == 1);
  REQUIRE(container.popBack() == &ent3);
  REQUIRE(container.size() == 0);
}

TEST_CASE("move_collectible_container_to_unmanaged", "[CollectibleContainer]")
{
  CollectibleContainer container(Color::Black, true, nullptr);
  UnmanagedCollectibleContainer unmanaged;
  Entity ent1;
  Entity ent2;
  Entity ent3;
  unmanaged.move(container);
  REQUIRE(container.empty());
  container.add(&ent1);
  container.add(&ent2);
  unmanaged.move(container);
  REQUIRE(container.empty());
  container.add(&ent3);
  unmanaged.move(container);
  REQUIRE(container.empty());
  REQUIRE(!unmanaged.empty());
  REQUIRE(unmanaged.popBack() == &ent3);
  REQUIRE(!unmanaged.empty());
  REQUIRE(unmanaged.popBack() == &ent2);
  REQUIRE(!unmanaged.empty());
  REQUIRE(unmanaged.popBack() == &ent1);
  REQUIRE(unmanaged.empty());
}
