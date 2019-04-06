/******************************************************************************
Copyright (c) 2019, Stefan Wolfsheimer

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
#include <functional>
#include <catch.hpp>
#include <lpp/core/vm.h>
#include <lpp/core/types/lisp_string.h>
#include <lpp/core/types/forms/choice_of.h>
#include <lpp/core/types/forms/type_of.h>

using Vm = Lisp::Vm;
using Object = Lisp::Object;
using Cell = Lisp::Cell;
using String = Lisp::String;
using Form = Lisp::Form;
using IntegerType = Lisp::IntegerType;
using StringForm = Lisp::TypeOf<String>;
using IntegerForm = Lisp::TypeOf<IntegerType>;
using ChoiceOf = Lisp::ChoiceOf;


TEST_CASE("choice_of", "[Form]")
{
  Vm vm;
  const Form * matchedForm = nullptr;
  Object strForm = vm.make<StringForm>([&matchedForm](const Form * form, const Cell & cell){
      matchedForm = form; });
  Object intForm = vm.make<IntegerForm>([&matchedForm](const Form * form, const Cell & cell){
      matchedForm = form; });
  Object choiceForm = vm.make<ChoiceOf>(std::vector<Form*>{
      strForm.as<Form>(),
      intForm.as<Form>()
    });
  Object str = vm.make<String>("hello");
  Object intval(1);
  
  REQUIRE(choiceForm.as<Form>()->isInstance(str));
  REQUIRE(choiceForm.as<Form>()->isInstance(intval));
  REQUIRE(choiceForm.as<Form>()->match(str));
  REQUIRE(choiceForm.as<Form>()->match(intval));

  {
    matchedForm = nullptr;
    REQUIRE_FALSE(choiceForm.as<Form>()->isInstance(Lisp::nil));
    REQUIRE_FALSE(choiceForm.as<Form>()->match(Lisp::nil));
    REQUIRE(matchedForm == nullptr);
  }
  {
    matchedForm = nullptr;
    REQUIRE(choiceForm.as<Form>()->isInstance(str));
    REQUIRE(choiceForm.as<Form>()->match(str));
    REQUIRE(matchedForm == strForm.as<Form>());
  }

  {
    matchedForm = nullptr;
    REQUIRE(choiceForm.as<Form>()->isInstance(intval));
    REQUIRE(choiceForm.as<Form>()->match(intval));
    REQUIRE(matchedForm == intForm.as<Form>());
  }
}
