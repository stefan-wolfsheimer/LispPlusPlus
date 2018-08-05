#include <random>
#include <lpp/simul/gc_sim.h>
#include <cli.h>

struct GcSimRunner
{
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

  void run()
  {
    for(std::size_t i = 0; i < numSteps; i++)
    {
    }
  }
};

int main(int argc, const char ** argv)
{
  unsigned seed = 1.0;
  Lisp::GcSim sim();
  GcSimRunner runner;


  std::default_random_engine generator (seed);

  std::poisson_distribution<int> distribution (5.2);

  std::cout << "some Poisson-distributed results (mean=5.2): ";
  for (int i=0; i<10; ++i)
    std::cout << distribution(generator) << " ";
  std::cout << std::endl;
  runner.run();
  return 0;
}
