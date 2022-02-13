/*****************************************************************************
Filename: OutLine.cpp
Date    :
Description: Function inline confusion.
*****************************************************************************/
#include "OutLine.h"

/// \ Collection of variables  referenced during execution of TargetBasicBlock.
vector<Value*> OutLine::ExtractUseofInst(vector<BasicBlock*> TargetBasicBlock)
{
	vector<Value*> UseValueofTargetBasicBlock;
	UseValueofTargetBasicBlock.clear();
	for (vector<BasicBlock*>::iterator BB = TargetBasicBlock.begin(); BB != TargetBasicBlock.end(); BB++)
	{
		for (Instruction &I : **BB)
		{
			Instruction* tmpI = &I;
			for (Instruction::op_iterator V = tmpI->op_begin(); V != tmpI->op_end(); V++)
			{
				UseValueofTargetBasicBlock.push_back(*V);
			}
		}
	}
	return UseValueofTargetBasicBlock;
}
/// \ Collection of variables defined during execution of TargetBasicBlock.
vector<Value*> OutLine::ExtractDefofInst(vector<BasicBlock*> TargetBasicBlock)
{
	vector<Value*> DefValueofTargetBasicBlock;
	DefValueofTargetBasicBlock.clear();
	for (vector<BasicBlock*>::iterator BB = TargetBasicBlock.begin(); BB != TargetBasicBlock.end(); BB++)
	{
		for (Instruction &I : **BB)
		{
			if ((&I)->getNumUses())
			{
				DefValueofTargetBasicBlock.push_back(&I);
			}
		}
	}
	return DefValueofTargetBasicBlock;
}
/// \ Generate The Parameter of OutLine Function.
BOOL OutLine::GenParameter()
{
	vector<Value*> UseofdoOutLineCode;
	UseofdoOutLineCode = ExtractUseofInst(doOutLineCode);
	OutFuncParameter.clear();
	for (vector<BasicBlock*>::iterator BB = doOutLineCode.begin(); BB != doOutLineCode.end(); BB++)
	{
		for (Instruction &I : **BB)
		{
			doOutLineInst.push_back(&I);
		}
	}
	for (vector<Value*>::iterator V = UseofdoOutLineCode.begin(); V != UseofdoOutLineCode.end(); V++)
	{
		if (isa<Constant>(*V)|| isa<BasicBlock>(*V))
			continue;
		Instruction* inst = (Instruction*)(*V);		
		if (find(doOutLineInst.begin(), doOutLineInst.end(), inst) == doOutLineInst.end())
		{
			if(find(OutFuncParameter.begin(), OutFuncParameter.end(), inst) == OutFuncParameter.end())
				OutFuncParameter.push_back((Value*)inst);
		}
	}
	return true;
}
/// \ Generate The RetValue of OutLine Function.
BOOL OutLine::GenRetValue()
{
	vector<Value*> DefofdoOutLineCode;
	OutFuncRetValue.clear();
	DefofdoOutLineCode = ExtractDefofInst(doOutLineCode);	
	for (vector<Value*>::iterator V = DefofdoOutLineCode.begin(); V != DefofdoOutLineCode.end(); V++)
	{
		for (Value::user_iterator user = (*V)->user_begin(); user != (*V)->user_end(); user++)
		{
			if(find(doOutLineInst.begin(), doOutLineInst.end(), (Instruction*)*V) == doOutLineInst.end())
				OutFuncRetValue.push_back(*V);
		}
	}
	for (vector<Value*>::iterator I = OutFuncParameter.begin(); I != OutFuncParameter.end(); I++)
	{
		for (Value::user_iterator user = (*I)->user_begin(); user != (*I)->user_end(); user++)
		{
			if (find(doOutLineInst.begin(), doOutLineInst.end(), *user) != doOutLineInst.end())
			{
				Instruction* Inst = (Instruction*)*user;
				if (Inst->getOpcode() == 32)
				{
					if (find(OutFuncRetValue.begin(), OutFuncRetValue.end(), *I) == OutFuncRetValue.end())
						OutFuncRetValue.push_back(*I);
				}
			}
			
		}
	}

	return true;
}
/// \ Update ControlMatrix .
BOOL OutLine::UpdateControlMatrix()
{ 
	if (!ExtractKeyBranch.InitialBaiscBLockList(ToObfFunc))
	{
		errs() << "Failed to Upate BasicBlock List\n";
		return false;
	}
	if (!ExtractKeyBranch.GenControlMatrix()) {
		errs() << "Failed to Upate Function Control Flow Graph\n";
		return false;
	}
	return true;
}
/// \ Create OutLineFunc.
BOOL OutLine::CreateOutLineFunc()
{	
	vector<Type*> elements, FuncArgs;
	vector<Instruction*> DeletInst;
	//ReturnStruct
	RetStructType = StructType::create(ToObfFunc->getContext(), "Struct.RetValue");
	for (vector<Value*>::iterator value = OutFuncRetValue.begin(); value != OutFuncRetValue.end(); ++value) {
		elements.push_back((*value)->getType());
	}
	RetStructType->setBody(elements,true);
	RetStruct = new GlobalVariable(*ToObfFunc->getParent(), RetStructType, false, 
	GlobalValue::InternalLinkage,Constant::getNullValue(RetStructType), "Struct.RetValue");	
	//Parameter
	for (vector<Value*>::iterator V = OutFuncParameter.begin(); V != OutFuncParameter.end(); V++)
	{
		FuncArgs.push_back((*V)->getType());
	}
	FunctionType* FuncTy = FunctionType::get(Type::getVoidTy(ToObfFunc->getContext()),   //The return value type of the function
						   FuncArgs,                                                     //Parameter type
						   false);                                                       //Whether to include a variable number of arguments
	//Create OutLineFunc
	OutLineFunc = Function::Create(FuncTy, GlobalValue::LinkageTypes::ExternalLinkage, 
	doOutLineBranch.first->getName().str()+".OutLineFunc", ToObfFunc->getParent());  
	OutLineFunc->setCallingConv(CallingConv::C); 
	for (Function::arg_iterator args = OutLineFunc->arg_begin(); args != OutLineFunc->arg_end(); ++args) {
		Argument* arg = &*args;
		arg->setName("OutFuncArg");
	}
	return true;
}
/// \ Clone OutLineCode to OutLineFunc and Map logic of BasicBlock.
BOOL OutLine::CloneOutLineFuncCode()
{
	vector<BranchInst*> Br;
	ValueToValueMapTy BasicBlockMap;
	vector<PHINode*> PhiNode;
	vector<BasicBlock*> VecCloneBB;
	int Index = 0;
	BasicBlock* EntryBB = BasicBlock::Create(OutLineFunc->getContext(), "entry", OutLineFunc);
	for (Function::arg_iterator argiter = OutLineFunc->arg_begin(); argiter != OutLineFunc->arg_end(); argiter++, Index++) {
		AllocaInst* IndexInst = new AllocaInst(argiter->getType(), 0, 0, 4, argiter->getName() + ".addr", EntryBB);
		StoreInst* ArgSotre = new StoreInst(argiter, IndexInst, 0, 4, EntryBB);
		LoadInst* ArgLoad = new LoadInst(IndexInst, OutFuncParameter[Index]->getName().str()+".Clone", 0, 4, EntryBB);
		ParameterMap[OutFuncParameter[Index]] = ArgLoad;
	}
	for (vector<BasicBlock*>::iterator BB = doOutLineCode.begin(); BB != doOutLineCode.end(); BB++)
	{
		BasicBlock *cloneBB = CloneBasicBlock(*BB, ParameterMap,".Clone", OutLineFunc);
		BasicBlockMap[*BB] = cloneBB;
		VecCloneBB.push_back(cloneBB);
		for (Instruction &I : *cloneBB)
		{
			if (dyn_cast<BranchInst>(&I)) {
				Br.push_back((BranchInst*)&I);
			}
			if (PHINode *pn = dyn_cast<PHINode>(&I)) {
				PhiNode.push_back(pn);
			}
		}
	}
	for (vector<BasicBlock*>::iterator BB = VecCloneBB.begin(); BB != VecCloneBB.end(); BB++)
	{
		for (Instruction &I : **BB)
		{
			for (int i = 0; i < I.getNumOperands(); i++)
			{
				Value* V = MapValue(I.getOperand(i), ParameterMap);
				if (V)
				{
					I.setOperand(i, V);
				}
			}
		}

	}
	for (int i = 0; i < Br.size(); i++)
	{
		for (int j = 0; j < Br[i]->getNumSuccessors(); j++)
		{
			Value* V = MapValue(Br[i]->getSuccessor(j), BasicBlockMap);
			if (V)
			{
				Br[i]->setSuccessor(j, (BasicBlock*)V);
			}
			else
			{
				Br[i]->setSuccessor(j, (BasicBlock*)MapValue(doOutLineBranch.second, BasicBlockMap));
			}
		}
	}
	BasicBlock* ReturnBB = BasicBlock::Create(OutLineFunc->getContext(), "OutFuncRetBB", OutLineFunc);
	BranchInst::Create(ReturnBB, Br[Br.size() - 1]);
	Br[Br.size() - 1]->eraseFromParent();
	Value* const_0 = ConstantInt::get(IntegerType::getInt32Ty(OutLineFunc->getContext()), 0);
	Value* const_1 = ConstantInt::get(IntegerType::getInt32Ty(OutLineFunc->getContext()), 0);
	SmallVector<Value*, 2> indexVector;
	indexVector.push_back(const_0);
	Index = 0;
	for (vector<Value*>::iterator value = OutFuncRetValue.begin(); value != OutFuncRetValue.end(); value++, Index++) {
		const_1 = ConstantInt::get(IntegerType::getInt32Ty(OutLineFunc->getContext()), Index);
		indexVector.push_back(const_1);
		Value* RetValue = *value;
		Value* V = MapValue(*value, ParameterMap);
		if (V)
		{
			RetValue = V;		
			GetElementPtrInst* GetInst = GetElementPtrInst::CreateInBounds(RetStructType, RetStruct, indexVector, "", ReturnBB);
			new StoreInst(RetValue, GetInst, 0, 4, ReturnBB);
			indexVector.pop_back();
		}
	}
	ReturnInst::Create(OutLineFunc->getContext(), nullptr, ReturnBB);
	BranchInst::Create(EntryBB->getNextNode(), EntryBB);
	for (vector<PHINode*>::iterator P = PhiNode.begin(); P != PhiNode.end(); ++P) {
		for (unsigned j = 0, e = (*P)->getNumIncomingValues(); j != e; ++j) {
			(*P)->setIncomingBlock(j, (BasicBlock*)MapValue((*P)->getIncomingBlock(j),BasicBlockMap));
		}
	}
	if (!OutLineFunc->size())
	{
		return false;
	}
	return true;
}
/// \ Call OutLineFunc,Creating a CallInst.
BOOL OutLine::CallOutLineFunc()
{
	int i = 0;
	ValueToValueMapTy CondPrevMap;
	Value* const_0 = ConstantInt::get(IntegerType::getInt32Ty(ToObfFunc->getContext()), 0);
	Value* const_2 = ConstantInt::get(IntegerType::getInt32Ty(ToObfFunc->getContext()), 5);
	SmallVector<Value*, 2> indexVector;
	indexVector.push_back(const_0);
	BasicBlock* CallOutFuncBB = BasicBlock::Create(ToObfFunc->getContext(), "CallOutFuncBB",ToObfFunc, doOutLineBranch.first);
	OutFuncCallInst = CallInst::Create(OutLineFunc, OutFuncParameter, "", CallOutFuncBB);
	doOutLineBranch.first->replaceAllUsesWith(CallOutFuncBB);
	i = 0;
	for (vector<Value*>::iterator V = OutFuncRetValue.begin(); V != OutFuncRetValue.end(); V++,i++)
	{
		const_2 = ConstantInt::get(IntegerType::getInt32Ty(CallOutFuncBB->getContext()), i);
		indexVector.push_back(const_2);
		Value* ParRetValue = MapValue(*V, ParameterMap);
		if (ParRetValue) 
		{
			GetElementPtrInst* GetInst = GetElementPtrInst::CreateInBounds(RetStructType, RetStruct, indexVector, "", CallOutFuncBB);
			LoadInst* Var2 = new LoadInst(GetInst, "", false, 4, CallOutFuncBB);
			Var2 = new LoadInst(Var2, "", false, 4, CallOutFuncBB);
			StoreInst* newsI = new StoreInst(Var2, *V, CallOutFuncBB);
			indexVector.pop_back();
			continue;
		}
	}
	BranchInst* Br = BranchInst::Create(doOutLineBranch.second->getUniqueSuccessor(), CallOutFuncBB);
	if(OutFuncCallInst->isUsedInBasicBlock(CallOutFuncBB))
	{
		return false;
	}
	return true;
}
/// \ If EndBasicBlock is the Conditional Basicblock, Split EndBasicBlock to newBB  and Condition.
BOOL OutLine::TransformOutLineCode()
{
	BranchInst* Br = dyn_cast<BranchInst>(doOutLineBranch.second->getTerminator());
	if (!Br->isConditional())
		return true;
	BasicBlock* BrBB = doOutLineBranch.second;
	BasicBlock* NewBB = BrBB->splitBasicBlock(BrBB->begin(), "new");
	BrBB->replaceAllUsesWith(NewBB);
	for (vector<vector<BasicBlock*>>::iterator ite = ExtractKeyBranch.OutLineCode.begin(); ite != ExtractKeyBranch.OutLineCode.end(); ite++)
	{
		for (vector<BasicBlock*>::iterator BB = (*ite).begin(); BB != (*ite).end(); ++BB) {
			if (*BB == BrBB)
				*BB = NewBB;
		}
	}
	for (int i = 0; i < ExtractKeyBranch.OutKeyBranch.size(); i++)
	{
		ExtractKeyBranch.OutKeyBranch[i];
		BasicBlock* ConditionBB = ExtractKeyBranch.OutKeyBranch[i].first;
		BasicBlock* EndBB = ExtractKeyBranch.OutKeyBranch[i].second;
		if (BrBB == ConditionBB)
			ConditionBB = NewBB;
		if (BrBB == EndBB)
			EndBB = NewBB;
	}
	UpdateControlMatrix();
	return true;
}
/// \ Delete Origin OutLineCode.
BOOL OutLine::DeleteOutLineCode() {
	vector<Instruction*> DeletInst;
	queue<Instruction*> RemoveInst;
	int size = doOutLineCode.size();
	for (int i = 0; i < size; i++)
	{
		for (Instruction &I : *doOutLineCode[i])
		{
			RemoveInst.push(&I);
		}
	}
	while (!RemoveInst.empty())
	{
		Instruction* I = RemoveInst.front();
		if (!I->getNumUses()) {   
			RemoveInst.pop();
			I->eraseFromParent();
		}
		else
		{		
			RemoveInst.pop();
			RemoveInst.push(I);
		}
	}
	for (int i = 0; i < size; i++)
	{
		BasicBlock* DeletBB = doOutLineCode.back();
		doOutLineCode.pop_back();
		DeletBB->eraseFromParent();
	}
	if (!doOutLineCode.empty())
		return false;
	return true;
}
/// \ Generate OutLine Function by doOutLineCode.
BOOL OutLine::GenOutLineFunc()
{
	TransformOutLineCode();
	if (!GenParameter())
	{
		errs() << "Failed to Get the Parameter of OutLine Function\n";
		return false;
	}
	if (!GenRetValue())
	{
		errs() << "Failed to Get the RetValue of OutLine Function\n";
		return false;
	}
	if (!CreateOutLineFunc())
	{
		errs() << "Failed to Create OutLine Function\n";
		return false;
	}
	if (!CloneOutLineFuncCode())
	{
		errs() << "Failed to Generate OutLine Function Code\n";
		return false;
	}
	if (!CallOutLineFunc())
	{
		errs() << "Failed to Create OutLine Function CallInst\n";
		return false;
	}
	if (!DeleteOutLineCode())
	{
		errs() << "Failed to Delete OutLineCode\n";
		return false;
	}
	errs() << "Function [" << ToObfFunc->getName().str() << "]  has completed Outline obfuscation\n";
	return true;
}
/// \ OutLine Obfuscation Entry Point, Randomly select the key branch outside the function.
BOOL OutLine::OutLineObf() {
	int OutLineBranchIndex = 0;
	srand(time(NULL));
	for (int i = 0; i < OutLineFuncList.size(); i++)
	{
		ToObfFunc = OutLineFuncList[i];
		ExtractKeyBranch.OutLineCode.clear();
		ExtractKeyBranch.OutKeyBranch.clear();
		doOutLineCode.clear();
		doOutLineInst.clear();
		OutFuncParameter.clear();
		OutFuncRetValue.clear();
		ParameterMap.clear();
		ExtractKeyBranch.GetKeyBranchCode(ToObfFunc);
		OutLineBranchIndex = rand() % ExtractKeyBranch.OutLineCode.size();
		doOutLineCode.assign(ExtractKeyBranch.OutLineCode[OutLineBranchIndex].begin(), ExtractKeyBranch.OutLineCode[OutLineBranchIndex].end());
		doOutLineBranch = ExtractKeyBranch.OutKeyBranch[OutLineBranchIndex];
		GenOutLineFunc();
	}
	return true;
}

