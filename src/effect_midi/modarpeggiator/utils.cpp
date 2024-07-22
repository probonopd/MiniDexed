#include "utils.hpp"

ArpUtils::ArpUtils()
{

}

ArpUtils::~ArpUtils()
{

}

void ArpUtils::swap(uint8_t *a, uint8_t *b)
{
	int temp = *a;
	*a = *b;
	*b = temp;
}

//got the code for the quick sort algorithm here https://medium.com/human-in-a-machine-world/quicksort-the-best-sorting-algorithm-6ab461b5a9d0
void ArpUtils::quicksort(uint8_t arr[][2], int l, int r)
{
	if (l >= r)
	{
		return;
	}

	int pivot = arr[r][0];

	int cnt = l;

	for (int i = l; i <= r; i++)
	{
		if (arr[i][0] <= pivot)
		{
			swap(&arr[cnt][0], &arr[i][0]);
			swap(&arr[cnt][1], &arr[i][1]);
			cnt++;
		}
	}
	quicksort(arr, l, cnt-2);
	quicksort(arr, cnt, r);
}
