#include "Common.h"
#include "KeyBranch.h"
#include <queue>
#include "llvm/IR/CFG.h"

class OutLine 
{
public:
	BOOL OutLineObf();
	vector<Function*> OutLineFuncList;
protected:
	BOOL GenParameter();
	BOOL GenRetValue();
	BOOL GenOutLineFunc();
	BOOL UpdateControlMatrix();
	BOOL CreateOutLineFunc();
	BOOL CloneOutLineFuncCode();
	BOOL CallOutLineFunc();
	BOOL TransformOutLineCode();
	vector<Value*> ExtractUseofInst(vector<BasicBlock*> TargetBasicBlock);
	vector<Value*> ExtractDefofInst(vector<BasicBlock*> TargetBasicBlock);
	BOOL DeleteOutLineCode();

	KeyBranch ExtractKeyBranch;
	vector<BasicBlock*> doOutLineCode;
	pair<BasicBlock*, BasicBlock*> doOutLineBranch;
	vector<Instruction*> doOutLineInst;
	vector<Value*> OutFuncParameter, OutFuncRetValue;
	Function *ToObfFunc, *OutLineFunc;
	StructType* RetStructType;
	ValueToValueMapTy ParameterMap;
	CallInst *OutFuncCallInst;
	GlobalVariable* RetStruct;
};

