#include <random>
#include <lpp/simul/gc_sim_runner.h>
#include <cli.h>

int main(int argc, const char ** argv)
{
  unsigned seed = 1.0;
  std::default_random_engine generator (seed);
  Lisp::GcSimRunner<std::default_random_engine> runner(generator);
  std::vector<Lisp::GcSimRecord> run = runner.run();
  std::cout << run << std::endl;
  return 0;
}
