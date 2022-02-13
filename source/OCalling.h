#include "rwBcFile.h"
#include "Common.h"
#include "CallGraphRandom.h"
#include "InLine.h"
#include "OutLine.h"

static cl::list<std::string>
InputFilenames(cl::Positional, cl::OneOrMore,
	cl::desc("<input bitcode files>"));             

int defaultObfRate = 80;

