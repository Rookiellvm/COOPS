#include "Common.h"
#include "KeyBranch.h"
#include <random>
#include "include/llvm/Transforms/Utils.h"
class CallGraphRandom
{
public:
	BOOL GenCallGraph(Module* M);
	int ERProb = 100;   
	vector<Function*> doInLineList, doOutLineList;
protected:
	BOOL InitialFuncList(Module* M);
	BOOL GenCallMatrix();
	BOOL ERGraph();
	BOOL GetInLineFuncList();
	BOOL GetOutLineFuncList();
	int FindFuncIndex(Function* Fun);
	int **CallMatrix;
	KeyBranch doKeyBranch;
	vector<Function*> FuncList;
	vector<Function*> InLineFuncList, OutLineFuncList;
	FunctionPass *lower = createLowerSwitchPass();

protected:
};