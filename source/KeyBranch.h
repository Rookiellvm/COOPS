#pragma once
#include "Common.h"

class KeyBranch
{
public:
	BOOL GetKeyBranchCode(Function* Func);
	BOOL ishasKeyBranch(Function* Func);
	vector<vector<BasicBlock*>> OutLineCode;
	vector<pair< BasicBlock*, BasicBlock*>> OutKeyBranch;
	vector<BasicBlock*> BasicBlockList;
	vector<BasicBlock*> CodeOnAccessPath;
	BOOL IsConditionBB(BasicBlock* TargetBB);
	int** ControlMatrix;
	int** AccessMatrix;
	vector<int> Stack;
	int Vist[200] = { 0 };
	int RecDepth;
	int FindBasicBlockIndex(BasicBlock* TargetBB);
	void DFS(BasicBlock *StartBB, BasicBlock *EndBB);
	BOOL CheckOutLineCode(BasicBlock *StartBB, BasicBlock *EndBB);
	BOOL InitialBaiscBLockList(Function* Fun);
	BOOL GenControlMatrix();
protected:
	BOOL GenAccessMatrix();
	BOOL GetCandidateBranch(Function* Fun);
	BOOL ExtractOutLineCode();
	BOOL CheckKeyBranch();
	int** CopyGraph(int size, int** sourceGraph);
	int** MatrixMulti(int** tempMatrixA, int** tempMatrixB, int MatrixLength);
	int** MatrixPlus(int** MatrixA, int** MatrixB, int MatrixLength);
	BasicBlock* IdEndBasicBlock(BasicBlock* LBB, BasicBlock* RBB);

	vector<pair< BasicBlock*, BasicBlock*>> CandidateKeyBranch;
	vector<BasicBlock*> ConditionBBSet;
};