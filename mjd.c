#include <stdio.h>
#include <string.h>

int main() 
{

	printf("Test");
	float num1 = 15076.2;
	float num2 = 14956.1;
	float num3 = 30.6001;
	float num4 = 365.2;
	int MJD = 55097;
	int k = 0;
	


	i_year = ((MJD - num1)/num4);
	i_month = (MJD - num2 - (i_year * num4)/num3);
	day = MJD - num2 - (i_year * num4) - (i_month * num3);


	if ((i_month = 14) || (i_month = 15))
	{
		k = 1;
	} 
	else
	{
		k = 0;
	}

	year = i_year + k;
	month = i_month- 1 - k*12;

	printf("Test");

	return 0;
;

}
