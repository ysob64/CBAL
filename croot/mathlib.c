// I know math.h exist, but I only want what I need. Why would I have cos() or sin() function for example ? that would be useless !
#include "../hroot/macros.h"

int mpow(int n,int e)
{
	int sign=e;

	if(e==0)
	{
		return 1;
	}

	if(e==1)
	{
		return n;
	}

	if(e<0)
	{
		e*=-1;
	}

	for(int i=0;i<e-1;i++)
	{
		n*=n;
	}

	return sign>0 ? n : 1/n;
}

int abs(int n)
{
	if(n<0)
	{
		return n*-1;
	}

	return n;
}