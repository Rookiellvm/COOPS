/*****************************************************************************
Filename: OCalling.cpp
Date    :
Description: Use ER algorithm to generate function call graph randomly.
*****************************************************************************/
#include "CallGraphRandom.h"

/// \Gets the function list index number
int CallGraphRandom::FindFuncIndex(Function* Fun)
{
	int Index = -1;
	for (int i = 0; i < FuncList.size(); i++)
	{
		if (Fun == FuncList[i])
		{
			Index = i;
			break;
		}
	}
	return Index;
}
/// \Initialize the list of functions
BOOL CallGraphRandom::InitialFuncList(Module* M)
{
	for (Function &F : *M)
	{
		if (F.isDeclaration() || F.getName().str() == "main"|| F.isVarArg())
			continue;
		lower->runOnFunction(F);
		FuncList.push_back(&F);
	}
	errs() << "This Program has [" << FuncList.size() << "] Functions!\n";
	return true;
}
/// \Generate Original Function Call Graph
BOOL CallGraphRandom::GenCallMatrix()
{
	CallMatrix = MallocGraph(FuncList.size());
	if (CallMatrix == NULL)
	{
		errs() << "Memory allocation failed\n";
		return false;
	}
	for (int i = 0; i < FuncList.size(); i++)
	{
		for (Value::user_iterator useri = FuncList[i]->user_begin(), usere = FuncList[i]->user_end(); useri != usere; useri++)
		{
			Value* tmpuser = dyn_cast<Value>(*useri);
			if (Instruction* inst = dyn_cast<Instruction>(tmpuser))
			{
				if (FindFuncIndex(inst->getParent()->getParent()) != -1)
				{
					CallMatrix[FindFuncIndex(inst->getParent()->getParent())][i] = 1;
				}
			}
		}
	}
	return true;
}
/// \Random Generate Function Call Graph
BOOL CallGraphRandom::ERGraph()
{
	srand(time(NULL));
	int RandKey; 
	Function* NewFun;
	for (vector<Function*>::iterator iter = InLineFuncList.begin(); iter != InLineFuncList.end(); iter++)
	{
		RandKey = rand() % 100;
		if (RandKey < ERProb)
		{
			if (FindFuncIndex(*iter) != -1)
			{
				doInLineList.push_back(*iter);
			}		
			for (int i = 0; i < FuncList.size(); i++)
			{
				CallMatrix[i][FindFuncIndex(*iter)] = 0;
			}
		}
	}
	for (vector<Function*>::iterator iter = OutLineFuncList.begin(); iter != OutLineFuncList.end(); iter++)
	{
		RandKey = rand() % 100;
		if (RandKey < ERProb)
		{
			doOutLineList.push_back(*iter);
			//OutLineList.push_back(NewFun);
			CallMatrix[FindFuncIndex(*iter)][FuncList.size() - 1] = 2;
		}
	}
	if (doOutLineList.empty() || doInLineList.empty())
	{
		return false;
	}
	return true;
}
/// \Gets a list of Inline functions
BOOL CallGraphRandom::GetInLineFuncList()
{
	vector<pair<Function*, int>> FuncSemantics;
	FuncSemantics.clear();
	InLineFuncList.clear();
	int CallNum = 0,SumCallNum=0;
	int size = FuncList.size();
	for (int i = 0; i < size; i++)
	{
		if (find(OutLineFuncList.begin(), OutLineFuncList.end(), FuncList[i]) != OutLineFuncList.end())
		{
			continue;
		}
		for (User *U : FuncList[i]->users())
		{
			CallNum++;
		}
		FuncSemantics.push_back(make_pair(FuncList[i], CallNum));
		CallNum = 0;
	}
	pair<Function*, int> tmp;
	size = FuncSemantics.size();
	for (int i = 0; i != size; ++i)
	{
		for (int j = 0; j != size; ++j)
		{
			if (FuncSemantics[i].second > FuncSemantics[j].second)
			{
				tmp = FuncSemantics[i];
				FuncSemantics[i] = FuncSemantics[j];
				FuncSemantics[j] = tmp;
			}		
		}
	}
	int InlineSize = 0;
	if (size < 10)
	{
		InlineSize = size;
	}
	else
	{
		InlineSize = size * 0.5;
	}	
	for (int i = 0; i < InlineSize; ++i)
	{
		InLineFuncList.push_back(FuncSemantics[i].first);
	}
	if (InLineFuncList.empty())
	{
		return false;
	}
	return true;
}
/// \Gets a list of Outline functions
BOOL CallGraphRandom::GetOutLineFuncList()
{
	OutLineFuncList.clear();
	int size = FuncList.size();
	for (int i = 0; i < size; i++)
	{
		if (FuncList[i]->size() < 10|| FuncList[i]->size()>200)
			continue;	
		if (doKeyBranch.ishasKeyBranch(FuncList[i]))
		{			
			OutLineFuncList.push_back(FuncList[i]);
		}
	}
	return true;
}
/// \Generate Function Call Graph
BOOL CallGraphRandom::GenCallGraph(Module* M)
{
	if (!InitialFuncList(M))
	{
		errs() << "Failed to Initial Function List\n";
		return false;
	}
	if (!GenCallMatrix())
	{
		errs() << "Failed to Generate Function Call Matrix\n";
		return false;
	}
	if (!GetOutLineFuncList())
	{
		errs() << "Failed to Get OutLineFuncList\n";
		return false;
	}
	if (!GetInLineFuncList())
	{
		errs() << "Failed to Get InLineFuncList\n";
		return false;
	}
	
	if (!ERGraph())
	{
		errs() << "Failed to Generate  Erdos-Renyi Call Graph\n";
		return false;
	}
	return true;
}