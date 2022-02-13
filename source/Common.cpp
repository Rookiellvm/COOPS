#include "Common.h"


int** MallocGraph(int size)
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
			Graph[i][j] = 0;
		}
	}
	return Graph;
}


