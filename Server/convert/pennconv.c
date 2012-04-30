/* Original Code provided by Zenty (c) 1999, All rights reserved
 * Modified by Ashen-Shugar for attribute allignment and functionary locks
 * (c) 1999, All rights reserved
 */
#include <stdio.h>
#include <string.h>
#define IMMORTAL  0x00200000
#define ADMIN     0x00000100
#define RANSI     0x02000000
#define ANSICOLOR 0x04000000
#define BUILDER   0x20000000
#define ROYALTY   0x20000000
#define STAFF     0x00010000
#define ANSI      0x00000200
#define MAGE      0x00000001
#define WIZARD    0x00000010
#define MUXANSI   0x00000200
#define TINYANSI  0x00040000

int main(void) {
	int val, flag1, flag2, flag3, nflag1, nflag2, nflag3, obj;
	int mage, royalty, staff, ansi, immortal, offsetchk;
	char f[16384], *q, *f1, *f2;
	memset(f,'\0', sizeof(f));
	q = f;
	f1 = f+1;
	f2 = f+2;
	offsetchk = 0;
	
	gets(q);
	while(q != NULL && !feof(stdin) ) {
		if(f[0] == '!') {
			obj=atoi(f1);
			/* object conversion */
			printf("%s\n",q);
			gets(q); printf("%s\n",q); /* name */
			gets(q); /* List info - Rhost doesn't have */
			gets(q); printf("%s\n",q); /* location */
			gets(q); printf("%s\n",q); /* Contents */
			gets(q); printf("%s\n",q); /* Exits */
/*			gets(q); printf("%s\n",q);*/ /* Link */ /* PENN doesn't have */
			gets(q); printf("%s\n",q); /* Next */
			gets(q); printf("%s\n",q); /* Parent */
			gets(q);
			   if ( strchr(f, '/') == 0 && strchr(f, ':') == 0 ) {
				   printf("%s\n", q); /* Bool */
			   } else {
				   printf("%s\n", q); /* Functionary Lock */
/*				   gets(q);printf("%s\n",q); */  /* Bool */ /* Tiny doesn't have */
				   offsetchk = 1;
			   }
			/* If previous was null, last was owner, not bool */
/*			if ( (!( q && *q) && offsetchk) || !offsetchk ) { */
			   gets(q); printf("%s\n",q); /* Owner */
/*			} */
			offsetchk = 0;
                        gets(q); /* Zone - Rhost doesn't use PENN zones */
			gets(q); printf("%s\n",q); /* Pennies */
			/* flag conv */
			gets(q); flag1 = atoi(q); 
  			gets(q); /* PENN toggles - Rhost doesn't use */
			nflag1 = (flag1 & 0xFFDFFFFF);
                        if ( nflag2 & TINYANSI )
			   nflag2 = (flag2 & 0xD60000FF);
                        else
			   nflag2 = (flag2 & 0xD00000FF);
			nflag3 = 0;			
/* Don't convert flags.  Players may not want it */			
/*		        if((flag1 & WIZARD) != 0) {
 *				  nflag1 |=IMMORTAL;
 *			} else if((flag1 & ROYALTY) != 0) {
 *				nflag1 = (flag1 & ~ROYALTY);
 *				nflag2 |= ADMIN;
 *			} else if((flag1 & IMMORTAL) != 0) {
 *				nflag1 = (flag1 & ~IMMORTAL);
 *				nflag2 |= BUILDER;
 * 			} 
 */
			if((flag2 & ANSI) != 0) {
				nflag2 = (flag2 & ~ANSI);
				nflag2 |= RANSI;
				nflag2 |= ANSICOLOR;
			}
			if((obj == 1) && !(nflag1 & IMMORTAL))
			  nflag1 |= IMMORTAL; // This should not happen.
			printf("%i\n",nflag1);
			printf("%i\n",nflag2);
  			printf("%i\n",nflag3);
			fflush(stdout);
  			gets(q);   /* powers  */
                        gets(q);   /* Channels */
                        gets(q);   /* Warnings */
                        gets(q);   /* Time stuff */
                        gets(q);   /* Local Data */
/*			gets(q);*/ /* power 2 */ 
			printf("0\n"); /* Flags4 */
			printf("0\n"); /* toggles */
                        printf("0\n"); /* toggles1 */
			printf("0\n"); /* toggles2 */
			printf("0\n"); /* toggles3 */
			printf("0\n"); /* toggles4 */
			printf("0\n"); /* toggles5 */
			printf("0\n"); /* toggles6 */
			printf("0\n"); /* toggles7 */
			printf("-1\n"); /* Unknown */
			fflush(stdout);
			gets(q);
		} else 
		    if(f[0] == '+') {
			    if((f[1] == 'A') || (f[1] == 'N')) {
				    val = atoi(f2);
				    if(val > 255)
					val = val + 256;
				    printf("+%c%d\n",f[1],val);
				    fflush(stdout);
				    gets(q);
			    } else if(f[1] == 'X') {
				    printf("+V74247\n",val);
				    fflush(stdout);
				    gets(q);
			    } else {
				    printf("%s\n",q);
				    fflush(stdout);
				    gets(q);
			    }
		    } else
		    if(f[0] == '>') {
			    val = atoi(f1);
			    if(val > 255)
				val = val + 256;
			    /* attr conversion */
			    if((val == 96) || ((val > 199) && (val < 222))) {
				    gets(q);
				    gets(q);
			    } else  {
				    printf(">%d\n",val);
				    fflush(stdout);
				    gets(q);
				    printf("%s\n",q);
				    gets(q);
			    }
		    } else 
		    if(f[0] == '-') {
			    gets(q);
		    } else {
			    printf("%s\n",q);
			    fflush(stdout);
			    gets(q);
		    }
		
		if(strstr(f, "***END OF DUMP***") != NULL )
		  {
			  printf("***END OF DUMP***\n");
			  fflush(stdout);
			  q = NULL;
			  return(0);
		  }
	} /* while */
	return(0);	
}
