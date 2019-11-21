#include <iostream>

#define DO_ASM_LOG

#ifdef NDEBUG
#define DEBUG(EXPR)
#define LLOG(EXPR)
#else
#define DEBUG(EXPR) EXPR
#define LLOG(EXPR) (std::cout << __FILE__ << ":" << __LINE__ << "\t" << EXPR << std::endl)
#endif

#ifdef DO_ASM_LOG
#define ASM_LOG(ARG) { std::cout << __FILE__ << ":" << __LINE__ << "\t" << ARG << std::endl; }
#else
#define ASM_LOG(ARG)
#endif


