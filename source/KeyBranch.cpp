/*****************************************************************************
Filename: KeyBranch.cpp
Date    :
Description: Determine whether there is a key branch in the function, 
if there is a key branch, extract the Outline code of the key branch.
*****************************************************************************/
#include "KeyBranch.h"
/// \Initialize the Basicblock List.
BOOL KeyBranch::InitialBaiscBLockList(Function* Fun) 
{
	BasicBlockList.clear();
	for (BasicBlock &B : *Fun) {
		BasicBlockList.push_back(&B);
	}
	return true;
}
/// \Generate Function Control Flow Graph.
BOOL KeyBranch::GenControlMatrix() 
{
	ControlMatrix = MallocGraph(BasicBlockList.size());
	if (ControlMatrix == NULL) 
	{
		errs() << "Memory allocation failed\n";
		return false;
	}
	for (int i = 0; i < BasicBlockList.size(); i++)
	{
		for (Value::user_iterator useri = BasicBlockList[i]->user_begin(), usere = BasicBlockList[i]->user_end(); useri != usere; useri++) 
		{
			Value* tmpuser = dyn_cast<Value>(*useri);
			if (Instruction* inst = dyn_cast<Instruction>(tmpuser))
			{
				if (FindBasicBlockIndex(inst->getParent()) == -1) {
					errs() << "Fail to generate Control flow graph\n";
					return false;
				}
				ControlMatrix[FindBasicBlockIndex(inst->getParent())][i] = 1;
			}
			
		}
	}

	return true;
}
/// \Get the Index Number in the Basicblock List.
int KeyBranch::FindBasicBlockIndex(BasicBlock* TargetBB) {
	int Index = -1;
	for (int i = 0; i < BasicBlockList.size(); i++)
	{
		if (TargetBB == BasicBlockList[i])
		{
			Index = i;
			break;
		}
	}
	return Index;
}
/// \Generate Reachability Matrix.
BOOL KeyBranch::GenAccessMatrix()
{
	int MatrixLength = BasicBlockList.size();

	int** PowerMatrix = CopyGraph(MatrixLength, ControlMatrix);
	AccessMatrix = CopyGraph(MatrixLength, ControlMatrix);

	for (int i = 2; i <= MatrixLength; i++)
	{
		PowerMatrix = MatrixMulti(PowerMatrix, ControlMatrix, MatrixLength);
		AccessMatrix = MatrixPlus(PowerMatrix, AccessMatrix, MatrixLength);
	}
	for (int i = 0; i < BasicBlockList.size(); i++)
	{
		for (int j = 0; j < BasicBlockList.size(); j++)
		{
			if (AccessMatrix[i][j] != 0)
			{
				AccessMatrix[i][j] = 1;
			}
		}
	}

	return true;
}
/// \Copy the value of matrix A to matrix B and return the pointer of matrix B.
int** KeyBranch::CopyGraph(int size, int** sourceGraph)
{
	int** Graph = (int **)malloc(sizeof(int) * size);
	for (int j = 0; j < size; j++)
	{
		Graph[j] = (int *)malloc(sizeof(int) * size);
	}
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			Graph[i][j] = sourceGraph[i][j];
		}
	}
	return Graph;
}
/// \Matrix Multiplication.
int** KeyBranch::MatrixMulti(int** tempMatrixA, int** tempMatrixB, int MatrixLength)
{
	int** pathMatrix = MallocGraph(MatrixLength);
	for (int i = 0; i < MatrixLength; i++)
	{
		for (int k = 0; k < MatrixLength; k++)
		{
			int r = tempMatrixA[i][k];
			for (int j = 0; j < MatrixLength; j++)
			{
				pathMatrix[i][j] += r * tempMatrixB[k][j];
				if (pathMatrix[i][j] != 0)
				{
					pathMatrix[i][j] = 1;
				}
			}
		}
	}
	return pathMatrix;
}
/// \Matrix Addition.
int** KeyBranch::MatrixPlus(int** MatrixA, int** MatrixB, int MatrixLength)
{
	int** tempAccessMatrix = MallocGraph(MatrixLength);
	for (int i = 0; i < MatrixLength; i++)
	{
		for (int j = 0; j < MatrixLength; j++)
		{
			tempAccessMatrix[i][j] = MatrixA[i][j] + MatrixB[i][j];
		}
	}
	return tempAccessMatrix;
}
/// \Get Candidate Conditional Branches.
BOOL KeyBranch::GetCandidateBranch(Function* Fun)
{
	CandidateKeyBranch.clear();
	ConditionBBSet.clear();
	for (BasicBlock &B : *Fun) 
	{
		BranchInst *br = NULL;
		BasicBlock* tmpB = &B;
		if (tmpB == &*Fun->begin())
			continue;
		if (isa<BranchInst>(tmpB->getTerminator())) 
		{
			br = cast<BranchInst>(tmpB->getTerminator());
			if ((br != NULL && br->isConditional()) || tmpB->getTerminator()->getNumSuccessors() == 2){
				BasicBlock* LBB = tmpB->getTerminator()->getSuccessor(0);
				BasicBlock* RBB = tmpB->getTerminator()->getSuccessor(1);
				ConditionBBSet.push_back(tmpB);
				if(IdEndBasicBlock(LBB, RBB))   
				{		
					CandidateKeyBranch.push_back(make_pair(tmpB, IdEndBasicBlock(LBB, RBB)));
				}		
			}
		}
	}
	if(CandidateKeyBranch.empty())
	{
		return false;
	}

	return true;
}
/// \Get the End Basicblock of the Conditional Branch.
BasicBlock* KeyBranch::IdEndBasicBlock(BasicBlock* LBB, BasicBlock* RBB)
{
	BasicBlock* EndBasicBlock;
	int LIndex, RIndex;
	for (int i = 0; i < BasicBlockList.size(); i++)
	{
		if (LBB == BasicBlockList[i])
		{
			LIndex = i;
			continue;
		}
		if (RBB == BasicBlockList[i])
		{
			RIndex = i;
			continue;
		}
	}
	if (AccessMatrix[LIndex][RIndex])
	{
		EndBasicBlock = BasicBlockList[RIndex];
		return EndBasicBlock;
	}

	if (AccessMatrix[RIndex][LIndex])
	{
		EndBasicBlock = BasicBlockList[LIndex];
		return EndBasicBlock;
	}

	int Flag = 0;
	for (int j = LIndex; j < BasicBlockList.size(); j++)
	{
		if (AccessMatrix[LIndex][j] == 1 && AccessMatrix[RIndex][j] == 1)
		{
			Flag = 1;
			EndBasicBlock = BasicBlockList[j];
			break;
		}

	}	
	if (!Flag) {
		return NULL;
	}

	return EndBasicBlock;
}

BOOL KeyBranch::IsConditionBB(BasicBlock* TargetBB)
{
	for (vector<pair<BasicBlock*, BasicBlock*>>::iterator pairBB = CandidateKeyBranch.begin(); pairBB != CandidateKeyBranch.end(); pairBB++)
	{
		if (TargetBB == (*pairBB).first)
			return true;
	}
	return false;
}
/// \Adhere to the Data Flow of Outline Code.
BOOL KeyBranch::CheckOutLineCode(BasicBlock *StartBB, BasicBlock *EndBB)
{
	for (Instruction &I : *StartBB)
	{
		if (isa<PHINode>(I))
			return false;
	}
	for (Instruction &I : *EndBB)
	{
		if(isa<ReturnInst>(I)|| isa<PHINode>(I))
			return false;			
	}
	CodeOnAccessPath.push_back(EndBB);
	BasicBlock* AdjacentBB;
	for (vector<BasicBlock*>::iterator BB = CodeOnAccessPath.begin(), BBe = CodeOnAccessPath.end(); BB != BBe; BB++)
	{
		if (*BB == EndBB)
		{
			continue;
		}
		for (int i = 0; i < BasicBlockList.size(); i++)
		{
			if (ControlMatrix[FindBasicBlockIndex(*BB)][i])
			{
				AdjacentBB = BasicBlockList[i];
				if (find(CodeOnAccessPath.begin(), CodeOnAccessPath.end(), AdjacentBB) == CodeOnAccessPath.end())
				{
					return false;
				}
			}
		}
	}
	for (vector<BasicBlock*>::iterator BB = CodeOnAccessPath.begin(), BBe = CodeOnAccessPath.end(); BB != BBe; BB++)
	{
		if (*BB == StartBB)
		{
			continue;
		}
		for (int i = 0; i < BasicBlockList.size(); i++)
		{
			if (ControlMatrix[i][FindBasicBlockIndex(*BB)])
			{
				AdjacentBB = BasicBlockList[i];
				if (find(CodeOnAccessPath.begin(), CodeOnAccessPath.end(), AdjacentBB) == CodeOnAccessPath.end())
				{
					if (*BB == EndBB)
					{
						if (i > FindBasicBlockIndex(EndBB))
							continue;
					}
					return false;
				}
			}
		}
	}

	return true;
}
/// \Extract OutLineCode.
BOOL KeyBranch::ExtractOutLineCode()
{
	OutLineCode.clear();
	OutKeyBranch.clear();
	BasicBlock *ConditionBasicBlock, *EndBasicBlock;
	for (vector<pair<BasicBlock*, BasicBlock*>>::iterator pairBB = CandidateKeyBranch.begin(), pairBBe = CandidateKeyBranch.end(); pairBB != pairBBe; pairBB++)
	{
		ConditionBasicBlock = (*pairBB).first;
		EndBasicBlock = (*pairBB).second;
		Stack.clear();
		Vist[200] = { 0 };
		CodeOnAccessPath.clear();
		RecDepth = 0;
		DFS(ConditionBasicBlock, EndBasicBlock);
		if (CheckOutLineCode(ConditionBasicBlock, EndBasicBlock))
		{
			OutLineCode.push_back(CodeOnAccessPath);
			OutKeyBranch.push_back(*pairBB);
		}
	}
	if (OutLineCode.empty() || OutKeyBranch.empty())
		return false;
	return true;
}
/// \Check if there are Key Branches.
BOOL KeyBranch::CheckKeyBranch()
{
	BasicBlock *ConditionBasicBlock, *EndBasicBlock;
	for (vector<pair<BasicBlock*, BasicBlock*>>::iterator pairBB = CandidateKeyBranch.begin(), pairBBe = CandidateKeyBranch.end(); pairBB != pairBBe; pairBB++)
	{
		ConditionBasicBlock = (*pairBB).first;
		EndBasicBlock = (*pairBB).second;
		Stack.clear();
		Vist[200] = { 0 };
		CodeOnAccessPath.clear();
		RecDepth = 0;
		DFS(ConditionBasicBlock, EndBasicBlock);
		if (CheckOutLineCode(ConditionBasicBlock, EndBasicBlock))
		{
			return true;
		}
	}
	return false;
}
/// \Use DFS algorithm to Obtain all Reachable Paths between Two BasicBlocks.
void KeyBranch::DFS(BasicBlock *StartBB, BasicBlock *EndBB)
{
	RecDepth++;
	if (RecDepth > 200)
		return;
	int Start = FindBasicBlockIndex(StartBB);
	if (StartBB == EndBB)
	{
		for (vector<int>::iterator I = Stack.begin(); I != Stack.end(); I++)
		{
			if (find(CodeOnAccessPath.begin(), CodeOnAccessPath.end(), BasicBlockList[*I]) == CodeOnAccessPath.end())
				CodeOnAccessPath.push_back(BasicBlockList[*I]);
		}
		return;
	}
	Vist[Start] = 1;
	Stack.push_back(Start);
	if (find(ConditionBBSet.begin(), ConditionBBSet.end(), StartBB) != ConditionBBSet.end())
	{
		Vist[Start] = 0;
	}
	for (int i = 0; i < BasicBlockList.size(); i++)
	{
		if (!Vist[i] && ControlMatrix[Start][i])
		{
			DFS(BasicBlockList[i], EndBB);
		}
	}
	Vist[Start] = 0;
	Stack.pop_back();
}
/// \Get Key Branches and Their OutLine code.
BOOL KeyBranch::GetKeyBranchCode(Function* Func) 
{
	if (!ishasKeyBranch(Func))
	{
		errs() << " Failed to Extract KeyBranch\n";
		return false;
	}
	if (!ExtractOutLineCode())
	{
		errs() << " Failed to Extract OutLineCode\n";
		return false;
	}
	return true;
}
/// \Check if there are Key Branches.
BOOL KeyBranch::ishasKeyBranch(Function* Func)
{
	if (!InitialBaiscBLockList(Func))
	{
		errs() << "Failed to Initial BasicBlock List\n";
		return false;
	}
	if (!GenControlMatrix()) {
		errs() << "Failed to Generate Function Control Flow Graph\n";
		return false;
	}
	if (!GenAccessMatrix()) {
		errs() << "Failed to Generate BasicBlock Reachable matrix\n";
		return false;
	}
	if (!GetCandidateBranch(Func))
	{
		errs() << " Failed to Extract CandidateBranch\n";
		return false;
	}
	if (!CheckKeyBranch())
	{
		errs() << " Function:[" << Func->getName().str() << "]has no Key Branch\n";	
		return false;
	}
	return true;
}
