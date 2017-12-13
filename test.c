#include <stdio.h>
#include <string.h>


int main(filename)
{
	char red[50];
	char znak_za_parsiranje[2] = ":";
	FILE *file;
	file = fopen("config.ini", "r");
	char c;
	if( (file=fopen("config.ini","r")) == NULL )
	{
		printf("Datoteka <%s> nije uspesno otvorena.");
		return -1;
	}
	
	while(fgets(red,50,file) != NULL)
		puts(red);
		printf("\n>>Kraj sadrzaja datoteke <%s>...\n", "config.ini");

	fclose(file);
	return 0;
}

