#include<stdio.h>
#include<string.h>

int main()
{
	unsigned char x[2]={0xff,0x11};
	unsigned char a[4]="0xff";
	//strcpy(&x,a);
	printf("%c\n",x[0]);
	return 0;
} 
