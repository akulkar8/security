#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>

char plainText[100];
char userKey[100];
char cypherText[100];
//-----------------------------------------------------------------------------------------
#define w 32    /* word size in bits */
#define r 20    /* based on security estimates */
 
#define P32 0xB7E15163  /* Magic constants for key setup */
#define Q32 0x9E3779B9
 
/* derived constants */
#define bytes   (w / 8)             /* bytes per word */
#define c       ((b + bytes - 1) / bytes)   /* key in words, rounded up */
#define R24     (2 * r + 4)
#define lgw     5                           /* log2(w) -- wussed out */
 
/* Rotations */
#define ROTL(x,y) (((x)<<(y&(w-1))) | ((x)>>(w-(y&(w-1)))))
#define ROTR(x,y) (((x)>>(y&(w-1))) | ((x)<<(w-(y&(w-1)))))
 
unsigned int S[R24 - 1];        /* Key schedule */
//------------------------------------------------------------------------------------


typedef struct 
{
	int keylen;
    	unsigned char key[32];
    	unsigned int pt[4];
    	unsigned int ct[4];
}testStruct;

testStruct *test;

void readfile(FILE *fp);
void *processKey(char *userKey);
int getSize(char *userKey);
void *processText(char *text);
void rc6_block_encrypt(unsigned int *pt, unsigned int *ct);
void rc6_key_setup(unsigned char *K, int b);

int main()
{
	unsigned int ct[4];
	char *pText;
	test=(testStruct*)malloc(sizeof(testStruct));
	FILE *fp=fopen("input.txt","r");
	if(fp==NULL)
	{
		printf("error opening file");
		exit(0);
	}
	readfile(fp);
	processKey(userKey);
	processText(plainText);
	rc6_key_setup(test->key, test->keylen);
        rc6_block_encrypt(test->pt, ct);
	 printf("Cypher Text: %08x %08x %08x %08x\n",
            ct[0], ct[1], ct[2], ct[3]);
	return 0;
		
}

void readfile(FILE *fp)
{
	if(fgets(plainText, 100, fp) != NULL)
	{
		if(fgets(userKey, 100, fp)==NULL)
		{
			printf("error in key\n");
			exit(0);
		}
	}
	else
	{
		printf("error reading file\n");
		exit(0);
	}
}

void *processText(char *text)
{
	int textLen=getSize(text)-1;
	unsigned int *textArr= (int *)malloc(sizeof(int)*4);
	unsigned char *pt=(unsigned char *)malloc(sizeof(unsigned char)* 4);
	unsigned char *arr=(unsigned char *)malloc(sizeof(unsigned char)* (textLen));
	int i=0;
	char *pch;
	pch=strtok(text," ");
	
	while(pch!=NULL)
	{
		strcpy(&arr[i],pch+0);
		i++;
		strcpy(&arr[i],pch+1);
		i++;
		pch=strtok(NULL," ");
	}
	char myString[12]="0x";
	char temp;
	int j=0;
	for(i=0;i<32;i++)
	{
		
		strncat(myString,arr+i,1);
		if((i+1)%8==0)
		{
			printf("%s\n",myString);
			sscanf(myString,"%x",textArr+j);
			j++;
			strcpy(myString,"0x");
		}
	}	
	for(i=0;i<4;i++)
	{
		test->pt[i]=textArr[i];
	}
}



void *processKey(char *userKey)
{
	
	int userKeyLen;
	userKeyLen=getSize(userKey)-1;
	test->keylen=userKeyLen/2;
	
	unsigned int x;
	
	unsigned int *arr=(unsigned int *)malloc(sizeof(unsigned int)* (userKeyLen/2));
	unsigned char *charArr=(unsigned char *)malloc(sizeof(unsigned char)* (userKeyLen/2));
	

	int i=0;
	int j=userKeyLen;
	
	char *pch;
	pch= strtok(userKey," ");
	
	
	while(pch!=NULL)
	{
		char tempString[10]="0x";
		strncat(tempString,pch,2);
		//printf("%s ",tempString);
		sscanf(tempString,"%x",arr+i);	
		pch= strtok(NULL," ");
		
		
		i++;
		
	}
	
	

	for(i=0;i<16;i++)
	{
		test->key[i]=arr[i];
		
	}
	
	
}

int getSize(char *userKey)
{
	int size=0;
	int i=0;
	for(i=0;i<strlen(userKey);i++)
	{
		if(userKey[i]!= ' ')
			size++;
	}
	
	return size;
}	


//----------------------key Generation----------------------------
void rc6_key_setup(unsigned char *K, int b)
{
    int i, j, s, v;
    unsigned int L[(32 + bytes - 1) / bytes]; 
    unsigned int A, B;
 
    L[c - 1] = 0;
    for (i = b - 1; i >= 0; i--)
        L[i / bytes] = (L[i / bytes] << 8) + K[i];
 
    S[0] = P32;
    for (i = 1; i <= 2 * r + 3; i++)
        S[i] = S[i - 1] + Q32;
 
    A = B = i = j = 0;
    v = R24;
    if (c > v) v = c;
    v *= 3;
 
    for (s = 1; s <= v; s++)
    {
        A = S[i] = ROTL(S[i] + A + B, 3);
        B = L[j] = ROTL(L[j] + A + B, A + B);
        i = (i + 1) % R24;
        j = (j + 1) % c;
    }
}

//------------------------------------------------------------------------
void rc6_block_encrypt(unsigned int *pt, unsigned int *ct)
{
    unsigned int A, B, C, D, t, u, x;
    int i, j;
 
    A = pt[0];
    B = pt[1];
    C = pt[2];
    D = pt[3];
    B += S[0];
    D += S[1];
    for (i = 2; i <= 2 * r; i += 2)
    {
        t = ROTL(B * (2 * B + 1), lgw);
        u = ROTL(D * (2 * D + 1), lgw);
        A = ROTL(A ^ t, u) + S[i];
        C = ROTL(C ^ u, t) + S[i + 1];
        x = A;
        A = B;
        B = C;
        C = D;
        D = x;
    }
    A += S[2 * r + 2];
    C += S[2 * r + 3];
    ct[0] = A;
    ct[1] = B;
    ct[2] = C;
    ct[3] = D;
}

/*----------------------------------------------------------------------*/
