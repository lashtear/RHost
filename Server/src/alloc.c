/* alloc.c - memory allocation subsystem */

#include "copyright.h"
#include "autoconf.h"

#include "db.h"
#include "alloc.h"
#include "mudconf.h"
#include "externs.h"

/* ensure quad byte divisable length to avoid bus error on some machines */
#define QUADALIGN(x) ((pmath1)(x) % ALLIGN1 ? \
                      (x) + ALLIGN1 - ((pmath1)(x) % ALLIGN1) : (x))

/* this structure size must be x % 4 = 0 */
typedef struct pool_header {
    int magicnum;		/* For consistency check */
    int pool_size;		/* For consistency check */
    struct pool_header *next;	/* Next pool header in chain */
    struct pool_header *nxtfree;	/* Next pool header in freelist */
    char *buf_tag;		/* Debugging/trace tag */
} POOLHDR;

typedef struct pool_footer {
    int magicnum;               /* For consistency check */
} POOLFTR;

typedef struct pooldata {
    int pool_size;		/* Size in bytes of a buffer */
    POOLHDR *free_head;		/* Buffer freelist head */
    POOLHDR *chain_head;	/* Buffer chain head */
    double tot_alloc;		/* Total buffers allocated */
    double num_alloc;		/* Number of buffers currently allocated */
    double max_alloc;		/* Max # buffers allocated at one time */
    double num_lost;		/* Buffers lost due to corruption */
} POOL;

POOL pools[NUM_POOLS];
const char *poolnames[] =
{"Sbufs", "Mbufs", "Lbufs", "Bools", "Descs", "Qentries", "Pcaches", "ZListNodes"};

#define POOL_MAGICNUM 0xdeadbeef

int 
rtpool()
{
    return pools[2].num_alloc;
}

void 
pool_init(int poolnum, int poolsize)
{
    pools[poolnum].pool_size = poolsize;
    pools[poolnum].free_head = NULL;
    pools[poolnum].chain_head = NULL;
    pools[poolnum].max_alloc = 0;
    pools[poolnum].num_alloc = 0;
    pools[poolnum].tot_alloc = 0;
    pools[poolnum].num_lost = 0;
    return;
}

static void 
pool_err(const char *logsys, int logflag, int poolnum,
	 const char *tag, POOLHDR * ph, const char *action,
	 const char *reason, int line_num, char *file_name)
{
    if (!mudstate.logging) {
	STARTLOG(logflag, logsys, "ALLOC")
	    sprintf(mudstate.buffer,
		    "%.10s[%d] (tag %.40s) %.50s at %x (line %d of %.20s).",
		    action, pools[poolnum].pool_size, tag, reason,
		    (int) ph, line_num, file_name);
	log_text(mudstate.buffer);
	ENDLOG
    } else if (logflag != LOG_ALLOCATE) {
	sprintf(mudstate.buffer,
		"\n***< %.10s[%d] (tag %.40s) %.50s at %x (line %d of %.20s). >***",
		action, pools[poolnum].pool_size, tag, reason,
		(int) ph, line_num, file_name);
	log_text(mudstate.buffer);
    }
}

static void 
pool_vfy(int poolnum, const char *tag, int line_num, char *file_name)
{
    POOLHDR *ph, *lastph;
    POOLFTR *pf;
    char *h;
    int psize;

    lastph = NULL;
    psize = pools[poolnum].pool_size;
    for (ph = pools[poolnum].chain_head; ph; lastph = ph, ph = ph->next) {
	h = (char *) ph;
	h += sizeof(POOLHDR);
	h += pools[poolnum].pool_size;
        h = QUADALIGN(h);
	pf = (POOLFTR *) h;

	if (ph->magicnum != POOL_MAGICNUM) {
	    pool_err("BUG", LOG_ALWAYS, poolnum, tag, ph,
		     "Verify", "header corrupted (clearing freelist)", line_num, file_name);

	    /* Break the header chain at this point so we don't
	     * generate an error for EVERY alloc and free,
	     * also we can't continue the scan because the next
	     * pointer might be trash too.
	     */

	    if (lastph)
		lastph->next = NULL;
	    else
		pools[poolnum].chain_head = NULL;
	    return;		/* not safe to continue */
	}
	if (pf->magicnum != POOL_MAGICNUM) {
	    pool_err("BUG", LOG_ALWAYS, poolnum, tag, ph,
		     "Verify", "footer corrupted", line_num, file_name);
	    pf->magicnum = POOL_MAGICNUM;
	}
	if (ph->pool_size != psize) {
	    pool_err("BUG", LOG_ALWAYS, poolnum, tag, ph,
		     "Verify", "header has incorrect size", line_num, file_name);
	}
    }
}

void 
pool_check(const char *tag, int line_num, char *file_name)
{
    pool_vfy(POOL_LBUF, tag, line_num, file_name);
    pool_vfy(POOL_MBUF, tag, line_num, file_name);
    pool_vfy(POOL_SBUF, tag, line_num, file_name);
    pool_vfy(POOL_BOOL, tag, line_num, file_name);
    pool_vfy(POOL_DESC, tag, line_num, file_name);
    pool_vfy(POOL_QENTRY, tag, line_num, file_name);
    pool_vfy(POOL_ZLISTNODE, tag, line_num, file_name);
}

char *
pool_alloc(int poolnum, const char *tag, int line_num, char *file_name)
{
    int *p;
    char *h;
    POOLHDR *ph;
    POOLFTR *pf;

    if (mudconf.paranoid_alloc)
	pool_check(tag, line_num, file_name);
    do {
	if (pools[poolnum].free_head == NULL) {
            /* add 3 to size to allow for quad byte alignment */
	    h = (char *) malloc(3 + pools[poolnum].pool_size +
				sizeof(POOLHDR) + sizeof(POOLFTR));
	    if (h == NULL) {
		STARTLOG(LOG_ALWAYS, "MEM", "FATAL")
                  log_text((char *)"pool_alloc failed!");
	          sprintf(mudstate.buffer, " (line %d of file %s)", 
                          line_num, file_name);
                  log_text((char *)mudstate.buffer);
                ENDLOG
		abort();
            }
	    ph = (POOLHDR *) h;
	    h += sizeof(POOLHDR);
	    p = (int *) h;
	    h += pools[poolnum].pool_size;
            h = QUADALIGN(h);
	    pf = (POOLFTR *) h;

	    ph->next = pools[poolnum].chain_head;
	    ph->nxtfree = NULL;
	    ph->magicnum = POOL_MAGICNUM;
	    ph->pool_size = pools[poolnum].pool_size;
	    pf->magicnum = POOL_MAGICNUM;
	    *p = POOL_MAGICNUM;
	    pools[poolnum].chain_head = ph;
	    pools[poolnum].max_alloc++;
	} else {
	    ph = (POOLHDR *) (pools[poolnum].free_head);
	    h = (char *) ph;
	    h += sizeof(POOLHDR);
	    p = (int *) h;
	    h += pools[poolnum].pool_size;
            h = QUADALIGN(h);
	    pf = (POOLFTR *) h;
	    pools[poolnum].free_head = ph->nxtfree;

	    /* If corrupted header we need to throw away the
	     * freelist as the freelist pointer may be corrupt.
	     */

	    if (ph->magicnum != POOL_MAGICNUM) {
		pool_err("BUG", LOG_ALWAYS, poolnum, tag, ph,
			 "Alloc", "corrupted buffer header", line_num, file_name);

		/* Start a new free list and record stats */

		p = NULL;
		pools[poolnum].free_head = NULL;
		pools[poolnum].num_lost +=
		    (pools[poolnum].max_alloc -
		     pools[poolnum].num_alloc);
		pools[poolnum].max_alloc =
		    pools[poolnum].num_alloc;
	    }
	    /* Check for corrupted footer, just report and
	     * fix it */

	    if (pf->magicnum != POOL_MAGICNUM) {
		pool_err("BUG", LOG_ALWAYS, poolnum, tag, ph,
			 "Alloc", "corrupted buffer footer", line_num, file_name);
		pf->magicnum = POOL_MAGICNUM;
	    }
	}
    } while (p == NULL);

    ph->buf_tag = (char *) tag;
    pools[poolnum].tot_alloc++;
    pools[poolnum].num_alloc++;

    pool_err("DBG", LOG_ALLOCATE, poolnum, tag, ph, "Alloc", "buffer", line_num, file_name);

    /* If the buffer was modified after it was last freed, log it. */

    if ((*p != POOL_MAGICNUM) && (!mudstate.logging)) {
	pool_err("BUG", LOG_PROBLEMS, poolnum, tag, ph, "Alloc",
		 "buffer modified after free", line_num, file_name);
    }
    *p = 0;
    return (char *) p;
}

int
getBufferSize(char *pBuff) 
{
    POOLHDR *ph;
    ph = (POOLHDR *) (pBuff - sizeof(POOLHDR));
    if (ph->magicnum != POOL_MAGICNUM) {
       return 0;
    } else {
       return ph->pool_size;
    }
}


void 
pool_free(int poolnum, char **buf, int line_num, char *file_name)
{
    int *ibuf;
    char *h;
    POOLHDR *ph;
    POOLFTR *pf;

    ibuf = (int *) *buf;
    h = (char *) ibuf;
    h -= sizeof(POOLHDR);
    ph = (POOLHDR *) h;
    h = (char *) ibuf;    
    h += pools[poolnum].pool_size;
    h = QUADALIGN(h);
    pf = (POOLFTR *) h;
    if (mudconf.paranoid_alloc)
	pool_check(ph->buf_tag, line_num, file_name);

    /* Make sure the buffer header is good.  If it isn't, log the error and
     * throw away the buffer. */

    if (ph->magicnum != POOL_MAGICNUM) {
	pool_err("BUG", LOG_ALWAYS, poolnum, ph->buf_tag, ph, "Free",
		 "corrupted buffer header", line_num, file_name);
	pools[poolnum].num_lost++;
	pools[poolnum].num_alloc--;
	pools[poolnum].tot_alloc--;
	return;
    }
    /* Verify the buffer footer.  Don't unlink if damaged, just repair */

    if (pf->magicnum != POOL_MAGICNUM) {
	pool_err("BUG", LOG_ALWAYS, poolnum, ph->buf_tag, ph, "Free",
		 "corrupted buffer footer", line_num, file_name);
	pf->magicnum = POOL_MAGICNUM;
    }
    /* Verify that we are not trying to free someone else's buffer */

    if (ph->pool_size != pools[poolnum].pool_size) {
	pool_err("BUG", LOG_ALWAYS, poolnum, ph->buf_tag, ph, "Free",
		 "Attempt to free into a different pool.", line_num, file_name);
	return;
    }
    pool_err("DBG", LOG_ALLOCATE, poolnum, ph->buf_tag, ph, "Free",
	     "buffer", line_num, file_name);

    /* Make sure we aren't freeing an already free buffer.  If we are,
     * log an error, otherwise update the pool header and stats 
     */

    if (*ibuf == POOL_MAGICNUM) {
	pool_err("BUG", LOG_BUGS, poolnum, ph->buf_tag, ph, "Free",
		 "buffer already freed", line_num, file_name);
    } else {
	*ibuf = POOL_MAGICNUM;
	ph->nxtfree = pools[poolnum].free_head;
	pools[poolnum].free_head = ph;
	pools[poolnum].num_alloc--;
    }
}

static char *
pool_stats(int poolnum, const char *text)
{
    char *buf;
    char format_str[80];

    buf = alloc_mbuf("pool_stats");
    memset(format_str, 0, sizeof(format_str));
    strcpy(format_str, "%-14.14s %5d");
    if ( pools[poolnum].num_alloc > 999999999 )
       strcat(format_str, " %9.4g");
    else
       strcat(format_str, " %9.9g");
    if ( pools[poolnum].max_alloc > 999999999 )
       strcat(format_str, " %9.4g");
    else
       strcat(format_str, " %9.9g");
    if ( pools[poolnum].tot_alloc > 9.99999e11 )
       strcat(format_str, " %12.7g");
    else
       strcat(format_str, " %12.12g");
    if ( pools[poolnum].num_lost > 999999999)
       strcat(format_str, " %9.4g");
    else
       strcat(format_str, " %9.9g");
    if ( (pools[poolnum].max_alloc * pools[poolnum].pool_size) > 9.99999e13 )
       strcat(format_str, " %14.9g");
    else
       strcat(format_str, " %14.14g");
    sprintf(buf, format_str, text, pools[poolnum].pool_size,
	    pools[poolnum].num_alloc, pools[poolnum].max_alloc,
	    pools[poolnum].tot_alloc, pools[poolnum].num_lost,
            pools[poolnum].max_alloc * pools[poolnum].pool_size);
    return buf;
}

static void 
pool_trace(dbref player, int poolnum, const char *text)
{
    POOLHDR *ph;
    typedef struct tmp_holder {
              char buff_name[200];
              int i_cntr;
              struct tmp_holder *next;
           } THOLD;
    THOLD *st_holder = NULL, *st_ptr = NULL, *st_ptr2 = NULL;
    int numfree, *ibuf;
    char *h, s_fieldbuff[250];

    numfree = 0;
    notify(player, unsafe_tprintf("----- %s -----", text));
    for (ph = pools[poolnum].chain_head; ph != NULL; ph = ph->next) {
	if (ph->magicnum != POOL_MAGICNUM) {
	    notify(player, "*** CORRUPTED BUFFER HEADER, ABORTING SCAN ***");
	    notify(player,
		   unsafe_tprintf("%d free %s (before corruption)",
			   numfree, text));
            st_ptr = st_holder;
            while ( st_ptr ) {
               st_ptr2 = st_ptr->next;
               free(st_ptr);
               st_ptr = st_ptr2;
            }
	    return;
	}
	h = (char *) ph;
	h += sizeof(POOLHDR);
	ibuf = (int *) h;
	if (*ibuf != POOL_MAGICNUM) {
/*	    notify(player, ph->buf_tag); */
            for (st_ptr = st_holder; st_ptr; st_ptr = st_ptr->next) {
               if ( !strcmp( ph->buf_tag, st_ptr->buff_name) ) {
                  st_ptr->i_cntr++;
                  break;
               }
            }
            if ( !st_ptr ) {
               st_ptr2 = malloc(sizeof(THOLD));
               sprintf(st_ptr2->buff_name, "%.199s", ph->buf_tag);
               st_ptr2->i_cntr = 1;
               st_ptr2->next = NULL;
               if ( !st_holder )
                  st_holder = st_ptr2;
               else {
                  for (st_ptr = st_holder; st_ptr && st_ptr->next; st_ptr = st_ptr->next);
                  st_ptr->next = st_ptr2;
               }
            }   
        } else
	    numfree++;
    }
    st_ptr = st_holder;
    while ( st_ptr ) {
       sprintf(s_fieldbuff, "%s  [%d entries]", st_ptr->buff_name, st_ptr->i_cntr);
       notify(player, s_fieldbuff);
       st_ptr2 = st_ptr->next;
       free(st_ptr);
       st_ptr = st_ptr2;
    }
    notify(player, unsafe_tprintf("%d free %s", numfree, text));
}

static void 
list_bufstat(dbref player, int poolnum, const char *pool_name)
{
    char *buff;

    buff = pool_stats(poolnum, poolnames[poolnum]);
    notify(player, buff);
    free_mbuf(buff);
}

void 
list_bufstats(dbref player)
{
    int i;

    notify(player, "Buffer Stats    Size     InUse     Total       Allocs      Lost      Total Mem");
    for (i = 0; i < NUM_POOLS; i++)
	list_bufstat(player, i, poolnames[i]);

    showTrackedBufferStats(player);

    showdbstats(player);

}

void 
list_buftrace(dbref player)
{
    int i;

    for (i = 0; i < NUM_POOLS; i++)
	pool_trace(player, i, poolnames[i]);
}

#define BFACT 2
#define BMIN 10

void do_buff_free(dbref player, dbref cause, int key)
{
  int x, y, z;
  POOLHDR *ph;

  for (x = 0; x < NUM_POOLS; x++) {
    if (BFACT)
      y = pools[x].num_alloc + (pools[x].num_alloc / BFACT);
    else
      y = pools[x].num_alloc;
    if (!y)
      y = BMIN;
    if (y < pools[x].max_alloc) {
      for (z = 0; z < (pools[x].max_alloc - y); z++) {
        ph = (POOLHDR *)(pools[x].free_head);
	if (ph == NULL) 
	  break;
	pools[x].free_head = ph->nxtfree;
	if (ph->magicnum != POOL_MAGICNUM) {
	  pool_err("BUG", LOG_ALWAYS, x, "buff_free", ph, "Alloc", "corrupted buffer header", 
                   -1, (char *)"");
	  pools[x].free_head = NULL;
	  pools[x].num_lost += (pools[x].max_alloc - pools[x].num_alloc);
	  pools[x].max_alloc = pools[x].num_alloc;
	  notify(player,"Corrupted buffer header detected");
	  break;
	}
	free ((char *)ph);
	pools[x].max_alloc--;
      }
    }
  }
 notify(player,"Buff free: Done");
}
