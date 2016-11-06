#pragma once

void swapMin(int & a, int & b)
{
	if (a > b)
	{
		int t = b;
		b = a;
		a = t;
	}
}