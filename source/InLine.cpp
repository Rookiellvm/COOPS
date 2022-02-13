/*****************************************************************************
Filename: InLine.cpp
Date    :
Description: Function inline confusion.
*****************************************************************************/
#include "InLine.h"

/// \Gets the user of the inline function
BOOL InLine::GetInLineFuncUser(Function &F)
{
	InLineFunCL.clear();
	InLineFun = &F;
	for (User *U : InLineFun->users()) {
		if (!isa<CallInst>(U))
		{
			NoDeleFunc = true;
		}
		if (doInLineFunCL = dyn_cast<CallInst>(U))
		{
			BasicBlockMap.clear();
			CloneBBVec.clear();
			BasicBlock* NewBB = doInLineFunCL->getParent()->splitBasicBlock(doInLineFunCL, doInLineFunCL->getParent()->getName().str() + ".Split");
			InLineFunCL.push_back(doInLineFunCL);
			CloneInLineCode();
			NewBB->getPrevNode()->getTerminator()->eraseFromParent();  
			BranchInst* BR = BranchInst::Create((BasicBlock*)MapValue(&*InLineFun->begin(), BasicBlockMap), NewBB->getPrevNode());
			BR = BranchInst::Create(NewBB, (BasicBlock*)MapValue(InLineFunRetBB, BasicBlockMap));
			for (int i = 0; i < CloneBBVec.size(); i++)
			{
				CloneBBVec[i]->moveBefore(NewBB);
			}						
		}
	}

	// Erase CallInst
	int Size = InLineFunCL.size();
	for (int i = 0; i < Size; i++)
	{
		CallInst* CL = InLineFunCL.back();
		InLineFunCL.pop_back();
		CL->eraseFromParent();
	}	
	return true;
}
/// \Clone the function code to the call point
BOOL InLine::CloneInLineCode()
{
	ValueToValueMapTy ValueMap;
	ValueToValueMapTy ArgValueMap;
	vector<BranchInst*> Br;
	vector<PHINode*> PhiNode;
	Function* CallInLineFunc = doInLineFunCL->getFunction();
	Value* RetValue;
	ReturnInst* RetInst;
	ValueMap.clear();
	PhiNode.clear();
	Br.clear();
	ArgValueMap.clear();
	int i = 0;
	for (Use &U : doInLineFunCL->operands())
	{
		Value *V = U.get();
		if (!isa<Function>(V))
		{
			ValueMap[InLineFun->arg_begin() + i] = V;
			i++;
		}		
	}
	for (BasicBlock &BB : *InLineFun)
	{		
		if (isa<ReturnInst>((&BB)->getTerminator()))
		{
			InLineFunRetBB = &BB;
		}
		BasicBlock *CloneBB = CloneBasicBlock(&BB, ValueMap, "." + InLineFun->getName().str() + ".Clone", CallInLineFunc);
		CloneBBVec.push_back(CloneBB);
		BasicBlockMap[&BB] = CloneBB;
		for (Instruction &I : *CloneBB)
		{
			if (dyn_cast<BranchInst>(&I)) {
				Br.push_back((BranchInst*)&I);
			}
			if (PHINode *pn = dyn_cast<PHINode>(&I)) {
				PhiNode.push_back(pn);
			}
			if (isa<ReturnInst>(&I))
			{		
				RetInst = (ReturnInst*)&I;
				if (InLineFun->getReturnType()->getTypeID() != 0)
				{
					RetValue = MapValue((&I)->getOperand(0), ValueMap);
					AllocaInst* Ret = new AllocaInst(RetValue->getType(), 0, 0, 4, "RetValuePtr", (&*CallInLineFunc->begin())->getTerminator());
					StoreInst* SI = new StoreInst(RetValue, Ret, RetInst);
					LoadInst* LI = new LoadInst(Ret, "RetValue", doInLineFunCL);
					doInLineFunCL->replaceAllUsesWith(LI);
				}			
			}
		}
	}

	//Repair Instruction
	for (vector<BasicBlock*>::iterator BB = CloneBBVec.begin(); BB != CloneBBVec.end(); BB++)
	{
		for (Instruction &I : **BB)
		{
			for (int i = 0; i < I.getNumOperands(); i++)
			{
				Value* V = MapValue(I.getOperand(i), ValueMap);
				if (V)
				{
					I.setOperand(i, V);
				}
			}
		}
	}
	//Mapping BasicBlock Logic
	for (int i = 0; i < Br.size(); i++)
	{
		for (int j = 0; j < Br[i]->getNumSuccessors(); j++)
		{
			Value* V = MapValue(Br[i]->getSuccessor(j), BasicBlockMap);
			if (V)
			{
				Br[i]->setSuccessor(j, (BasicBlock*)V);
			}
		}
	}
	//Mapping Phi Instruction
	for (vector<PHINode*>::iterator P = PhiNode.begin(); P != PhiNode.end(); ++P) {
		for (unsigned j = 0, e = (*P)->getNumIncomingValues(); j != e; ++j) {
			(*P)->setIncomingBlock(j, (BasicBlock*)MapValue((*P)->getIncomingBlock(j), BasicBlockMap));
		}
	}
	RetInst->eraseFromParent();
	return true;
}
/// \Inline obfuscation entry point
BOOL InLine::InLineObf() {
	int Size = InLineFuncList.size();
	for (int i = 0; i < Size; i++)
	{
		GetInLineFuncUser(*InLineFuncList[i]);
		errs() << "Function [" << InLineFuncList[i]->getName().str() << "]  has completed inline obfuscation\n";
		if (!NoDeleFunc) {
			DeletFunc.push_back(InLineFuncList[i]);
		}	
		NoDeleFunc = false;
	}
	//Erase InLineFunc Prototype
	Size = DeletFunc.size();
	for (int i = 0; i < Size; i++)
	{
		Function* tmpFun = DeletFunc.back();
		DeletFunc.pop_back();
		tmpFun->eraseFromParent();
	}
	return true;
}