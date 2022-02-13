#include "Common.h"

class InLine
{
public:
	BOOL InLineObf();
	vector<Function*> InLineFuncList;
protected:
	BOOL CloneInLineCode();
	BOOL GetInLineFuncUser(Function &F);

	BOOL NoDeleFunc = false;
	Function *InLineFun;
	vector<CallInst*> InLineFunCL;
	vector<BasicBlock*> CloneBBVec;
	vector<Function*> DeletFunc;
	CallInst* doInLineFunCL;
	ValueToValueMapTy BasicBlockMap;
	BasicBlock* InLineFunRetBB;
};


