/*****************************************************************************
Filename: OCalling.cpp
Date    : 
Description: Random generation of function call graph by obfuscating functions inline and outline.
			 1. Random generation of function call graph
			 2. Inline obfuscation based on function call graph.
			 3. Outline obfuscation based on function call graph.
*****************************************************************************/
#include "OCalling.h"

int main(int argc, char** argv)
{
	if (argc < 3) {
		errs() << "Usage::OCalling.exe [input].bc [output].bc";
		return -1;
	}
	cl::ParseCommandLineOptions(argc, argv, "global analysis\n");
	string OriginalFile = argv[1];
	string TransformedFile = argv[2];
	Module* Mod = MyParseIRFile(OriginalFile);
	if (Mod == nullptr)
	{
		errs() << "Failed to read input file\n";
		return -1;
	}
	//Random generation of function call graph.
	CallGraphRandom doCallGraphRandom;
	errs() << "Start to Generate Erdos-Renyi Call Graph...\n";
	doCallGraphRandom.ERProb = defaultObfRate;             
	doCallGraphRandom.GenCallGraph(Mod);
	//Inline obfuscation based on function call graph.
	OutLine doOutLine;
	errs() << "Start to do OutLine Obfuscation...\n";
	doOutLine.OutLineFuncList.clear();
	doOutLine.OutLineFuncList = doCallGraphRandom.doOutLineList;
	doOutLine.OutLineObf();
	//Outline obfuscation based on function call graph.
	InLine doInLine;
	errs() << "Start to do InLine Obfuscation...\n";
	doInLine.InLineFuncList.clear();
	doInLine.InLineFuncList = doCallGraphRandom.doInLineList;
	doInLine.InLineObf();
	errs() << "The Number of Obfuscation Functions is [" << doInLine.InLineFuncList.size() + doOutLine.OutLineFuncList.size() << "]\n";	
	if (!doWriteBackLL(Mod, TransformedFile))
	{
		errs() << "Faile to Write IR to file...\n";
		return -1;
	}
	return 0;
} 



