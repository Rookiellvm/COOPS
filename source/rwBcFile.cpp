/*****************************************************************************
Filename: rwBcFile.cpp
Date    : 
Description: Read and Write Intermedia Representation file
*****************************************************************************/
#include "rwBcFile.h"
LLVMContext MyGlobalContext;

/// \Read Intermedia Representation file
Module* MyParseIRFile(StringRef filename)
{
	SMDiagnostic Err;
	Module* M = NULL;
	std::unique_ptr<Module> mod = nullptr;
	mod = llvm::parseIRFile(filename, Err, MyGlobalContext);
	if (!mod)
	{
		Err.print("Open Module file error", errs());
		return NULL;
	}
	M = mod.get();
	if (!M)
	{
		errs() << ": error loading file '" << filename << "'\n";
		return NULL;
	}
	mod.release();
	return M;
}

/// \Write Intermedia Representation file
BOOL doWriteBackLL(Module* M, StringRef filename)
{
	std::error_code ErrorInfo;
	std::unique_ptr<ToolOutputFile> out(new ToolOutputFile(filename, ErrorInfo, llvm::sys::fs::F_None));

	if (ErrorInfo)
	{
		errs() << ErrorInfo.message() << "\n";
		return false;
	}

	M->print(out->os(), NULL);
	out->keep();
	return true;
}
