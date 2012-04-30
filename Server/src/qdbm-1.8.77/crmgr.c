/*************************************************************************************************
 * Utility for debugging Curia and its applications
 *                                                      Copyright (C) 2000-2007 Mikio Hirabayashi
 * This file is part of QDBM, Quick Database Manager.
 * QDBM is free software; you can redistribute it and/or modify it under the terms of the GNU
 * Lesser General Public License as published by the Free Software Foundation; either version
 * 2.1 of the License or any later version.  QDBM is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * You should have received a copy of the GNU Lesser General Public License along with QDBM; if
 * not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307 USA.
 *************************************************************************************************/


#include <depot.h>
#include <curia.h>
#include <cabin.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#undef TRUE
#define TRUE           1                 /* boolean true */
#undef FALSE
#define FALSE          0                 /* boolean false */

#define ALIGNSIZ       32                /* basic size of alignment */


/* for RISC OS */
#if defined(__riscos__) || defined(__riscos)
#include <unixlib/local.h>
int __riscosify_control = __RISCOSIFY_NO_PROCESS;
#endif


/* global variables */
const char *progname;                    /* program name */


/* function prototypes */
int main(int argc, char **argv);
void usage(void);
char *hextoobj(const char *str, int *sp);
char *dectoiobj(const char *str, int *sp);
int runcreate(int argc, char **argv);
int runput(int argc, char **argv);
int runout(int argc, char **argv);
int runget(int argc, char **argv);
int runlist(int argc, char **argv);
int runoptimize(int argc, char **argv);
int runinform(int argc, char **argv);
int runremove(int argc, char **argv);
int runrepair(int argc, char **argv);
int runexportdb(int argc, char **argv);
int runimportdb(int argc, char **argv);
int runsnaffle(int argc, char **argv);
void pdperror(const char *name);
void printobj(const char *obj, int size);
void printobjhex(const char *obj, int size);
int docreate(const char *name, int bnum, int dnum, int sparse);
int doput(const char *name, const char *kbuf, int ksiz,
          const char *vbuf, int vsiz, int dmode, int lob, int align);
int doout(const char *name, const char *kbuf, int ksiz, int lob);
int doget(const char *name, int opts, const char *kbuf, int ksiz,
          int start, int max, int ox, int lob, int nb);
int dolist(const char *name, int opts, int kb, int vb, int ox);
int dooptimize(const char *name, int bnum, int align);
int doinform(const char *name, int opts);
int doremove(const char *name);
int dorepair(const char *name);
int doexportdb(const char *name, const char *dir);
int doimportdb(const char *name, const char *dir, int bnum, int dnum);
int dosnaffle(const char *name, const char *kbuf, int ksiz, int ox, int nb);


/* main routine */
int main(int argc, char **argv){
  char *env;
  int rv;
  cbstdiobin();
  progname = argv[0];
  if((env = getenv("QDBMDBGFD")) != NULL) dpdbgfd = atoi(env);
  if(argc < 2) usage();
  rv = 0;
  if(!strcmp(argv[1], "create")){
    rv = runcreate(argc, argv);
  } else if(!strcmp(argv[1], "put")){
    rv = runput(argc, argv);
  } else if(!strcmp(argv[1], "out")){
    rv = runout(argc, argv);
  } else if(!strcmp(argv[1], "get")){
    rv = runget(argc, argv);
  } else if(!strcmp(argv[1], "list")){
    rv = runlist(argc, argv);
  } else if(!strcmp(argv[1], "optimize")){
    rv = runoptimize(argc, argv);
  } else if(!strcmp(argv[1], "inform")){
    rv = runinform(argc, argv);
  } else if(!strcmp(argv[1], "remove")){
    rv = runremove(argc, argv);
  } else if(!strcmp(argv[1], "repair")){
    rv = runrepair(argc, argv);
  } else if(!strcmp(argv[1], "exportdb")){
    rv = runexportdb(argc, argv);
  } else if(!strcmp(argv[1], "importdb")){
    rv = runimportdb(argc, argv);
  } else if(!strcmp(argv[1], "snaffle")){
    rv = runsnaffle(argc, argv);
  } else if(!strcmp(argv[1], "version") || !strcmp(argv[1], "--version")){
    printf("Powered by QDBM version %s on %s%s\n",
           dpversion, dpsysname, dpisreentrant ? " (reentrant)" : "");
    printf("Copyright (c) 2000-2007 Mikio Hirabayashi\n");
    rv = 0;
  } else {
    usage();
  }
  return rv;
}


/* print the usage and exit */
void usage(void){
  fprintf(stderr, "%s: administration utility for Curia\n", progname);
  fprintf(stderr, "\n");
  fprintf(stderr, "usage:\n");
  fprintf(stderr, "  %s create [-s] [-bnum num] [-dnum num] name\n", progname);
  fprintf(stderr, "  %s put [-kx|-ki] [-vx|-vi|-vf] [-keep|-cat] [-lob] [-na] "
          "name key val\n", progname);
  fprintf(stderr, "  %s out [-kx|-ki] [-lob] name key\n", progname);
  fprintf(stderr, "  %s get [-nl] [-kx|-ki] [-start num] [-max num] [-ox] [-lob] [-n] name key\n",
          progname);
  fprintf(stderr, "  %s list [-nl] [-k|-v] [-ox] name\n", progname);
  fprintf(stderr, "  %s optimize [-bnum num] [-na] name\n", progname);
  fprintf(stderr, "  %s inform [-nl] name\n", progname);
  fprintf(stderr, "  %s remove name\n", progname);
  fprintf(stderr, "  %s repair name\n", progname);
  fprintf(stderr, "  %s exportdb name dir\n", progname);
  fprintf(stderr, "  %s importdb [-bnum num] [-dnum num] name dir\n", progname);
  fprintf(stderr, "  %s snaffle [-kx|-ki] [-ox] [-n] name key\n", progname);
  fprintf(stderr, "  %s version\n", progname);
  fprintf(stderr, "\n");
  exit(1);
}


/* create a binary object from a hexadecimal string */
char *hextoobj(const char *str, int *sp){
  char *buf, mbuf[3];
  int len, i, j;
  len = strlen(str);
  if(!(buf = malloc(len + 1))) return NULL;
  j = 0;
  for(i = 0; i < len; i += 2){
    while(strchr(" \n\r\t\f\v", str[i])){
      i++;
    }
    if((mbuf[0] = str[i]) == '\0') break;
    if((mbuf[1] = str[i+1]) == '\0') break;
    mbuf[2] = '\0';
    buf[j++] = (char)strtol(mbuf, NULL, 16);
  }
  buf[j] = '\0';
  *sp = j;
  return buf;
}


/* create a integer object from a decimal string */
char *dectoiobj(const char *str, int *sp){
  char *buf;
  int num;
  num = atoi(str);
  if(!(buf = malloc(sizeof(int)))) return NULL;
  *(int *)buf = num;
  *sp = sizeof(int);
  return buf;
}


/* parse arguments of create command */
int runcreate(int argc, char **argv){
  char *name;
  int i, sb, bnum, dnum, rv;
  name = NULL;
  sb = FALSE;
  bnum = -1;
  dnum = -1;
  for(i = 2; i < argc; i++){
    if(!name && argv[i][0] == '-'){
      if(!strcmp(argv[i], "-s")){
        sb = TRUE;
      } else if(!strcmp(argv[i], "-bnum")){
        if(++i >= argc) usage();
        bnum = atoi(argv[i]);
      } else if(!strcmp(argv[i], "-dnum")){
        if(++i >= argc) usage();
        dnum = atoi(argv[i]);
      } else {
        usage();
      }
    } else if(!name){
      name = argv[i];
    } else {
      usage();
    }
  }
  if(!name) usage();
  rv = docreate(name, bnum, dnum, sb);
  return rv;
}


/* parse arguments of put command */
int runput(int argc, char **argv){
  char *name, *key, *val, *kbuf, *vbuf;
  int i, kx, ki, vx, vi, vf, lob, ksiz, vsiz, align, rv;
  int dmode;
  name = NULL;
  kx = FALSE;
  ki = FALSE;
  vx = FALSE;
  vi = FALSE;
  vf = FALSE;
  lob = FALSE;
  align = ALIGNSIZ;
  key = NULL;
  val = NULL;
  dmode = CR_DOVER;
  for(i = 2; i < argc; i++){
    if(!name && argv[i][0] == '-'){
      if(!strcmp(argv[i], "-kx")){
        kx = TRUE;
      } else if(!strcmp(argv[i], "-ki")){
        ki = TRUE;
      } else if(!strcmp(argv[i], "-vx")){
        vx = TRUE;
      } else if(!strcmp(argv[i], "-vi")){
        vi = TRUE;
      } else if(!strcmp(argv[i], "-vf")){
        vf = TRUE;
      } else if(!strcmp(argv[i], "-keep")){
        dmode = CR_DKEEP;
      } else if(!strcmp(argv[i], "-cat")){
        dmode = CR_DCAT;
      } else if(!strcmp(argv[i], "-lob")){
        lob = TRUE;
      } else if(!strcmp(argv[i], "-na")){
        align = 0;
      } else {
        usage();
      }
    } else if(!name){
      name = argv[i];
    } else if(!key){
      key = argv[i];
    } else if(!val){
      val = argv[i];
    } else {
      usage();
    }
  }
  if(!name || !key || !val) usage();
  if(kx){
    kbuf = hextoobj(key, &ksiz);
  } else if(ki){
    kbuf = dectoiobj(key, &ksiz);
  } else {
    kbuf = cbmemdup(key, -1);
    ksiz = -1;
  }
  if(vx){
    vbuf = hextoobj(val, &vsiz);
  } else if(vi){
    vbuf = dectoiobj(val, &vsiz);
  } else if(vf){
    vbuf = cbreadfile(val, &vsiz);
  } else {
    vbuf = cbmemdup(val, -1);
    vsiz = -1;
  }
  if(kbuf && vbuf){
    rv = doput(name, kbuf, ksiz, vbuf, vsiz, dmode, lob, align);
  } else {
    if(vf){
      fprintf(stderr, "%s: %s: cannot read\n", progname, val);
    } else {
      fprintf(stderr, "%s: out of memory\n", progname);
    }
    rv = 1;
  }
  free(kbuf);
  free(vbuf);
  return rv;
}


/* parse arguments of out command */
int runout(int argc, char **argv){
  char *name, *key, *kbuf;
  int i, kx, ki, lob, ksiz, rv;
  name = NULL;
  kx = FALSE;
  ki = FALSE;
  lob = FALSE;
  key = NULL;
  for(i = 2; i < argc; i++){
    if(!name && argv[i][0] == '-'){
      if(!strcmp(argv[i], "-kx")){
        kx = TRUE;
      } else if(!strcmp(argv[i], "-ki")){
        ki = TRUE;
      } else if(!strcmp(argv[i], "-lob")){
        lob = TRUE;
      } else {
        usage();
      }
    } else if(!name){
      name = argv[i];
    } else if(!key){
      key = argv[i];
    } else {
      usage();
    }
  }
  if(!name || !key) usage();
  if(kx){
    kbuf = hextoobj(key, &ksiz);
  } else if(ki){
    kbuf = dectoiobj(key, &ksiz);
  } else {
    kbuf = cbmemdup(key, -1);
    ksiz = -1;
  }
  if(kbuf){
    rv = doout(name, kbuf, ksiz, lob);
  } else {
    fprintf(stderr, "%s: out of memory\n", progname);
    rv = 1;
  }
  free(kbuf);
  return rv;
}


/* parse arguments of get command */
int runget(int argc, char **argv){
  char *name, *key, *kbuf;
  int i, opts, kx, ki, ox, lob, nb, start, max, ksiz, rv;
  name = NULL;
  opts = 0;
  kx = FALSE;
  ki = FALSE;
  ox = FALSE;
  lob = FALSE;
  nb = FALSE;
  start = 0;
  max = -1;
  key = NULL;
  for(i = 2; i < argc; i++){
    if(!name && argv[i][0] == '-'){
      if(!strcmp(argv[i], "-nl")){
        opts |= CR_ONOLCK;
      } else if(!strcmp(argv[i], "-kx")){
        kx = TRUE;
      } else if(!strcmp(argv[i], "-ki")){
        ki = TRUE;
      } else if(!strcmp(argv[i], "-ox")){
        ox = TRUE;
      } else if(!strcmp(argv[i], "-lob")){
        lob = TRUE;
      } else if(!strcmp(argv[i], "-n")){
        nb = TRUE;
      } else if(!strcmp(argv[i], "-start")){
        if(++i >= argc) usage();
        start = atoi(argv[i]);
      } else if(!strcmp(argv[i], "-max")){
        if(++i >= argc) usage();
        max = atoi(argv[i]);
      } else {
        usage();
      }
    } else if(!name){
      name = argv[i];
    } else if(!key){
      key = argv[i];
    } else {
      usage();
    }
  }
  if(!name || !key || start < 0) usage();
  if(kx){
    kbuf = hextoobj(key, &ksiz);
  } else if(ki){
    kbuf = dectoiobj(key, &ksiz);
  } else {
    kbuf = cbmemdup(key, -1);
    ksiz = -1;
  }
  if(kbuf){
    rv = doget(name, opts, kbuf, ksiz, start, max, ox, lob, nb);
  } else {
    fprintf(stderr, "%s: out of memory\n", progname);
    rv = 1;
  }
  free(kbuf);
  return rv;
}


/* parse arguments of list command */
int runlist(int argc, char **argv){
  char *name;
  int i, opts, kb, vb, ox, rv;
  name = NULL;
  opts = 0;
  kb = FALSE;
  vb = FALSE;
  ox = FALSE;
  for(i = 2; i < argc; i++){
    if(!name && argv[i][0] == '-'){
      if(!strcmp(argv[i], "-nl")){
        opts |= CR_ONOLCK;
      } else if(!strcmp(argv[i], "-k")){
        kb = TRUE;
      } else if(!strcmp(argv[i], "-v")){
        vb = TRUE;
      } else if(!strcmp(argv[i], "-ox")){
        ox = TRUE;
      } else {
        usage();
      }
    } else if(!name){
      name = argv[i];
    } else {
      usage();
    }
  }
  if(!name) usage();
  rv = dolist(name, opts, kb, vb, ox);
  return rv;
}


/* parse arguments of optimize command */
int runoptimize(int argc, char **argv){
  char *name;
  int i, bnum, align, rv;
  name = NULL;
  bnum = -1;
  align = ALIGNSIZ;
  for(i = 2; i < argc; i++){
    if(!name && argv[i][0] == '-'){
      if(!strcmp(argv[i], "-bnum")){
        if(++i >= argc) usage();
        bnum = atoi(argv[i]);
      } else if(!strcmp(argv[i], "-na")){
        align = 0;
      } else {
        usage();
      }
    } else if(!name){
      name = argv[i];
    } else {
      usage();
    }
  }
  if(!name) usage();
  rv = dooptimize(name, bnum, align);
  return rv;
}


/* parse arguments of inform command */
int runinform(int argc, char **argv){
  char *name;
  int i, opts, rv;
  name = NULL;
  opts = 0;
  for(i = 2; i < argc; i++){
    if(!name && argv[i][0] == '-'){
      if(!strcmp(argv[i], "-nl")){
        opts |= CR_ONOLCK;
      } else {
        usage();
      }
    } else if(!name){
      name = argv[i];
    } else {
      usage();
    }
  }
  if(!name) usage();
  rv = doinform(name, opts);
  return rv;
}


/* parse arguments of remove command */
int runremove(int argc, char **argv){
  char *name;
  int i, rv;
  name = NULL;
  for(i = 2; i < argc; i++){
    if(!name && argv[i][0] == '-'){
      usage();
    } else if(!name){
      name = argv[i];
    } else {
      usage();
    }
  }
  if(!name) usage();
  rv = doremove(name);
  return rv;
}


/* parse arguments of repair command */
int runrepair(int argc, char **argv){
  char *name;
  int i, rv;
  name = NULL;
  for(i = 2; i < argc; i++){
    if(!name && argv[i][0] == '-'){
      usage();
    } else if(!name){
      name = argv[i];
    } else {
      usage();
    }
  }
  if(!name) usage();
  rv = dorepair(name);
  return rv;
}


/* parse arguments of exportdb command */
int runexportdb(int argc, char **argv){
  char *name, *dir;
  int i, rv;
  name = NULL;
  dir = NULL;
  for(i = 2; i < argc; i++){
    if(!name && argv[i][0] == '-'){
      usage();
    } else if(!name){
      name = argv[i];
    } else if(!dir){
      dir = argv[i];
    } else {
      usage();
    }
  }
  if(!name || !dir) usage();
  rv = doexportdb(name, dir);
  return rv;
}


/* parse arguments of importdb command */
int runimportdb(int argc, char **argv){
  char *name, *dir;
  int i, bnum, dnum, rv;
  name = NULL;
  dir = NULL;
  bnum = -1;
  dnum = -1;
  for(i = 2; i < argc; i++){
    if(!name && argv[i][0] == '-'){
      if(!strcmp(argv[i], "-bnum")){
        if(++i >= argc) usage();
        bnum = atoi(argv[i]);
      } else if(!strcmp(argv[i], "-dnum")){
        if(++i >= argc) usage();
        dnum = atoi(argv[i]);
      } else {
        usage();
      }
    } else if(!name){
      name = argv[i];
    } else if(!dir){
      dir = argv[i];
    } else {
      usage();
    }
  }
  if(!name || !dir) usage();
  rv = doimportdb(name, dir, bnum, dnum);
  return rv;
}


/* parse arguments of snaffle command */
int runsnaffle(int argc, char **argv){
  char *name, *key, *kbuf;
  int i, kx, ki, ox, nb, start, max, ksiz, rv;
  name = NULL;
  kx = FALSE;
  ki = FALSE;
  ox = FALSE;
  nb = FALSE;
  start = 0;
  max = -1;
  key = NULL;
  for(i = 2; i < argc; i++){
    if(!name && argv[i][0] == '-'){
      if(!strcmp(argv[i], "-kx")){
        kx = TRUE;
      } else if(!strcmp(argv[i], "-ki")){
        ki = TRUE;
      } else if(!strcmp(argv[i], "-ox")){
        ox = TRUE;
      } else if(!strcmp(argv[i], "-n")){
        nb = TRUE;
      } else {
        usage();
      }
    } else if(!name){
      name = argv[i];
    } else if(!key){
      key = argv[i];
    } else {
      usage();
    }
  }
  if(!name || !key || start < 0) usage();
  if(kx){
    kbuf = hextoobj(key, &ksiz);
  } else if(ki){
    kbuf = dectoiobj(key, &ksiz);
  } else {
    kbuf = cbmemdup(key, -1);
    ksiz = -1;
  }
  if(kbuf){
    rv = dosnaffle(name, kbuf, ksiz, ox, nb);
  } else {
    fprintf(stderr, "%s: out of memory\n", progname);
    rv = 1;
  }
  free(kbuf);
  return rv;
}


/* print an error message */
void pdperror(const char *name){
  fprintf(stderr, "%s: %s: %s\n", progname, name, dperrmsg(dpecode));
}


/* print an object */
void printobj(const char *obj, int size){
  int i;
  for(i = 0; i < size; i++){
    putchar(obj[i]);
  }
}


/* print an object as a hexadecimal string */
void printobjhex(const char *obj, int size){
  int i;
  for(i = 0; i < size; i++){
    printf("%s%02X", i > 0 ? " " : "", ((const unsigned char *)obj)[i]);
  }
}


/* perform create command */
int docreate(const char *name, int bnum, int dnum, int sparse){
  CURIA *curia;
  int omode;
  omode = CR_OWRITER | CR_OCREAT | CR_OTRUNC | (sparse ? CR_OSPARSE : 0);
  if(!(curia = cropen(name, omode, bnum, dnum))){
    pdperror(name);
    return 1;
  }
  if(!crclose(curia)){
    pdperror(name);
    return 1;
  }
  return 0;
}


/* perform put command */
int doput(const char *name, const char *kbuf, int ksiz,
          const char *vbuf, int vsiz, int dmode, int lob, int align){
  CURIA *curia;
  if(!(curia = cropen(name, CR_OWRITER, -1, -1))){
    pdperror(name);
    return 1;
  }
  if(align > 0 && !crsetalign(curia, ALIGNSIZ)){
    pdperror(name);
    crclose(curia);
    return 1;
  }
  if(lob){
    if(!crputlob(curia, kbuf, ksiz, vbuf, vsiz, dmode)){
      pdperror(name);
      crclose(curia);
      return 1;
    }
  } else {
    if(!crput(curia, kbuf, ksiz, vbuf, vsiz, dmode)){
      pdperror(name);
      crclose(curia);
      return 1;
    }
  }
  if(!crclose(curia)){
    pdperror(name);
    return 1;
  }
  return 0;
}


/* perform out command */
int doout(const char *name, const char *kbuf, int ksiz, int lob){
  CURIA *curia;
  if(!(curia = cropen(name, CR_OWRITER, -1, -1))){
    pdperror(name);
    return 1;
  }
  if(lob){
    if(!croutlob(curia, kbuf, ksiz)){
      pdperror(name);
      crclose(curia);
      return 1;
    }
  } else {
    if(!crout(curia, kbuf, ksiz)){
      pdperror(name);
      crclose(curia);
      return 1;
    }
  }
  if(!crclose(curia)){
    pdperror(name);
    return 1;
  }
  return 0;
}


/* perform get command */
int doget(const char *name, int opts, const char *kbuf, int ksiz,
          int start, int max, int ox, int lob, int nb){
  CURIA *curia;
  char *vbuf;
  int vsiz;
  if(!(curia = cropen(name, CR_OREADER | opts, -1, -1))){
    pdperror(name);
    return 1;
  }
  if(lob){
    if(!(vbuf = crgetlob(curia, kbuf, ksiz, start, max, &vsiz))){
      pdperror(name);
      crclose(curia);
      return 1;
    }
  } else {
    if(!(vbuf = crget(curia, kbuf, ksiz, start, max, &vsiz))){
      pdperror(name);
      crclose(curia);
      return 1;
    }
  }
  if(ox){
    printobjhex(vbuf, vsiz);
  } else {
    printobj(vbuf, vsiz);
  }
  free(vbuf);
  if(!nb) putchar('\n');
  if(!crclose(curia)){
    pdperror(name);
    return 1;
  }
  return 0;
}


/* perform list command */
int dolist(const char *name, int opts, int kb, int vb, int ox){
  CURIA *curia;
  char *kbuf, *vbuf;
  int ksiz, vsiz;
  if(!(curia = cropen(name, CR_OREADER | opts, -1, -1))){
    pdperror(name);
    return 1;
  }
  criterinit(curia);
  while((kbuf = criternext(curia, &ksiz)) != NULL){
    if(!(vbuf = crget(curia, kbuf, ksiz, 0, -1, &vsiz))){
      pdperror(name);
      free(kbuf);
      break;
    }
    if(ox){
      if(!vb) printobjhex(kbuf, ksiz);
      if(!kb && !vb) putchar('\t');
      if(!kb) printobjhex(vbuf, vsiz);
    } else {
      if(!vb) printobj(kbuf, ksiz);
      if(!kb && !vb) putchar('\t');
      if(!kb) printobj(vbuf, vsiz);
    }
    putchar('\n');
    free(vbuf);
    free(kbuf);
  }
  if(!crclose(curia)){
    pdperror(name);
    return 1;
  }
  return 0;
}


/* perform optimize command */
int dooptimize(const char *name, int bnum, int align){
  CURIA *curia;
  if(!(curia = cropen(name, CR_OWRITER, -1, -1))){
    pdperror(name);
    return 1;
  }
  if(align > 0 && !crsetalign(curia, ALIGNSIZ)){
    pdperror(name);
    crclose(curia);
    return 1;
  }
  if(!croptimize(curia, bnum)){
    pdperror(name);
    crclose(curia);
    return 1;
  }
  if(!crclose(curia)){
    pdperror(name);
    return 1;
  }
  return 0;
}


/* perform inform command */
int doinform(const char *name, int opts){
  CURIA *curia;
  if(!(curia = cropen(name, CR_OREADER | opts, -1, -1))){
    pdperror(name);
    return 1;
  }
  printf("name: %s\n", crname(curia));
  printf("file size: %.0f\n", crfsizd(curia));
  printf("all buckets: %d\n", crbnum(curia));
  printf("used buckets: %d\n", crbusenum(curia));
  printf("records: %d\n", crrnum(curia));
  printf("inode number: %d\n", crinode(curia));
  printf("modified time: %.0f\n", (double)crmtime(curia));
  printf("LOB records: %d\n", crrnumlob(curia));
  if(!crclose(curia)){
    pdperror(name);
    return 1;
  }
  return 0;
}


/* perform remove command */
int doremove(const char *name){
  if(!crremove(name)){
    pdperror(name);
    return 1;
  }
  return 0;
}


/* perform repair command */
int dorepair(const char *name){
  if(!crrepair(name)){
    pdperror(name);
    return 1;
  }
  return 0;
}


/* perform exportdb command */
int doexportdb(const char *name, const char *dir){
  CURIA *curia;
  if(!(curia = cropen(name, CR_OREADER, -1, -1))){
    pdperror(name);
    return 1;
  }
  if(!crexportdb(curia, dir)){
    pdperror(name);
    crclose(curia);
    return 1;
  }
  if(!crclose(curia)){
    pdperror(name);
    return 1;
  }
  return 0;
}


/* perform importdb command */
int doimportdb(const char *name, const char *dir, int bnum, int dnum){
  CURIA *curia;
  if(!(curia = cropen(name, CR_OWRITER | CR_OCREAT | CR_OTRUNC, bnum, dnum))){
    pdperror(name);
    return 1;
  }
  if(!crimportdb(curia, dir)){
    pdperror(name);
    crclose(curia);
    return 1;
  }
  if(!crclose(curia)){
    pdperror(name);
    return 1;
  }
  return 0;
}


/* perform snaffle command */
int dosnaffle(const char *name, const char *kbuf, int ksiz, int ox, int nb){
  char *vbuf;
  int vsiz;
  if(!(vbuf = crsnaffle(name, kbuf, ksiz, &vsiz))){
    pdperror(name);
    return 1;
  }
  if(ox){
    printobjhex(vbuf, vsiz);
  } else {
    printobj(vbuf, vsiz);
  }
  free(vbuf);
  if(!nb) putchar('\n');
  return 0;
}



/* END OF FILE */
