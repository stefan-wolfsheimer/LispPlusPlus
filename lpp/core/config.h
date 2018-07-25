#include <iostream>
#ifdef NDEBUG
#define DEBUG(EXPR)
#define LLOG(EXPR)
#else
#define DEBUG(EXPR) EXPR
#define LLOG(EXPR) (std::cout << __FILE__ << ":" << __LINE__ << "\t" << EXPR << std::endl)
#endif

