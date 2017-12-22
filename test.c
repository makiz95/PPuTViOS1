#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

int main()
{
	int64_t mjdTime = 55097;

	uint16_t Year;
	uint16_t tmpYear;
    uint16_t tmpMonth;
	uint16_t day;
	uint16_t K;

	tmpYear = (int) ((mjdTime - 15078.2) / 365.25);
    tmpMonth = (int) ((mjdTime - 14956.1 - (int) (tmpYear * 365.25)) 
        / 30.6001);
        
    if (tmpMonth == 14 || tmpMonth == 15)
    {
        K = 1;
    }
    else
    {
        K = 0;
    }
	tmpYear = tmpYear + K;
	tmpMonth = tmpMonth - 1 - K * 12;
    day = mjdTime - 14987 - (int) (tmpYear * 365.25)  - (int) (tmpMonth * 30.6001);
	Year=tmpYear+1900;
 
	printf("Rad na sortiranju!");
printf("INFO: Time read from stream: %hu/%hu/%hu \n", Year, tmpMonth, day);
switch(tmpMonth)
   { 
	case 1:
		 printf("INFO: Time read from stream: January/%hu/%hu \n", day, Year);
		  break;
	case 2:
		   printf("INFO: Time read from stream: February/%hu/%hu \n", day, Year);
		  break;
	case 3:
		  printf("INFO: Time read from stream: March/%hu/%hu \n", day, Year);
		  break;
	case 4:
		  printf("INFO: Time read from stream: April/%hu/%hu \n", day, Year);;
		  break;
	case 5:
		 printf("INFO: Time read from stream: May/%hu/%hu \n", day, Year);
		  break;
	case 6:
		  printf("INFO: Time read from stream: June/%hu/%hu \n", day, Year);
		  break;
	case 7:
		 printf("INFO: Time read from stream: July/%hu/%hu \n", day, Year);
		  break;
	case 8:
		   printf("INFO: Time read from stream: August/%hu/%hu \n", day, Year);
		  break;
	case 9:
		  printf("INFO: Time read from stream: September/%hu/%hu \n", day, Year);
		  break;
	case 10:
		  printf("INFO: Time read from stream: October/%hu/%hu \n", day, Year);
		  break;
	case 11:
		   printf("INFO: Time read from stream: November/%hu/%hu \n", day, Year);;
		  break;
	case 12:
		   printf("INFO: Time read from stream: December/%hu/%hu \n", day, Year);
		  break;
	}
    printf("INFO: Time read from stream: %hu/%hu/%hu \n", Year, tmpMonth, day);

	return 0;
}
