#pragma once
#include "Common.h"

Module* MyParseIRFile(StringRef filename);
BOOL doWriteBackLL(Module* M, StringRef filename);
