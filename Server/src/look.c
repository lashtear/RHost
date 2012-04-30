
/* look.c -- commands which look at things */

#ifdef SOLARIS
/* Once again, broken declarations */
char *index(const char *, int);
#endif

#include "copyright.h"
#include "autoconf.h"

#include "mudconf.h"
#include "config.h"
#include "db.h"
#include "interface.h"
#include "match.h"
#include "externs.h"
#include "flags.h"
#include "command.h"
#include "alloc.h"
#include "rhost_ansi.h"
#ifdef REALITY_LEVELS
#include "levels.h"
#endif /* REALITY_LEVELS */

static void 
look_exits(dbref player, dbref loc, const char *exit_name, int keytype)
{
    dbref thing, parent, pcheck, dest, aowner;
    char *buff, *e, *s, *buf2, *buf3, *tbuff, *tbuffptr, *tbuffatr, *tpr_buff, *tprp_buff;
    int foundany, lev, key, light_dark, do_ret, aflags, oldparent;

    /* make sure location has exits */

    if (!Good_obj(loc) || !Has_exits(loc))
	return;

    /* make sure there is at least one visible exit */

    do_ret = 0;
    if (!keytype && mudconf.fmt_exits && !NoFormat(player)) {
       buf3 = atr_pget(loc, A_LEXIT_FMT, &aowner, &aflags);
       if (*buf3) {
            did_it(player, loc, A_LEXIT_FMT, NULL, 0, NULL,
                   0, (char **) NULL, 0);
            free_lbuf(buf3);
            do_ret=1;
       } else if (buf3) {
            free_lbuf(buf3);
       }
       buf3 = atr_pget(loc, A_LDEXIT_FMT, &aowner, &aflags);
       if (*buf3) {
            did_it(player, loc, A_LDEXIT_FMT, NULL, 0, NULL,
                   0, (char **) NULL, 0);
            free_lbuf(buf3);
            do_ret=1;
       } else if (buf3) {
            free_lbuf(buf3);
       }
    }
    if ( do_ret )
       return;

    /* Added dark exit viewing for wizards 3/95 - Thorin */

    pcheck = loc;
    for (light_dark = 0;
	 light_dark == 0 || (Wizard(player) && (light_dark < 2) && !Myopic(player));
	 light_dark++) {
	foundany = 0;
	if (light_dark == 1)
	  key = VE_ACK;
	else
	 key = 0;
#ifdef REALITY_LEVELS
        if ((Dark(loc) && !IsReal(player, loc))
                && !could_doit( player, loc, A_LDARK, 0 ))
#else
        if (Dark(loc) && !could_doit( player, loc, A_LDARK, 0 ))
#endif /* REALITY_LEVELS */
	    key |= VE_BASE_DARK;
	ITER_PARENTS(loc, parent, lev) {
	    key &= ~VE_LOC_DARK;
	    if (Dark(parent) && !could_doit( player, parent, A_LDARK, 0 ))
		key |= VE_LOC_DARK;
	    DOLIST(thing, Exits(parent)) {
                if ( ((parent != pcheck) && (Flags3(thing) & PRIVATE)) || 
                     (keytype && (Floating(thing) || (Flags3(thing) & PRIVATE))) ) {
		    continue;
		}
#ifdef REALITY_LEVELS
                if (((light_dark == 0 &&
                     exit_displayable(thing, player, key)) ||
                    (light_dark == 1 &&
                     !exit_displayable(thing, player, key))) &&
                    IsReal(player, thing)) {
#else
                if ((light_dark == 0 &&
                     exit_displayable(thing, player, key)) ||
                    (light_dark == 1 &&
                     !exit_displayable(thing, player, key))) {
#endif /* REALITY_LEVELS */ 
		    foundany = 1;
		    break;
		}
	    }
	    if (foundany)
		break;
	}

	if (!foundany)
	    continue;

	/* Display the list of exit names */

	if (light_dark == 0) {
            tprp_buff = tpr_buff = alloc_lbuf("look_exits");
	    notify(player, safe_tprintf(tpr_buff, &tprp_buff, "%s%s%s",
                   ANSIEX(ANSI_HILITE), exit_name, ANSIEX(ANSI_NORMAL)));
            free_lbuf(tpr_buff);
	} else {
            tprp_buff = tpr_buff = alloc_lbuf("look_exits");
            if ( keytype )
	       notify(player, safe_tprintf(tpr_buff, &tprp_buff, "%sDark Global Exits:%s", 
                      ANSIEX(ANSI_HILITE), ANSIEX(ANSI_NORMAL)));
            else
	       notify(player, safe_tprintf(tpr_buff, &tprp_buff, "%sDark Exits:%s", 
                      ANSIEX(ANSI_HILITE), ANSIEX(ANSI_NORMAL)));
            free_lbuf(tpr_buff);
        }
	e = buff = alloc_lbuf("look_exits");
	oldparent = -1;
	ITER_PARENTS(loc, parent, lev) {
            /* prevents exits being displayed multiple times if room
	     * parented to #0.
	     * Hack solution - Ashen Shugar
	     */
            if ( parent == oldparent) {
	      continue;
	    }
            key &= ~VE_LOC_DARK;
	    if (Dark(parent) && !could_doit( player, parent, A_LDARK, 0 ))
		key |= VE_LOC_DARK;
	    DOLIST(thing, Exits(parent)) {
                if ( ((parent != pcheck) && (Flags3(thing) & PRIVATE)) || 
                     (keytype && (Floating(thing) || (Flags3(thing) & PRIVATE))) )
		    continue;
#ifdef REALITY_LEVELS
                if (((light_dark == 0 &&
                     exit_displayable(thing, player, key)) ||
                    (light_dark == 1 &&
                     !exit_displayable(thing, player, key))) &&
                    IsReal(player, thing)) {
#else
                if ((light_dark == 0 &&
                     exit_displayable(thing, player, key)) ||
                    (light_dark == 1 &&
                     !exit_displayable(thing, player, key))) {
#endif /* REALITY_LEVELS */
		    /* chop off first exit alias to display */

		    if (buff != e) {
		      if (Transparent(loc))
			safe_str("\r\n", buff, &e);
		      else
			safe_str((char *) "  ", buff, &e);
		    }

                    if ( ExtAnsi(thing) ) {
                       tbuffptr = tbuff = alloc_lbuf("look_exits_ansi");
		       for (s = Name(thing); *s && (*s != ';'); s++)
			   safe_chr(*s, tbuff, &tbuffptr);
                       tbuffatr = atr_get(thing, A_ANSINAME, &aowner, &aflags);
                       if ( isExit(thing) && !NoAnsiExit(player) && (mudconf.allow_ansinames & ANSI_EXIT) &&
                            !NoAnsiName(thing) && !NoAnsiName(Owner(thing)) ) {
                          if ( strcmp(strip_all_special(tbuffatr), tbuff) == 0 ) {
                             if ( !Accents(player) ) {
                                safe_str(strip_safe_accents(tbuffatr), buff, &e);
                             } else {
                                safe_str(tbuffatr, buff, &e);
                             }
                             safe_str(ANSI_NORMAL, buff, &e);
                          } else {
                             safe_str(tbuff, buff, &e);
                          }
                       } else {
                          safe_str(tbuff, buff, &e);
                       }
                       free_lbuf(tbuff);
                       free_lbuf(tbuffatr);
                    } else {
                       buf2 = ansi_exitname(thing);
                       if ( isExit(thing) && !NoAnsiExit(player) && (mudconf.allow_ansinames & ANSI_EXIT) &&
                            !NoAnsiName(thing) && !NoAnsiName(Owner(thing)) ) {
                          if ( !Accents(player) )
                             safe_str(strip_safe_accents(buf2), buff, &e);
                          else
                             safe_str(buf2, buff, &e);
                       }
                       free_lbuf(buf2);
		       for (s = Name(thing); *s && (*s != ';'); s++)
			   safe_chr(*s, buff, &e);
                       if ( isExit(thing) && !NoAnsiExit(player) && (mudconf.allow_ansinames & ANSI_EXIT) &&
                            !NoAnsiName(thing) && !NoAnsiName(Owner(thing)) )
                          safe_str(ANSI_NORMAL, buff, &e);
                    }
		    dest = Location(thing);
		    if (Transparent(loc) && Good_obj(dest) && !Going(dest) && !Recover(dest)) {
		      safe_str(" [leads to '", buff, &e);
                      if ( VariableExit(thing) ) {
                         safe_str("*VARIABLE LOCATION*", buff, &e);
                      } else {
		         safe_str(Name(dest), buff, &e);
                      }
		      safe_str("']", buff, &e);
		    }
		    else if (Transparent(loc) && (dest == HOME)) {
                      if ( VariableExit(thing) ) {
                         safe_str(" [leads to '*VARIABLE LOCATION*']", buff, &e);
                      } else {
		         safe_str(" [leads to '*HOME*']", buff, &e);
                      }
		    } else if (Transparent(loc) && !Good_chk(dest) ) {
                       safe_str(" [leads to '*NOWHERE*']", buff, &e);
                    }
		}
	    }
	}
	*e = 0;
	notify(player, buff);
	free_lbuf(buff);
    }
}

static void 
look_altinv(dbref player, dbref loc, const char *contents_name)
{
    dbref thing;
    dbref can_see_loc;
    dbref aowner;
    char *buff, *pbuf, *tbuff, *tpr_buff, *tprp_buff;
    int aflags, i_cntr=0;

    /* check to see if he can see the location */

    can_see_loc = (!Dark(loc) || (Dark(loc) && could_doit(player, loc, A_LDARK, 0)) || 
		   (mudconf.see_own_dark && MyopicExam(player, loc)));
/*  can_see_loc = (!Dark(loc) || 
		   (mudconf.see_own_dark && MyopicExam(player, loc))); */

#ifdef REALITY_LEVELS
    can_see_loc = can_see_loc && IsReal(player, loc);
#endif /* REALITY_LEVELS */

    /* check to see if there is anything there */

    tprp_buff = tpr_buff = alloc_lbuf("look_altinv");
    DOLIST(thing, Contents(loc)) {
#ifdef REALITY_LEVELS
        if (((can_see(player, thing, can_see_loc) && mudconf.player_dark) ||
                (can_see2(player, thing, can_see_loc) && !mudconf.player_dark)) &&
                IsReal(player, thing)) {
#else
        if ((can_see(player, thing, can_see_loc) && mudconf.player_dark) ||
                (can_see2(player, thing, can_see_loc) && !mudconf.player_dark)) {
#endif /* REALITY_LEVELS */
	    /* something exists!  show him everything */

	    DOLIST(thing, Contents(loc)) {
               if ( !Wearable(thing) && !Wieldable(thing) )
                  continue;
#ifdef REALITY_LEVELS
               if (((can_see(player, thing, can_see_loc) && mudconf.player_dark) ||
                   (can_see2(player, thing, can_see_loc) && !mudconf.player_dark)) &&
                   IsReal(player, thing)) {
#else
               if ((can_see(player, thing, can_see_loc) && mudconf.player_dark) ||
                   (can_see2(player, thing, can_see_loc) && !mudconf.player_dark)) {
#endif /* REALITY_LEVELS */
                  if ( NoAnsiName(thing) || NoAnsiName(Owner(thing)) ||
                      ((NoAnsiPlayer(player) || !(mudconf.allow_ansinames & ANSI_PLAYER)) && isPlayer(thing)) ||
                      ((NoAnsiThing(player) || !(mudconf.allow_ansinames & ANSI_THING)) && isThing(thing)) ||
                      ((NoAnsiExit(player) || !(mudconf.allow_ansinames & ANSI_EXIT)) && isExit(thing)) ||
                      ((NoAnsiRoom(player) || !(mudconf.allow_ansinames & ANSI_ROOM)) && isRoom(thing)) )
  		       buff = unparse_object_altname(player, thing, 1);
                    else
                       buff = unparse_object_ansi_altname(player, thing, 1);

                    if ( !i_cntr ) {
                       tprp_buff = tpr_buff;
	               notify(player, safe_tprintf(tpr_buff, &tprp_buff, "%s%s%s", 
                              ANSIEX(ANSI_HILITE), contents_name, ANSIEX(ANSI_NORMAL)));
                       i_cntr++;
                    }
                    tbuff = atr_pget(thing, A_INVTYPE, &aowner, &aflags);
                    if ( ((NoName(thing) && *buff) || !NoName(thing)) ) {
                       tprp_buff = tpr_buff;
                       if(isPlayer(thing)) {
                           pbuf = atr_get(thing, A_CAPTION, &aowner, &aflags);
                           if(*pbuf)
                              if ( !*tbuff )
		                 notify(player, safe_tprintf(tpr_buff, &tprp_buff, "Miscellaneous: %s, %.40s", buff, pbuf));
                              else
		                 notify(player, safe_tprintf(tpr_buff, &tprp_buff, "%.30s: %s, %s", tbuff, buff, pbuf));
                           else
                              if ( !*tbuff )
		                 notify(player, safe_tprintf(tpr_buff, &tprp_buff, "Miscellaneous: %s", buff));
                              else
		                 notify(player, safe_tprintf(tpr_buff, &tprp_buff, "%.30s: %s", tbuff, buff));
                           free_lbuf(pbuf);
                       } else {
                           if ( !*tbuff )
		              notify(player, safe_tprintf(tpr_buff, &tprp_buff, "Miscellaneous: %s", buff));
                           else
		              notify(player, safe_tprintf(tpr_buff, &tprp_buff, "%.30s: %s", tbuff, buff));
                       }
                    }

		    free_lbuf(buff);
                    free_lbuf(tbuff);
		}
	    }
	    break;		/* we're done */
	}
    }
    free_lbuf(tpr_buff);
}

static void 
look_contents_altinv(dbref player, dbref loc, const char *contents_name)
{
    dbref thing;
    dbref can_see_loc;
    dbref aowner;
    char *buff, *pbuf, *buf2, *tpr_buff, *tprp_buff;
    int aflags, i_cont=0;

    /* check to see if he can see the location */

    if (mudconf.fmt_contents && !NoFormat(player)) {
        buf2 = atr_pget(loc, A_LCON_FMT, &aowner, &aflags);
        if (*buf2) {
            did_it(player, loc, A_LCON_FMT, NULL, 0, NULL,
                   0, (char **) NULL, 0);
            free_lbuf(buf2);
            return;
        } else if (buf2) {
            free_lbuf(buf2);
        }
    }

    can_see_loc = (!Dark(loc) || (Dark(loc) && could_doit(player, loc, A_LDARK, 0)) ||
		   (mudconf.see_own_dark && MyopicExam(player, loc))); 
/*  can_see_loc = (!Dark(loc) || 
		   (mudconf.see_own_dark && MyopicExam(player, loc))); */

#ifdef REALITY_LEVELS
    can_see_loc = can_see_loc && IsReal(player, loc);
#endif /* REALITY_LEVELS */

    /* check to see if there is anything there */

    tprp_buff = tpr_buff = alloc_lbuf("look_contents_altinv");
    DOLIST(thing, Contents(loc)) {
#ifdef REALITY_LEVELS
        if (((can_see(player, thing, can_see_loc) && mudconf.player_dark) ||
                (can_see2(player, thing, can_see_loc) && !mudconf.player_dark)) &&
                IsReal(player, thing)) {
#else
        if ((can_see(player, thing, can_see_loc) && mudconf.player_dark) ||
                (can_see2(player, thing, can_see_loc) && !mudconf.player_dark)) {
#endif /* REALITY_LEVELS */

	    /* something exists!  show him everything */

	    DOLIST(thing, Contents(loc)) {
               if ( (Wearable(thing) || Wieldable(thing)) && mudconf.alt_inventories && 
                    mudconf.altover_inv )
                  continue;
	       if ((can_see(player, thing, can_see_loc) && mudconf.player_dark) ||
		   (can_see2(player, thing, can_see_loc) && !mudconf.player_dark)) {
                  if ( NoAnsiName(thing) || NoAnsiName(Owner(thing)) ||
                      ((NoAnsiPlayer(player) || !(mudconf.allow_ansinames & ANSI_PLAYER)) && isPlayer(thing)) ||
                      ((NoAnsiThing(player) || !(mudconf.allow_ansinames & ANSI_THING)) && isThing(thing)) ||
                      ((NoAnsiExit(player) || !(mudconf.allow_ansinames & ANSI_EXIT)) && isExit(thing)) ||
                      ((NoAnsiRoom(player) || !(mudconf.allow_ansinames & ANSI_ROOM)) && isRoom(thing)) )
  		       buff = unparse_object_altname(player, thing, 1);
                    else
                       buff = unparse_object_ansi_altname(player, thing, 1);

                    if ( !i_cont ) {
                       tprp_buff = tpr_buff;
	               notify(player, safe_tprintf(tpr_buff, &tprp_buff, "%s%s%s", 
                              ANSIEX(ANSI_HILITE), contents_name, ANSIEX(ANSI_NORMAL)));
                       i_cont++;
                    }
                    if ( ((NoName(thing) && *buff) || !NoName(thing)) ) {
                       if(isPlayer(thing)) {
                           pbuf = atr_get(thing, A_CAPTION, &aowner, &aflags);
                           if(*pbuf) {
                              tprp_buff = tpr_buff;
                              notify(player, safe_tprintf(tpr_buff, &tprp_buff, "%s, %.40s", buff, pbuf));
                           } else
                              notify(player, buff);
                           free_lbuf(pbuf);
                       } else
                           notify(player, buff);
                    }

		    free_lbuf(buff);
		}
	    }
	    break;		/* we're done */
	}
    }
    free_lbuf(tpr_buff);
}

static void 
look_contents(dbref player, dbref loc, const char *contents_name)
{
    dbref thing;
    dbref can_see_loc;
    dbref aowner;
    char *buff, *pbuf, *buf2, *tpr_buff, *tprp_buff;
    int aflags;

    /* check to see if he can see the location */

    if (mudconf.fmt_contents && !NoFormat(player)) {
        buf2 = atr_pget(loc, A_LCON_FMT, &aowner, &aflags);
        if (*buf2) {
            did_it(player, loc, A_LCON_FMT, NULL, 0, NULL,
                   0, (char **) NULL, 0);
            free_lbuf(buf2);
            return;
        } else if (buf2) {
            free_lbuf(buf2);
        }
    }

    can_see_loc = (!Dark(loc) || (Dark(loc) && could_doit(player, loc, A_LDARK, 0)) ||
		   (mudconf.see_own_dark && MyopicExam(player, loc))); 
/*  can_see_loc = (!Dark(loc) || 
		   (mudconf.see_own_dark && MyopicExam(player, loc))); */

#ifdef REALITY_LEVELS
    can_see_loc = can_see_loc && IsReal(player, loc);
#endif /* REALITY_LEVELS */
    /* check to see if there is anything there */

    tprp_buff = tpr_buff = alloc_lbuf("look_contents");
    DOLIST(thing, Contents(loc)) {
#ifdef REALITY_LEVELS
        if (((can_see(player, thing, can_see_loc) && mudconf.player_dark) ||
                (can_see2(player, thing, can_see_loc) && !mudconf.player_dark)) &&
                IsReal(player, thing)) {
#else
        if ((can_see(player, thing, can_see_loc) && mudconf.player_dark) ||
                (can_see2(player, thing, can_see_loc) && !mudconf.player_dark)) {
#endif /* REALITY_LEVELS */ 

	    /* something exists!  show him everything */

            tprp_buff = tpr_buff;
	    notify(player, safe_tprintf(tpr_buff, &tprp_buff, "%s%s%s", 
                   ANSIEX(ANSI_HILITE), contents_name, ANSIEX(ANSI_NORMAL)));
	    DOLIST(thing, Contents(loc)) {
#ifdef REALITY_LEVELS
               if (((can_see(player, thing, can_see_loc) && mudconf.player_dark) ||
                   (can_see2(player, thing, can_see_loc) && !mudconf.player_dark)) &&
                   IsReal(player, thing)) {
#else
               if ((can_see(player, thing, can_see_loc) && mudconf.player_dark) ||
                   (can_see2(player, thing, can_see_loc) && !mudconf.player_dark)) {
#endif /* REALITY_LEVELS */
                  if ( NoAnsiName(thing) || NoAnsiName(Owner(thing)) ||
                      ((NoAnsiPlayer(player) || !(mudconf.allow_ansinames & ANSI_PLAYER)) && isPlayer(thing)) ||
                      ((NoAnsiThing(player) || !(mudconf.allow_ansinames & ANSI_THING)) && isThing(thing)) ||
                      ((NoAnsiExit(player) || !(mudconf.allow_ansinames & ANSI_EXIT)) && isExit(thing)) ||
                      ((NoAnsiRoom(player) || !(mudconf.allow_ansinames & ANSI_ROOM)) && isRoom(thing)) )
  		       buff = unparse_object_altname(player, thing, 1);
                    else
                       buff = unparse_object_ansi_altname(player, thing, 1);

                    if ( ((NoName(thing) && *buff) || !NoName(thing)) ) {
                       if(isPlayer(thing)) {
                           pbuf = atr_get(thing, A_CAPTION, &aowner, &aflags);
                           if(*pbuf) {
                              tprp_buff = tpr_buff;
                              notify(player, safe_tprintf(tpr_buff, &tprp_buff, "%s, %.40s", buff, pbuf));
                           } else
                              notify(player, buff);
                           free_lbuf(pbuf);
                       } else
                           notify(player, buff);
                    }

		    free_lbuf(buff);
		}
	    }
	    break;		/* we're done */
	}
    }
    free_lbuf(tpr_buff);
}

void
prtflags(dbref player, int aflags, char *xbuf, char *xbufp)
{
    xbufp = xbuf;
    if (aflags & AF_LOCK)
	*xbufp++ = '+';
    if (aflags & AF_NOPROG)
	*xbufp++ = '$';
    if (aflags & AF_PRIVATE)
	*xbufp++ = 'I';
    if (aflags & AF_VISUAL)
	*xbufp++ = 'V';
    if (aflags & AF_MDARK)
        *xbufp++ = 'M';
    if (aflags & AF_GOD)
	*xbufp++ = 'G';
    if (aflags & AF_IMMORTAL)
	*xbufp++ = 'i';
    if (aflags & AF_WIZARD)
	*xbufp++ = 'W';
    if (aflags & AF_ADMIN)
	*xbufp++ = 'a';
    if (aflags & AF_BUILDER)
	*xbufp++ = 'B';
    if (aflags & AF_GUILDMASTER)
	*xbufp++ = 'g';
    if ((aflags & AF_PINVIS) && Wizard(player))
	*xbufp++ = 'p';
    if (aflags & AF_NOCLONE)
	*xbufp++ = 'N';
    if (aflags & AF_DARK)
        *xbufp++ = 'D';
    if (aflags & AF_NOPARSE)
        *xbufp++ = 'n';
    if (aflags & AF_SAFE)
        *xbufp++ = 's';
    if (aflags & AF_USELOCK)
        *xbufp++ = 'u';
    if (aflags & AF_SINGLETHREAD)
        *xbufp++ = 'S';
    if (aflags & AF_DEFAULT)
        *xbufp++ = 'F';
    if ((aflags & AF_ATRLOCK) && Wizard(player))
        *xbufp++ = 'l';
    if ((aflags & AF_LOGGED) && Wizard(player))
        *xbufp++ = 'm';
    if (aflags & AF_REGEXP) 
        *xbufp++ = 'R';
    *xbufp = '\0';
}

static void 
view_atr(dbref player, dbref thing, ATTR * ap, char *text,
	 dbref aowner, int aflags, int skip_tag, int pobj)
{
    char *buf = NULL, *buf2 = NULL, *tpr_buff, *tprp_buff;
    char xbuf[32], ybuf[32], zbuf[32];
    char *xbufp = NULL, 
         *ybufp = NULL;
    BOOLEXP *bool;

    if (!Wizard(player) && ((ap->flags & AF_PINVIS) || (aflags & AF_PINVIS)))
	return;

    if (ap->flags & AF_IS_LOCK) {
	bool = parse_boolexp(player, text, 1);
	text = unparse_boolexp(player, bool);
	free_boolexp(bool);
    }
    /* If we don't control the object or own the attribute, hide the
     * attr owner and flag info.
     */
    memset(zbuf, '\0', sizeof(zbuf));
    if ( pobj != -1 ) {
       sprintf(zbuf, "#%d/", pobj);
    }
    if (!Controlsforattr(player, thing, ap, aflags) && (Owner(player) != aowner)) {
	if (skip_tag && (ap->number == A_DESC)) {
	    buf = text;
	    noansi_notify(player, buf);
	} else {
            tpr_buff = alloc_lbuf("view_atr");
            memset(tpr_buff, '\0', LBUF_SIZE);
            tprp_buff = tpr_buff;
            buf2 = safe_tprintf(tpr_buff, &tprp_buff, "%s%s%s:%s ",
                          ANSIEX(ANSI_HILITE), zbuf, ap->name,
                          ANSIEX(ANSI_NORMAL));
            buf = text;
            raw_notify(player, buf2, 0, 0);
	    noansi_notify(player, buf);
            free_lbuf(tpr_buff);
        }
	return;
    }
    /* Generate flags */

    memset(xbuf, 0, sizeof(xbuf));
    prtflags(player, aflags, xbuf, xbufp);
    if ( mudconf.look_moreflags ) {
       memset(ybuf, 0, sizeof(ybuf));
       prtflags(player, ap->flags, ybuf, ybufp);
    }

    tprp_buff = tpr_buff = alloc_lbuf("view_atr");
    if ((aowner != Owner(thing)) && (aowner != NOTHING)) {
        if ( mudconf.look_moreflags && *ybuf) {
	   buf2 = safe_tprintf(tpr_buff, &tprp_buff, "%s%s%s(#%d%s[%s]):%s ", 
                         ANSIEX(ANSI_HILITE), zbuf, ap->name, aowner, xbuf, ybuf,
                         ANSIEX(ANSI_NORMAL));
        } else {
	   buf2 = safe_tprintf(tpr_buff, &tprp_buff, "%s%s%s(#%d%s):%s ", 
                         ANSIEX(ANSI_HILITE), zbuf, ap->name, aowner, xbuf, 
                         ANSIEX(ANSI_NORMAL));
        }
    } else if (*xbuf) {
        if ( mudconf.look_moreflags && *ybuf) {
	   buf2 = safe_tprintf(tpr_buff, &tprp_buff, "%s%s%s(%s[%s]):%s ", 
                         ANSIEX(ANSI_HILITE), zbuf, ap->name, xbuf, ybuf,
                         ANSIEX(ANSI_NORMAL));
        } else {
	   buf2 = safe_tprintf(tpr_buff, &tprp_buff, "%s%s%s(%s):%s ", 
                         ANSIEX(ANSI_HILITE), zbuf, ap->name, xbuf, 
                         ANSIEX(ANSI_NORMAL));
        }
    } else if (!skip_tag || (ap->number != A_DESC)) {
        if ( mudconf.look_moreflags && *ybuf) {
	   buf2 = safe_tprintf(tpr_buff, &tprp_buff, "%s%s%s([%s]):%s ", 
                         ANSIEX(ANSI_HILITE), zbuf, ap->name, ybuf,
                         ANSIEX(ANSI_NORMAL));
        } else {
	   buf2 = safe_tprintf(tpr_buff, &tprp_buff, "%s%s%s:%s ", 
                         ANSIEX(ANSI_HILITE), zbuf, ap->name, 
                         ANSIEX(ANSI_NORMAL));
        }
    }
    buf = text;
    if( buf2 && *buf2 )
        raw_notify(player, buf2, 0, 0);
    noansi_notify(player, buf);
    free_lbuf(tpr_buff);
}

char *
grep_internal(dbref player, dbref thing, char *wcheck, char *watr, int i_key)
{
    time_t endtme;
    dbref aowner, othing;
    int ca, aflags, go2, go3, timechk;
    ATTR *attr;
    char *as, *buf, *bp, *retbuff, *go1, *buf2;
    char tbuf[80], tbuf2[80];

    retbuff = alloc_lbuf("grep_int");
    othing = thing;
    go1 = strpbrk(watr, "?\\*");
    if (strpbrk(wcheck, "?\\*")) {
	go3 = 0;
	buf2 = wcheck;
    } else {
	buf2 = alloc_lbuf("grep_int2");
	bp = buf2;
	safe_chr('*', buf2, &bp);
	safe_str(wcheck, buf2, &bp);
	safe_chr('*', buf2, &bp);
	*bp = '\0';
	go3 = 1;
    }
    bp = retbuff;
    timechk = mudconf.cputimechk;
    if ( mudconf.cputimechk < 10 )
       timechk = 10;
    if ( mudconf.cputimechk > 3600 )
       timechk = 3600;
    for (ca = atr_head(thing, &as); ca; ca = atr_next(&as)) {
	attr = atr_num(ca);
	if (!attr)
	    continue;
        endtme = time(NULL);
        if ( mudstate.chkcpu_toggle || ((endtme - mudstate.chkcpu_stopper) > timechk) ) {
           mudstate.chkcpu_toggle = 1;
           break;
        }
	strcpy(tbuf, attr->name);
	if (go1)
	    go2 = quick_wild(watr, tbuf);
	else
	    go2 = !stricmp(watr, tbuf);
	if (go2) {
	    buf = atr_get(thing, ca, &aowner, &aflags);
	    if ((Wizard(player) || (!(attr->flags & AF_PINVIS) && !(aflags & AF_PINVIS))) && 
		(Read_attr(player, othing, attr, aowner, aflags, 0))) {
		if (quick_wild(buf2, buf)) {
                    if ( i_key ) {
                       sprintf(tbuf2, "#%d/", othing);
                       safe_str(tbuf2, retbuff, &bp);
                    }
		    safe_str(tbuf, retbuff, &bp);
		    safe_chr(' ', retbuff, &bp);
		}
	    }
	    free_lbuf(buf);
	}
    }
    if (bp != retbuff)
	*(bp - 1) = '\0';
    else
	*bp = '\0';
    if (go3)
	free_lbuf(buf2);
    return retbuff;
}

void 
do_grep(dbref player, dbref cause, int key, char *source,
	char *wildch[], int nargs)
{
    dbref thing;
    char *pt1;

    if (nargs != 2) {
	notify(player, "Incorrect command format.");
	return;
    }
    init_match(player, source, NOTYPE);
    match_everything(MAT_EXIT_PARENTS);
    thing = noisy_match_result();
    if (thing == NOTHING)
	notify(player, "Bad object specified.");
    else if (!Examinable(player, thing))
	notify(player, "Bad object specified.");
    else {
        if ( PCRE_EXEC && (key & GREP_REGEXP) )
           pt1 = grep_internal_regexp(player, thing, wildch[1], wildch[0], 1);
        else
	   pt1 = grep_internal(player, thing, wildch[1], wildch[0], 0);
	notify(player, pt1);
	free_lbuf(pt1);
    }
    if ( !(key & GREP_QUIET) )
	notify(player, "Grep: Done.");
}

typedef struct atrstruct {
    int atr;
    char *info;
    char wt;
    char *oname;
    int flags;
    struct atrstruct *next;
} ATRST;

static ATRST *atrpt;
static ATRST *atrpt2;

void 
atrpush(int val, const char *data, int fl)
{
    ATRST *t1, **t2;

    if (val == -1)
	t2 = &atrpt2;
    else
	t2 = &atrpt;
    t1 = (ATRST *) malloc(sizeof(struct atrstruct));

    t1->info = (char *) malloc(strlen(data) + 1);
    strcpy(t1->info, data);
    t1->wt = 0;
    t1->flags = fl;
    t1->atr = val;
    t1->next = *t2;
    *t2 = t1;
}

void 
atrptclr()
{
    ATRST *t1;

    while (atrpt) {
	t1 = atrpt;
	atrpt = atrpt->next;
	free(t1->info);
	free(t1);
    }
    while (atrpt2) {
	t1 = atrpt2;
	atrpt2 = atrpt2->next;
	free(t1->info);
	free(t1);
    }
}

#define NAMECUT	16

void 
do_cpattr(dbref player, dbref cause, int key, char *source,
	  char *destlist[], int nargs)
{
    char *sep1, *as, *buf, *dest, *pt2, *sepsave, *bk_source, *buff2, *buff2ret;
    dbref thing1, thing2, aowner, aowner2;
    int wyes, aflags, ca, ca2, ca3, mt, t2, verbose, ex1, twk1, clr1, clr2, chkv1, aflags2, no_set;
    ATTR *attr;
    ATRST *pt1;
    char tbuf[80], *tpr_buff, *tprp_buff;

    wyes = ca = ca2 = ca3 = mt = t2 = clr1 = clr2 = chkv1 = 0;
    if (!nargs || ((nargs == 1) && !*destlist[0])) {
	notify(player, "No destination given.");
	return;
    }
    bk_source = alloc_lbuf("do_cpattr");
    strcpy(bk_source, source);
    sep1 = index(bk_source, '/');
    if (!sep1 || !*(sep1 + 1)) {
        sprintf(bk_source, "me/%.3900s", source);
        sep1 = index(bk_source, '/');
        if (!sep1 || !*(sep1 + 1)) {
 	   notify(player, "No source attribute(s) given.");
           free_lbuf(bk_source);
	   return;
        }
    }
    *sep1 = '\0';
    sepsave = sep1;
    if (strpbrk(sep1 + 1, "?\\*"))
	wyes = 1;
    else
	wyes = 0;
    init_match(player, bk_source, NOTYPE);
    match_everything(MAT_EXIT_PARENTS);
    thing1 = noisy_match_result();
    if (thing1 != NOTHING) {
      ex1 = Examinable(player, thing1);
      twk1 = could_doit(player,thing1,A_LTWINK,0);
    }
    if ((thing1 == NOTHING) || (!ex1 && !twk1)) {
	notify(player, "Bad source object.");
        free_lbuf(bk_source);
	return;
    }
    if ((key & CPATTR_VERB) != 0) {
	verbose = 1;
	key &= ~CPATTR_VERB;
    } else
	verbose = 0;
    atrpt = atrpt2 = NULL;
    for (ca = atr_head(thing1, &as); ca; ca = atr_next(&as)) {
	attr = atr_num(ca);
	if (!attr)
	    continue;
	strcpy(tbuf, attr->name);
	if (wyes)
	    mt = quick_wild(sep1 + 1, tbuf);
	else
	    mt = !stricmp(tbuf, sep1 + 1);
	if (mt) {
	    buf = atr_get(thing1, ca, &aowner, &aflags);
	    if (*buf && Read_attr(player, thing1, attr, aowner, aflags, 0)) {
	      if (ex1 || (!ex1 && twk1 && !(attr->flags & (AF_IS_LOCK | AF_LOCK)) && !(aflags & AF_LOCK)))
		atrpush(ca, buf, aflags);
	    }
	    free_lbuf(buf);
	}
    }
    if (!atrpt) {
	notify(player, "No valid source attributes given.");
        free_lbuf(bk_source);
	return;
    }
    tprp_buff = tpr_buff = alloc_lbuf("do_cpattr");
    for (mt = 0; mt < nargs; mt++) {
	dest = destlist[mt];
	sep1 = index(dest, '/');
	if (sep1) {
	    if (wyes) {
		notify(player, "Destination attribute ignored.");
		ca = -1;
	    } else
		ca = 0;
	    *sep1 = '\0';
	    if (!*(sep1 + 1))
		ca = -1;
	} else
	    ca = -1;
	init_match(player, dest, NOTYPE);
	match_everything(MAT_EXIT_PARENTS);
	thing2 = noisy_match_result();
        twk1 = could_doit(player,thing2,A_LTWINK,0);
	if ((thing2 == NOTHING) || (!Controls(player, thing2) && !twk1 && !chkv1)) {
            tprp_buff = tpr_buff;
	    notify(player, safe_tprintf(tpr_buff, &tprp_buff, "Bad destination object -> %s", dest));
	    continue;
	}
        if ((NoMod(thing2) && !WizMod(player)) ||
                 (DePriv(player,Owner(thing2),DP_MODIFY,POWER7,NOTHING))) {
            tprp_buff = tpr_buff;
	    notify(player, safe_tprintf(tpr_buff, &tprp_buff, "Bad destination object -> %s", dest));
	    continue;
        }
	if (!ca)
	    pt2 = strtok(sep1 + 1, "/");
	else
	    pt2 = NULL;
        if ( pt2 && strlen(pt2) >= SBUF_SIZE )
           *(pt2+SBUF_SIZE-1) = '\0';
	do {
	    if (pt2) {
		if (ok_attr_name(pt2)) {
		    ca3 = 1;
		    ca2 = mkattr(pt2);
		} else
		    ca3 = 0;
	    } else
		ca3 = 0;
            buff2 = alloc_lbuf("cpattr_atrlock");
	    for (pt1 = atrpt; pt1; pt1 = pt1->next) {
                no_set = 0;
		if ((ca == -1) || (!ca3))
		    t2 = pt1->atr;
		else
		    t2 = ca2;
                if ( t2 >= 0 ) {
		   atr_get_info(thing2, t2, &aowner, &aflags);
		   attr = atr_num(t2);
                } else {
                   attr = NULL;
                }
                if ( attr && !(((attr->flags) & AF_NOANSI) && index(pt1->info,ESC_CHAR)) ) {
                   if (!(((attr->flags) & AF_NORETURN) && (index(pt1->info,'\n') || index(pt1->info,'\r')))) {
		      if (((Set_attr(player, thing2, attr, aflags)) || 
			    (!Set_attr(player, thing2, attr, aflags) && twk1 && !(attr->flags & AF_LOCK))) &&
                            ((!((attr->flags & AF_IMMORTAL) || (aflags & AF_IMMORTAL)) ||
                            Immortal(player))) && ((!((attr->flags & AF_GOD) || (aflags & AF_GOD)) || God(player))) ) {
                         if ( Good_obj(mudconf.global_attrdefault) &&
                              !Recover(mudconf.global_attrdefault) &&
                              !Going(mudconf.global_attrdefault) &&
                              ((attr->flags & AF_ATRLOCK) || (aflags & AF_ATRLOCK)) &&
                              (thing2 != mudconf.global_attrdefault) ) {
                            atr_get_str(buff2, mudconf.global_attrdefault, t2, &aowner2, &aflags2);
                            if ( *buff2 ) {
                               buff2ret = exec(player, mudconf.global_attrdefault, mudconf.global_attrdefault,
                                               EV_STRIP | EV_FCHECK | EV_EVAL, buff2, &(pt1->info), 1);
                               if ( atoi(buff2ret) == 0 ) {
                                  free_lbuf(buff2ret);
                                  no_set = 1;
                               }
                               free_lbuf(buff2ret);
                            }
                         }
                         if ( !no_set ) {
		            mudstate.vlplay = player;
		            atr_add(thing2, t2, pt1->info, player, pt1->flags);
                            if ( (attr->flags & AF_LOGGED) || (aflags & AF_LOGGED) ) {
                               STARTLOG(LOG_ALWAYS, "LOG", "ATTR");
                                log_name_and_loc(player);
                                buff2ret = alloc_lbuf("log_attribute");
                                sprintf(buff2ret, " <cause: #%d> Attribute '%s' on #%d set to '%.3940s'", 
                                                   cause, attr->name, thing2, pt1->info);
                                log_text(buff2ret);
                                free_lbuf(buff2ret);
                                ENDLOG
                            }
		            atrpush(-1, attr->name, thing2);
		            if (mudstate.vlplay != NOTHING)
		               pt1->wt = 1;
		            else
		               notify_quiet(player,"Permission denied.");
                         }
		      }
                   }
                }
	    }
            free_lbuf(buff2);
	    if (pt2)
		pt2 = strtok(NULL, "/");
	} while (pt2 && (ca != -1));
    }
    free_lbuf(tpr_buff);
    if (key == CPATTR_CLEAR) {
	ca = Controls(player, thing1);
        clr1 = clr2 = 0;
	for (pt1 = atrpt; pt1; pt1 = pt1->next) {
	    if (pt1->wt) {
		atr_get_info(thing1, pt1->atr, &aowner, &aflags);
		attr = atr_num(pt1->atr);
		if (ca && Set_attr(player, thing1, attr, aflags)) {
		    atr_clr(thing1, pt1->atr);
                    clr1++;
                    if ( (attr->flags & AF_LOGGED) || (aflags & AF_LOGGED) ) {
                        STARTLOG(LOG_ALWAYS, "LOG", "ATTR");
                        log_name_and_loc(player);
                        buff2ret = alloc_lbuf("log_attribute");
                        sprintf(buff2ret, " <cause: #%d> Attribute '%s' on #%d cleared",
                                           cause, attr->name, thing1);
                        log_text(buff2ret);
                        free_lbuf(buff2ret);
                        ENDLOG
                    }

                } 
                clr2++;
	    }
	}
    }
    if (atrpt2) {
	sep1 = alloc_lbuf("cpattr");
	as = sep1;
	safe_str("Cpattr: ", sep1, &as);
	safe_str(Name(thing1), sep1, &as);
	safe_chr('/', sep1, &as);
	safe_str(sepsave + 1, sep1, &as);
	if (key == CPATTR_CLEAR) {
            if ( clr1 == 0 )
	       safe_str(" (Unable to Clear)", sep1, &as);
            else if (clr1 != clr2)
	       safe_str(" (Partially Cleared)", sep1, &as);
            else
	       safe_str(" (Cleared)", sep1, &as);
        }
	ca = -1;
	ca3 = 0;
	for (pt1 = atrpt2; pt1; pt1 = pt1->next) {
	    if (pt1->flags != ca) {
		if (ca3)
		    safe_chr(']', sep1, &as);
		safe_str("\r\n\t-> ", sep1, &as);
		safe_str(Name(pt1->flags), sep1, &as);
		safe_chr('/', sep1, &as);
		ca = pt1->flags;
		if ((pt1->next) && (pt1->next->flags == ca) && ((wyes && verbose) || !wyes)) {
		    ca3 = 1;
		    safe_chr('[', sep1, &as);
		} else
		    ca3 = 0;
	    }
	    if (wyes && !verbose) {
		safe_str(sepsave + 1, sep1, &as);
		ca2 = pt1->flags;
		while ((pt1->next) && ((pt1->next->flags) == ca2))
		    pt1 = pt1->next;
	    } else {
		if ((*(as - 1) != '[') && (*(as - 1) != '/'))
		    safe_chr(',', sep1, &as);
		safe_str(pt1->info, sep1, &as);
	    }
	}
	if (ca3)
	    safe_chr(']', sep1, &as);
	*as = '\0';
	notify(player, sep1);
	free_lbuf(sep1);
    } else
	notify(player, "Cpattr: Nothing copied.");
    atrptclr();
    free_lbuf(bk_source);
}

static void 
look_atrs1(dbref player, dbref thing, dbref othing,
	   int check_exclude, int hash_insert)
{
    dbref aowner;
    int ca, aflags;
    ATTR *attr;
    char *as, *buf;

    for (ca = atr_head(thing, &as); ca; ca = atr_next(&as)) {
	if ((ca == A_DESC) || (ca == A_LOCK))
	    continue;
	attr = atr_num(ca);
	if (!attr)
	    continue;

	/* Should we exclude this attr? */

	if (check_exclude &&
	    ((attr->flags & AF_PRIVATE) ||
	     nhashfind(ca, &mudstate.parent_htab)))
	    continue;

	buf = atr_get(thing, ca, &aowner, &aflags);
	if (Read_attr(player, othing, attr, aowner, aflags, 0)) {

	    if (!(check_exclude && (aflags & AF_PRIVATE))) {
		if (hash_insert)
		    nhashadd(ca, (int *) attr,
			     &mudstate.parent_htab);
		view_atr(player, thing, attr, buf,
			 aowner, aflags, 0, (thing != othing ? thing : -1));
	    }
	}
	free_lbuf(buf);
    }
}

static void 
look_atrs(dbref player, dbref thing, int check_parents)
{
    dbref parent;
    int lev, check_exclude, hash_insert;

    if (!check_parents) {
	look_atrs1(player, thing, thing, 0, 0);
    } else {
	hash_insert = 1;
	check_exclude = 0;
	nhashflush(&mudstate.parent_htab, 0);
	ITER_PARENTS(thing, parent, lev) {
	    if (!Good_obj(Parent(parent)))
		hash_insert = 0;
	    look_atrs1(player, parent, thing,
		       check_exclude, hash_insert);
	    check_exclude = 1;
            if ( Good_obj(Parent(parent)) ) {
             if ( NoEx(Parent(parent)) && !Wizard(player) )
               break;
             if (Backstage(player) && NoBackstage(Parent(parent)) &&
                 !Immortal(player))
               break;
            }
	}
    }
}

long 
count_atrs(dbref thing)
{
    dbref aowner;
    int ca, aflags, size;
    ATTR *attr;
    char *as, *buf;
    long tlen;

    tlen = 0;
    size = sizeof(int) * 3;

    for (ca = atr_head(thing, &as); ca; ca = atr_next(&as)) {
	attr = atr_num(ca);
	if (!attr)
	    continue;

	/* Should we exclude this attr? */

	buf = atr_get(thing, ca, &aowner, &aflags);
	tlen += strlen(buf) + 1 + size;

	free_lbuf(buf);
    }
    return( tlen );
}

static void 
look_simple(dbref player, dbref thing, int obey_terse)
{
    char *buff, *pbuf, *tpr_buff, *tprp_buff;
    dbref aowner;
    int pattr, aflags;

    /* Only makes sense for things that can hear */

    if (!Good_obj(thing))
        return;
    if (!Hearer(player))
	return;

    /* Get the name and db-number if we can examine it. */

    if (Examinable(player, thing)) {
        if ( NoAnsiName(thing) || NoAnsiName(Owner(thing)) ||
           ((NoAnsiPlayer(player) || !(mudconf.allow_ansinames & ANSI_PLAYER)) && isPlayer(thing)) ||
           ((NoAnsiThing(player) || !(mudconf.allow_ansinames & ANSI_THING)) && isThing(thing)) ||
           ((NoAnsiExit(player) || !(mudconf.allow_ansinames & ANSI_EXIT)) && isExit(thing)) ||
           ((NoAnsiRoom(player) || !(mudconf.allow_ansinames & ANSI_ROOM)) && isRoom(thing)) )
           if (isPlayer(thing) || isThing(thing))
	      buff = unparse_object_altname(player, thing, 1);
           else
	      buff = unparse_object(player, thing, 1);
        else
           if (isPlayer(thing) || isThing(thing))
              buff = unparse_object_ansi_altname(player, thing, 1);
           else
              buff = unparse_object_ansi(player, thing, 1);

        if ( Good_obj(thing) && ((NoName(thing) && *buff) || !NoName(thing)) ) {
           if(Good_obj(thing) && isPlayer(thing)) {
              pbuf = atr_get(thing, A_CAPTION, &aowner, &aflags);
              if(*pbuf) {
                 tprp_buff = tpr_buff = alloc_lbuf("look_simple");
                 notify(player, safe_tprintf(tpr_buff, &tprp_buff, "%s, %.40s", buff, pbuf));
                 free_lbuf(tpr_buff);
              } else
                 notify(player, buff);
              free_lbuf(pbuf);
           } else
              notify(player, buff);
        }

	free_lbuf(buff);
    }
    pattr = (obey_terse && Terse(player)) ? 0 : A_DESC;
#ifdef REALITY_LEVELS
    did_it_rlevel(player, thing, pattr, "You see nothing special.",
           A_ODESC, NULL, A_ADESC, (char **) NULL, 0);
#else
    did_it(player, thing, pattr, "You see nothing special.",
           A_ODESC, NULL, A_ADESC, (char **) NULL, 0);
#endif /* REALITY_LEVELS */

    if (!mudconf.quiet_look && (!Terse(player) || mudconf.terse_look)) {
	look_atrs(player, thing, 0);
    }
}

static void 
show_desc(dbref player, dbref loc, int key)
{
    char *got;
    dbref aowner;
    int aflags;

    if ((key & LK_OBEYTERSE) && Terse(player)) {
	did_it(player, loc, 0, NULL, A_ODESC, NULL,
	       A_ADESC, (char **) NULL, 0);
    }
    else if ((Typeof(loc) != TYPE_ROOM) && (key & LK_IDESC)) {
	if (*(got = atr_pget(loc, A_IDESC, &aowner, &aflags)))
#ifdef REALITY_LEVELS
            did_it_rlevel(player, loc, A_IDESC, NULL, A_ODESC, NULL,
                   A_ADESC, (char **) NULL, 0);
#else
            did_it(player, loc, A_IDESC, NULL, A_ODESC, NULL,
                   A_ADESC, (char **) NULL, 0);
#endif /* REALITY_LEVELS */
	else
#ifdef REALITY_LEVELS
            did_it_rlevel(player, loc, A_DESC, NULL, A_ODESC, NULL,
                   A_ADESC, (char **) NULL, 0);
#else
            did_it(player, loc, A_DESC, NULL, A_ODESC, NULL,
                   A_ADESC, (char **) NULL, 0);
#endif /* REALITY_LEVELS */ 
	free_lbuf(got);
    } else {
#ifdef REALITY_LEVELS
        did_it_rlevel(player, loc, A_DESC, NULL, A_ODESC, NULL, A_ADESC,
               (char **) NULL, 0);
#else
        did_it(player, loc, A_DESC, NULL, A_ODESC, NULL, A_ADESC,
               (char **) NULL, 0);
#endif /* REALITY_LEVELS */
    }
    if ( Good_obj(player) && LogRoom(loc) && (Location(player) == loc) ) {
       notify_quiet(player, "This room is currently logging what it hears.");
    }
}

void 
look_in(dbref player, dbref loc, int key)
{
    char *s, *nfmt, *pt, *savereg[MAX_GLOBAL_REGS];
    int pattr, oattr, aattr, is_terse, showkey, has_namefmt, aflags2, x;
    dbref aowner2;

    is_terse = (key & LK_OBEYTERSE) ? Terse(player) : 0;

    /* Only makes sense for things that can hear */

    if (!Hearer(player))
	return;

    /* tell him the name, and the number if he can link to it */
    if ( !Good_obj(loc) || Going(loc) || Recover(loc) )
        return;

    has_namefmt = 0;
    if ( mudconf.fmt_names && (isRoom(loc) || isThing(loc)) && !NoName(loc) ) {
       nfmt = atr_pget(loc, A_NAME_FMT, &aowner2, &aflags2);
       if ( *nfmt ) {
          has_namefmt = 1;
       } else {
          free_lbuf(nfmt);
       }
    }
    if ( !has_namefmt ) {
       if ( NoAnsiName(loc) || NoAnsiName(Owner(loc)) ||
          ((NoAnsiPlayer(player) || !(mudconf.allow_ansinames & ANSI_PLAYER)) && isPlayer(loc)) ||
          ((NoAnsiThing(player) || !(mudconf.allow_ansinames & ANSI_THING)) && isThing(loc)) ||
          ((NoAnsiRoom(player) || !(mudconf.allow_ansinames & ANSI_ROOM)) && isRoom(loc)) )
          if (isPlayer(loc) || isThing(loc))
             s = unparse_object_altname(player, loc, 1);
          else
             s = unparse_object(player, loc, 1);
       else
          if (isPlayer(loc) || isThing(loc))
             s = unparse_object_ansi_altname(player, loc, 1);
          else
             s = unparse_object_ansi(player, loc, 1);
    } else {
       mudstate.chkcpu_stopper = time(NULL);
       mudstate.chkcpu_toggle = 0;
       if ( mudconf.formats_are_local ) {
          for (x = 0; x < MAX_GLOBAL_REGS; x++) {
            savereg[x] = alloc_lbuf("ulocal_reg");
            pt = savereg[x];
            safe_str(mudstate.global_regs[x],savereg[x],&pt);
          }
       }
       s = exec(loc, player, player, EV_FIGNORE|EV_EVAL|EV_TOP, nfmt, (char **) NULL, 0);
       if ( mudconf.formats_are_local ) {
          for (x = 0; x < MAX_GLOBAL_REGS; x++) {
            pt = mudstate.global_regs[x];
            safe_str(savereg[x],mudstate.global_regs[x],&pt);
            free_lbuf(savereg[x]);
          }
       }
       free_lbuf(nfmt);
    }
    notify(player, s);
    free_lbuf(s);
    if (!Good_obj(loc))
	return;			/* If we went to NOTHING et al, 
				   skip the rest */

    /* tell him the description */

    showkey = 0;
    if (loc == Location(player))
	showkey |= LK_IDESC;
    if (key & LK_OBEYTERSE)
	showkey |= LK_OBEYTERSE;
    show_desc(player, loc, showkey);

    /* tell him the appropriate messages if he has the key */

    if (Typeof(loc) == TYPE_ROOM) {
	if (could_doit(player, loc, A_LOCK,1)) {
	    pattr = A_SUCC;
	    oattr = A_OSUCC;
	    aattr = A_ASUCC;
	} else {
	    pattr = A_FAIL;
	    oattr = A_OFAIL;
	    aattr = A_AFAIL;
	}
	if (is_terse)
	    pattr = 0;
	did_it(player, loc, pattr, NULL, oattr, NULL,
	       aattr, (char **) NULL, 0);
    }
    /* tell him the attributes, contents and exits */

    if ((key & LK_SHOWATTR) && !mudconf.quiet_look && !is_terse)
	look_atrs(player, loc, 0);
    if (!is_terse || mudconf.terse_contents)
	look_contents(player, loc, "Contents:");
    if ((key & LK_SHOWEXIT) && (!is_terse || mudconf.terse_exits)) {
	look_exits(player, loc, "Obvious exits:", 0);
        look_exits(player, mudconf.master_room, "Global exits:", 1);
    }
}

void 
do_look(dbref player, dbref cause, int key, char *name)
{
    dbref thing, loc, look_key;

    look_key = LK_SHOWATTR | LK_SHOWEXIT;
    if (!mudconf.terse_look)
	look_key |= LK_OBEYTERSE;

    loc = Location(player);
    if (!name || !*name) {
	thing = loc;
	if (Good_obj(thing)) {
	    if (key & LOOK_OUTSIDE) {
		if ((Typeof(thing) == TYPE_ROOM) ||
		    Opaque(thing)) {
		    notify_quiet(player,
				 "You can't look outside.");
		    return;
		}
		thing = Location(thing);
	    }
	    look_in(player, thing, look_key);
	}
	return;
    }
    /* Look for the target locally */

    thing = (key & LOOK_OUTSIDE) ? loc : player;
    init_match(thing, name, NOTYPE);
    match_exit_with_parents();
    match_neighbor();
    match_altname();
    match_possession();
    match_possession_altname();
    if (Privilaged(player) || HasPriv(player, NOTHING, POWER_LONG_FINGERS, POWER3, NOTHING)) {
	match_absolute();
	match_player();
    }
    match_here();
    match_me();
    match_master_exit();
    thing = match_result();

    /* Not found locally, check possessive */

    if (!Good_obj(thing)) {
	thing = match_status(player,
			     match_possessed(player,
				      ((key & LOOK_OUTSIDE) ? loc : player),
					     (char *) name, thing, 0));
    }
    /* If we found something, go handle it */

    if (Good_obj(thing)) {
#ifdef REALITY_LEVELS
        if (!IsReal(player, thing))
           return;
#endif /* REALITY_LEVELS */
	switch (Typeof(thing)) {
	case TYPE_ROOM:
	    if ((thing == loc) || (!Cloak(thing)) || (Cloak(thing) && SCloak(thing) && Immortal(player)) ||
		(Cloak(thing) && Wizard(player) && (!SCloak(thing) || Immortal(player)))) {
	       look_in(player, thing, look_key);
            } else
		notify(player, "I don't see that here.");
	    break;
	case TYPE_THING:
	case TYPE_PLAYER:
	    if ((thing == loc) || (!Cloak(thing)) || (Cloak(thing) && SCloak(thing) && Immortal(player)) ||
		(Cloak(thing) && Wizard(player) && (!SCloak(thing) || Immortal(player)))) {
		look_simple(player, thing, !mudconf.terse_look);
		if (!(Flags(thing) & OPAQUE) &&
		    (!Terse(player) || mudconf.terse_contents)) {
		    look_contents_altinv(player, thing, "Carrying:");
		}
                if (mudconf.showother_altinv && mudconf.alt_inventories)
                   look_altinv(player, thing, "Equipped:");
	    } else
		notify(player, "I don't see that here.");
	    break;
	case TYPE_EXIT:
	    if ((!Cloak(thing)) || (Cloak(thing) && SCloak(thing) && Immortal(player)) ||
		(Cloak(thing) && Wizard(player) && (!SCloak(thing) || Immortal(player)))) {
	       look_simple(player, thing, !mudconf.terse_look);
	       if (Transparent(thing) &&
		   (Location(thing) != NOTHING)) {
		   look_key &= ~LK_SHOWATTR;
		   look_in(player, Location(thing), look_key);
	       }
            } else 
		notify(player, "I don't see that here.");
	    break;
	default:
	    look_simple(player, thing, !mudconf.terse_look);
	}
    }
}

void viewzonelist( dbref player, dbref thing ) 
{
  ZLISTNODE* ptr; 
  char *buf, *buf2;
  char *cp;
  int i_numzones;

  if( !db[thing].zonelist ) 
    return;

  buf = alloc_lbuf("viewzonelist");
  cp = buf;
  safe_str((char *) ANSIEX(ANSI_HILITE), buf, &cp);
  safe_str((char *) "ZoneList: ", buf, &cp);
  safe_str((char *) ANSIEX(ANSI_NORMAL), buf, &cp);

  i_numzones = 0;
  for( ptr = db[thing].zonelist; ptr; ptr = ptr->next )
     i_numzones++;
  if (i_numzones <= 20)
     i_numzones = 0;
  else
     buf2 = alloc_sbuf("viewzonelist.sbuf");
  
  for( ptr = db[thing].zonelist; ptr; ptr = ptr->next ) {
    if (!i_numzones) {
       buf2 = unparse_object(player, ptr->object, 0);
       safe_str(buf2, buf, &cp);
       free_lbuf(buf2);
    } else {
       sprintf(buf2, "#%d", ptr->object);
       safe_str(buf2, buf, &cp);
    }

    if( ptr->next ) {
      safe_str((char *) ", ", buf, &cp);
    }
  }
  if ( i_numzones > 0)
    free_sbuf(buf2);

  safe_chr('\0', buf, &cp);
  notify(player, buf);
  free_lbuf(buf);
}


static void 
debug_examine(dbref player, dbref thing)
{
    dbref aowner;
    char *buf, *tpr_buff, *tprp_buff;
    int aflags, ca;
    BOOLEXP *bool;
    ATTR *attr;
    char *as, *cp;

    notify(player, unsafe_tprintf("Number  = %d", thing));
    if (!Good_obj(thing))
	return;

    notify(player, unsafe_tprintf("Name    = %s", Name(thing)));
    notify(player, unsafe_tprintf("Location= %d", Location(thing)));
    notify(player, unsafe_tprintf("Contents= %d", Contents(thing)));
    notify(player, unsafe_tprintf("Exits   = %d", Exits(thing)));
    notify(player, unsafe_tprintf("Link    = %d", Link(thing)));
    notify(player, unsafe_tprintf("Next    = %d", Next(thing)));
    notify(player, unsafe_tprintf("Owner   = %d", Owner(thing)));
    notify(player, unsafe_tprintf("Pennies = %d", Pennies(thing)));
    viewzonelist(player, thing);
    buf = flag_description(player, thing, 1);
    notify(player, unsafe_tprintf("Flags   = %s", buf));
    free_lbuf(buf);
    buf = toggle_description(player, thing, 1, 0);
    notify(player, unsafe_tprintf("Toggles = %s", buf));
    free_lbuf(buf);
    buf = power_description(player, thing, 0, 1);
    notify(player, unsafe_tprintf("Powers = %s", buf));
    free_lbuf(buf);
    buf = depower_description(player, thing, 0, 1);
    notify(player, unsafe_tprintf("Depowers = %s", buf));
    free_lbuf(buf);
#ifdef REALITY_LEVELS
    buf = rxlevel_description(player, thing, 0, 1);
    notify(player, unsafe_tprintf("RxLevel = %s", buf));
    free_lbuf(buf);
    buf = txlevel_description(player, thing, 0, 1);
    notify(player, unsafe_tprintf("TxLevel = %s", buf));
    free_lbuf(buf);
#endif /* REALITY_LEVELS */
    buf = atr_get(thing, A_LOCK, &aowner, &aflags);
    bool = parse_boolexp(player, buf, 1);
    free_lbuf(buf);
    notify(player, unsafe_tprintf("Lock    = %s", unparse_boolexp(player, bool)));
    free_boolexp(bool);

    buf = alloc_lbuf("debug_dexamine");
    cp = buf;
    safe_str((char *) "Attr list: ", buf, &cp);

    tprp_buff = tpr_buff = alloc_lbuf("debug_examine");
    for (ca = atr_head(thing, &as); ca; ca = atr_next(&as)) {
	attr = atr_num(ca);
	if (!attr)
	    continue;

	atr_get_info(thing, ca, &aowner, &aflags);
	if (Read_attr(player, thing, attr, aowner, aflags, 0)) {
	    if (attr) {		/* Valid attr. */
		safe_str((char *) attr->name, buf, &cp);
		safe_chr(' ', buf, &cp);
	    } else {
                tprp_buff = tpr_buff;
		safe_str(safe_tprintf(tpr_buff, &tprp_buff, "%d ", ca), buf, &cp);
	    }
	}
    }
    free_lbuf(tpr_buff);
    *cp = '\0';
    noansi_notify(player, buf);
    free_lbuf(buf);

    for (ca = atr_head(thing, &as); ca; ca = atr_next(&as)) {
	attr = atr_num(ca);
	if (!attr)
	    continue;

	buf = atr_get(thing, ca, &aowner, &aflags);
	if (Read_attr(player, thing, attr, aowner, aflags, 0))
	    view_atr(player, thing, attr, buf, aowner, aflags, 0, -1);
	free_lbuf(buf);
    }
}

static void 
exam_wildattrs(dbref player, dbref thing, int do_parent,
	       OBLOCKMASTER * master, int key)
{
    int atr, aflags, got_any, pobj;
    char *buf;
    dbref aowner;
    ATTR *ap;

    got_any = 0;
    for (atr = olist_first(master); atr != NOTHING; atr = olist_next(master)) {
        ap = atr_num(atr);
        if (!ap)
	    continue;

        pobj=-1;

        if (do_parent && !(ap->flags & AF_PRIVATE)) {
              buf = alloc_lbuf("parent_exam");
              pobj=player;
              atr_pget_str(buf, thing, atr, &aowner, &aflags, &pobj);
              if ( pobj == thing )
                 pobj = -1;
        } else
	    buf = atr_get(thing, atr, &aowner, &aflags);

	/* Decide if the player should see the attr:
	 * If obj is Examinable and has rights to see, yes.
	 * If a player and has rights to see, yes...
	 *   except if faraway, attr=DESC, and
	 *   remote DESC-reading is not turned on.
	 * If I own the attrib and have rights to see, yes...
	 *   except if faraway, attr=DESC, and
	 *   remote DESC-reading is not turned on.
	 */

	if (Examinable(player, thing) &&
	    Read_attr(player, thing, ap, aowner, aflags, 0)) {
	    got_any = 1;
	    view_atr(player, thing, ap, buf,
		     aowner, aflags, 0, pobj);
	} else if ((Typeof(thing) == TYPE_PLAYER) &&
		   Read_attr(player, thing, ap, aowner, aflags, 0)) {
	    got_any = 1;
	    if (aowner == Owner(player)) {
		view_atr(player, thing, ap, buf,
			 aowner, aflags, 0, pobj);
	    } else if ((atr == A_DESC) &&
		       (mudconf.read_rem_desc ||
			nearby(player, thing))) {
		show_desc(player, thing, 0);
	    } else if (atr != A_DESC) {
		view_atr(player, thing, ap, buf,
			 aowner, aflags, 0, pobj);
	    } else {
		notify(player,
		       "<Too far away to get a good look>");
	    }
	} else if (Read_attr(player, thing, ap, aowner, aflags, 0)) {
	    got_any = 1;
	    if (aowner == Owner(player)) {
		view_atr(player, thing, ap, buf,
			 aowner, aflags, 0, pobj);
	    } else if ((atr == A_DESC) &&
		       (mudconf.read_rem_desc ||
			nearby(player, thing))) {
		show_desc(player, thing, 0);
	    } else if (nearby(player, thing)) {
		view_atr(player, thing, ap, buf,
			 aowner, aflags, 0, pobj);
	    } else {
		notify(player,
		       "<Too far away to get a good look>");
	    }
        }
	free_lbuf(buf);
    }

    if (!got_any && !key)
	notify_quiet(player, "No matching attributes found.");

}

void 
do_examine(dbref player, dbref cause, int key, char *name)
{
    dbref thing, content, exit, aowner, loc, aowner2;
    char savec;
    char *temp, *buf, *buf2, *modbuf, *find_chr;
    BOOLEXP *bool;
    int control, aflags, do_parent, echeck, aflags2, keyfound;
    OBLOCKMASTER master;
    int cntr=0;
    ATTR *a_chk;

    /* This command is pointless if the player can't hear. */

    if (!Hearer(player))
	return;

    mudstate.outputflushed = 0;
    do_parent = key & EXAM_PARENT;
    thing = NOTHING;
    keyfound = 0;
    if (!name || !*name) {
	thing = Location(player);
	if (!Good_obj(thing))
	    return;
    } else {
	/* Check for obj/attr first. */
	olist_init(&master);
	if (parse_attrib_wild(player, name, &thing, do_parent, 1, 0, &master, 0)) {
    	  if (Cloak(thing)) {
	    if (Immortal(thing) && SCloak(thing) && !(Immortal(cause))) {
	      notify(player, "I don't see that here.");
	      olist_cleanup(&master);
	      return;
	    } else if (Wizard(thing) && !(Wizard(cause))) {
	      notify(player, "I don't see that here.");
	      olist_cleanup(&master);
	      return;
	    }
    	  }
          if (Going(thing)) {
            if (!(Immortal(cause))) {
	      notify(player, "I don't see that here.");
	      olist_cleanup(&master);
	      return;
            }
          }
          if ( mudconf.examine_restrictive && !Immortal(player) && !Controls(player, thing) ) {
             if ( ((mudconf.examine_restrictive == 1) && !Guildmaster(player)) ||
                  ((mudconf.examine_restrictive == 2) && !Builder(player)) ||
                  ((mudconf.examine_restrictive == 3) && !Admin(player)) ||
                  ((mudconf.examine_restrictive == 4) && !Wizard(player)) ||
                  ((mudconf.examine_restrictive == 5) && !Immortal(player)) ) {
                notify(player, "You don't have permission to examine that.");
	        olist_cleanup(&master);
                return;
             }
          }
          /* Mod/Create stamps are usually INTERNAL flags and can't be normally checked */
          if ( mudconf.enable_tstamps && Controls(player, thing) ) {
             find_chr = strchr(name, '/');
             if ( *find_chr )
                find_chr++;
             if ( *find_chr && quick_wild(find_chr, "ModifiedStamp") ) {
                modbuf = atr_get(thing, A_MODIFY_TIME, &aowner2, &aflags2);
                a_chk = atr_num(A_MODIFY_TIME);
                if ( ((aflags2 & AF_INTERNAL) || (a_chk && (a_chk->flags & AF_INTERNAL))) ) {
                   if ( *modbuf )
                      notify(player, unsafe_tprintf("%sModifiedStamp:%s %s",
                                     ANSIEX(ANSI_HILITE), ANSIEX(ANSI_NORMAL), modbuf));
                   else
                      notify(player, unsafe_tprintf("%sModifiedStamp:%s %s",
                                     ANSIEX(ANSI_HILITE), ANSIEX(ANSI_NORMAL), "**NULL**"));
                }
		keyfound = 1;
                free_lbuf(modbuf);
             } 
             if ( *find_chr && quick_wild(find_chr, "CreatedStamp") ) {
                modbuf = atr_get(thing, A_CREATED_TIME, &aowner2, &aflags2);
                a_chk = atr_num(A_CREATED_TIME);
                if ( ((aflags2 & AF_INTERNAL) || (a_chk && (a_chk->flags & AF_INTERNAL))) ) {
                   if ( *modbuf )
                      notify(player, unsafe_tprintf("%sCreatedStamp:%s %s",
                                     ANSIEX(ANSI_HILITE), ANSIEX(ANSI_NORMAL), modbuf));
                   else
                      notify(player, unsafe_tprintf("%sCreatedStamp:%s %s",
                                     ANSIEX(ANSI_HILITE), ANSIEX(ANSI_NORMAL), "**NULL**"));
                }
		keyfound = 1;
                free_lbuf(modbuf);
             }
          }
	  exam_wildattrs(player, thing, do_parent, &master, keyfound);
	  olist_cleanup(&master);
          if ( mudstate.outputflushed ) {
              notify_quiet(player, "WARNING: Output limited exceeded on examine.  Attributes cut off.");
          }
	  return;
	}
	olist_cleanup(&master);

	/* Look it up */

	init_match(player, name, NOTYPE);
	match_everything(MAT_EXIT_PARENTS);
	thing = noisy_match_result();
	if (!Good_obj(thing))
	    return;
    }

    if (Cloak(thing)) {
	if (Immortal(thing) && SCloak(thing) && !(Immortal(cause))) {
	    notify(player, "I don't see that here.");
	    return;
	} else if (Wizard(thing) && !(Wizard(cause))) {
	    notify(player, "I don't see that here.");
	    return;
	}
    }
    if (Going(thing) || Recover(thing)) {
       if (!(Immortal(cause))) {
	  notify(player, "I don't see that here.");
	  return;
       }
    }

    if ( mudconf.examine_restrictive && !Immortal(player) && !Controls(player,thing) ) {
       if ( ((mudconf.examine_restrictive == 1) && !Guildmaster(player)) ||
            ((mudconf.examine_restrictive == 2) && !Builder(player)) ||
            ((mudconf.examine_restrictive == 3) && !Admin(player)) ||
            ((mudconf.examine_restrictive == 4) && !Wizard(player)) ||
            ((mudconf.examine_restrictive == 5) && !Immortal(player)) ) {
              notify(player, "You don't have permission to examine that.");
              return;
       }
    }
    /* Check for the /debug switch */

    if (key == EXAM_DEBUG) {
	if (!Examinable(player, thing)) {
	    notify_quiet(player, "Permission denied.");
	} else {
	    debug_examine(player, thing);
	}
        if ( mudstate.outputflushed ) {
            notify_quiet(player, "WARNING: Output limited exceeded on examine.  Attributes cut off.");
        }
	return;
    }
    control = (Examinable(player, thing) || Link_exit(player, thing));
    if (control && (key != EXAM_QUICK)) {
	buf2 = unparse_object(player, thing, 0);
	notify(player, buf2);
	free_lbuf(buf2);
	if (mudconf.ex_flags) {
	   buf2 = flag_description(player, thing, 1);
	   notify(player, buf2);
	   free_lbuf(buf2);
	}
    } else {
	if ((key == EXAM_QUICK) ||
	   ((key == EXAM_DEFAULT) && !mudconf.exam_public)) {
	   if (mudconf.read_rem_name) {
		buf2 = alloc_lbuf("do_examine.pub_name");
                sprintf(buf2, "%.3900s", Name(thing));
		notify(player,
		          unsafe_tprintf("%s is owned by %s",
			          buf2, Name(Owner(thing))));
		free_lbuf(buf2);
	   } else {
		notify(player,
		          unsafe_tprintf("Owned by %s",
			          Name(Owner(thing))));
	   }
	   return;
	}
    }

    temp = alloc_lbuf("do_examine.info");

    if (control || mudconf.read_rem_desc || nearby(player, thing) || Privilaged(player) ||
	HasPriv(player, NOTHING, POWER_LONG_FINGERS, POWER3, NOTHING)) {
	temp = atr_get_str(temp, thing, A_DESC, &aowner, &aflags);
	if (*temp) {
	    if (Examinable(player, thing) ||
		(aowner == Owner(player))) {
		view_atr(player, thing, atr_num(A_DESC), temp,
			 aowner, aflags, 1, -1);
	    } else {
		show_desc(player, thing, 0);
	    }
	}
    } else {
	notify(player, "<Too far away to get a good look>");
    }

    if (control) {

	/* print owner, key, and value */

	savec = mudconf.many_coins[0];
	mudconf.many_coins[0] =
	    (islower((int)savec) ? toupper((int)savec) : savec);
	buf2 = atr_get(thing, A_LOCK, &aowner, &aflags);
	bool = parse_boolexp(player, buf2, 1);
	buf = unparse_boolexp(player, bool);
	free_boolexp(bool);
        sprintf(buf2, "%.3900s", Name(Owner(thing)));
        if (FreeFlag(thing) && 
            (Typeof(thing) == TYPE_PLAYER) &&
            !DePriv(thing, NOTHING, DP_FREE, POWER6, POWER_LEVEL_NA))
	    notify(player,
		   unsafe_tprintf("%sOwner:%s %s  %sKey:%s %s %s%s:%s Free (%d)", 
                           ANSIEX(ANSI_HILITE), ANSIEX(ANSI_NORMAL), buf2, 
                           ANSIEX(ANSI_HILITE), ANSIEX(ANSI_NORMAL), buf,
			   ANSIEX(ANSI_HILITE), mudconf.many_coins,
                           ANSIEX(ANSI_NORMAL), Pennies(thing)));
	else if ((Builder(thing)) &&
	    (Typeof(thing) == TYPE_PLAYER) &&
	    !DePriv(thing, NOTHING, DP_FREE, POWER6, POWER_LEVEL_NA))
	    notify(player,
		   unsafe_tprintf("%sOwner:%s %s  %sKey:%s %s %s%s:%s Unlimited", 
                           ANSIEX(ANSI_HILITE), ANSIEX(ANSI_NORMAL), buf2, 
                           ANSIEX(ANSI_HILITE), ANSIEX(ANSI_NORMAL), buf,
			   ANSIEX(ANSI_HILITE), mudconf.many_coins,
                           ANSIEX(ANSI_NORMAL)));
	else
	    notify(player,
		   unsafe_tprintf("%sOwner:%s %s  %sKey:%s %s %s%s:%s %d", 
                           ANSIEX(ANSI_HILITE), ANSIEX(ANSI_NORMAL), buf2, 
                           ANSIEX(ANSI_HILITE), ANSIEX(ANSI_NORMAL), buf,
			   ANSIEX(ANSI_HILITE), mudconf.many_coins, 
                           ANSIEX(ANSI_NORMAL), Pennies(thing)));
	free_lbuf(buf2);

        viewzonelist(player, thing);
	buf2 = toggle_description(player, thing, 1, 0);
	if (strlen(buf2) > 9 + strlen(ANSIEX(ANSI_HILITE)) +
                           strlen(ANSIEX(ANSI_NORMAL)))	/* Toggles:  */
	    notify(player, buf2);
	free_lbuf(buf2);
	buf2 = power_description(player, thing, 0, 1);
	if (strlen(buf2) > 8 + strlen(ANSIEX(ANSI_HILITE)) + 
                           strlen(ANSIEX(ANSI_NORMAL)))
	    notify(player, buf2);
	free_lbuf(buf2);
	buf2 = depower_description(player, thing, 0, 1);
	if (strlen(buf2) > 10 + strlen(ANSIEX(ANSI_HILITE)) +
                           strlen(ANSIEX(ANSI_NORMAL)))
	    notify(player, buf2);
	free_lbuf(buf2);

#ifdef REALITY_LEVELS
        /* Show Rx & Tx levels. Always, even if empty */
        buf2 = rxlevel_description(player, thing, 0, 1);
        notify(player, buf2);
        free_lbuf(buf2);
        buf2 = txlevel_description(player, thing, 0, 1);
        notify(player, buf2);
        free_lbuf(buf2);
#endif /* REALITY_LEVELS */

	mudconf.many_coins[0] = savec;

	/* print parent */

	loc = Parent(thing);
	if (loc != NOTHING) {
	    buf2 = unparse_object(player, loc, 0);
	    notify(player, unsafe_tprintf("%sParent:%s %s", 
                   ANSIEX(ANSI_HILITE), ANSIEX(ANSI_NORMAL), buf2));
	    free_lbuf(buf2);
	}
    }
    if ( mudconf.enable_tstamps && Controls(player, thing) ) {
       modbuf = atr_get(thing, A_MODIFY_TIME, &aowner2, &aflags2);
       a_chk = atr_num(A_MODIFY_TIME);
       if ( ((aflags2 & AF_INTERNAL) || (a_chk->flags & AF_INTERNAL)) ) {
          if ( *modbuf )
             notify(player, unsafe_tprintf("%sModifiedStamp:%s %s",
                            ANSIEX(ANSI_HILITE), ANSIEX(ANSI_NORMAL), modbuf));
          else
             notify(player, unsafe_tprintf("%sModifiedStamp:%s %s",
                            ANSIEX(ANSI_HILITE), ANSIEX(ANSI_NORMAL), "**NULL**"));
       }
       free_lbuf(modbuf);
       modbuf = atr_get(thing, A_CREATED_TIME, &aowner2, &aflags2);
       a_chk = atr_num(A_CREATED_TIME);
       if ( ((aflags2 & AF_INTERNAL) || (a_chk->flags & AF_INTERNAL)) ) {
          if ( *modbuf )
             notify(player, unsafe_tprintf("%sCreatedStamp:%s %s",
                            ANSIEX(ANSI_HILITE), ANSIEX(ANSI_NORMAL), modbuf));
          else
             notify(player, unsafe_tprintf("%sCreatedStamp:%s %s",
                            ANSIEX(ANSI_HILITE), ANSIEX(ANSI_NORMAL), "**NULL**"));
       }
       free_lbuf(modbuf);
    }
    if ( key != EXAM_BRIEF ) {
       look_atrs(player, thing, do_parent);
    }

    /* show him interesting stuff */

    if (control) {

	/* Contents */

	if (Contents(thing) != NOTHING) {
	    DOLIST(content, Contents(thing)) {
		buf2 = unparse_object(player, content, 0);
		if (!Cloak(content) || (Immortal(cause) || (Wizard(cause) && !(Immortal(content) && 
                    SCloak(content))))) {
                    if ( cntr == 0 ) {
	               notify(player, unsafe_tprintf("%sContents:%s", 
                              ANSIEX(ANSI_HILITE), ANSIEX(ANSI_NORMAL)));
                       cntr++;
                    }
		    notify(player, buf2);
                }
		free_lbuf(buf2);
	    }
	}
	/* Show stuff that depends on the object type */

	switch (Typeof(thing)) {
	case TYPE_ROOM:

	    /* tell him about exits */

	    if (Exits(thing) != NOTHING) {
		echeck = 0;
		DOLIST(exit, Exits(thing)) {
		    if (Dark(exit) && Unfindable(exit) && SCloak(exit) && 
			Immortal(Owner(exit)) && !Immortal(player)) continue;
		    if (Dark(exit) && Unfindable(exit) && Wizard(Owner(exit)) &&
			!Wizard(player)) continue;
		    if (!echeck) {
		      notify(player, unsafe_tprintf("%sExits:%s",
                        ANSIEX(ANSI_HILITE), ANSIEX(ANSI_NORMAL)));
		      echeck = 1;
		    }
		    buf2 = unparse_object(player, exit, 0);
		    notify(player, buf2);
		    free_lbuf(buf2);
		}
		if (!echeck)
		  notify(player, "No exits.");
	    } else {
		notify(player, "No exits.");
	    }

	    /* print dropto if present */

	    if (Dropto(thing) != NOTHING) {
		buf2 = unparse_object(player,
				      Dropto(thing), 0);
		notify(player,
		       unsafe_tprintf("%sDropped objects go to:%s %s",
			       ANSIEX(ANSI_HILITE), ANSIEX(ANSI_NORMAL), buf2));
		free_lbuf(buf2);
	    }
	    break;
	case TYPE_THING:
	case TYPE_PLAYER:

	    /* tell him about exits */

	    if (Exits(thing) != NOTHING) {
		echeck = 0;
		DOLIST(exit, Exits(thing)) {
		    if (Dark(exit) && Unfindable(exit) && SCloak(exit) && 
			Immortal(Owner(exit)) && !Immortal(player)) continue;
		    if (Dark(exit) && Unfindable(exit) && Wizard(Owner(exit)) &&
			!Wizard(player)) continue;
		    if (!echeck) {
		      notify(player, unsafe_tprintf("%sExits:%s",
                        ANSIEX(ANSI_HILITE), ANSIEX(ANSI_NORMAL)));
		      echeck = 1;
		    }
		    buf2 = unparse_object(player, exit, 0);
		    notify(player, buf2);
		    free_lbuf(buf2);
		}
		if (!echeck)
		  notify(player, "No exits.");
	    } else {
		notify(player, "No exits.");
	    }

	    /* print home */

	    loc = Home(thing);
	    buf2 = unparse_object(player, loc, 0);
	    notify(player, unsafe_tprintf("%sHome:%s %s", 
                   ANSIEX(ANSI_HILITE), ANSIEX(ANSI_NORMAL), buf2));
	    free_lbuf(buf2);

	    /* print location if player can link to it */

	    loc = Location(thing);
	    if ((loc == HOME) || ((loc != NOTHING) &&
		(Examinable(player, loc) ||
		 Examinable(player, thing) ||
		 Linkable(player, loc)))) {
		buf2 = unparse_object(player, loc, 0);
		notify(player, unsafe_tprintf("%sLocation:%s %s", 
                       ANSIEX(ANSI_HILITE), ANSIEX(ANSI_NORMAL), buf2));
		free_lbuf(buf2);
	    }
	    break;
	case TYPE_EXIT:
	    buf2 = unparse_object(player, Exits(thing), 0);
	    notify(player, unsafe_tprintf("%sSource:%s %s",
                   ANSIEX(ANSI_HILITE), ANSIEX(ANSI_NORMAL), buf2));
	    free_lbuf(buf2);

	    /* print destination */

	    switch (Location(thing)) {
	    case NOTHING:
		break;
	    case HOME:
		notify(player, unsafe_tprintf("%sDestination:%s *HOME*",
                       ANSIEX(ANSI_HILITE), ANSIEX(ANSI_NORMAL)));
		break;
	    default:
		buf2 = unparse_object(player,
				      Location(thing), 0);
		notify(player, unsafe_tprintf("%sDestination:%s %s", 
                       ANSIEX(ANSI_HILITE), ANSIEX(ANSI_NORMAL), buf2));
		free_lbuf(buf2);
		break;
	    }
	    break;
	default:
	    break;
	}
    } else if (!Opaque(thing) && nearby(player, thing)) {
	if (Has_contents(thing)) 
	    look_contents(player, thing, "Contents:");
	if (Typeof(thing) != TYPE_EXIT) {
	    look_exits(player, thing, "Obvious exits:", 0);
            look_exits(player, mudconf.master_room, "Global exits:", 1);
        }
    }
    free_lbuf(temp);

    if (!control) {
	if (mudconf.read_rem_name) {
	    buf2 = alloc_lbuf("do_examine.pub_name");
            sprintf(buf2, "%.3900s", Name(thing));
	    notify(player,
		   unsafe_tprintf("%s is owned by %s",
			   buf2, Name(Owner(thing))));
	    free_lbuf(buf2);
	} else {
	    notify(player,
		   unsafe_tprintf("Owned by %s",
			   Name(Owner(thing))));
	}
    }
    if ( mudstate.outputflushed ) {
       notify_quiet(player, "WARNING: Output limited exceeded on examine.  Attributes cut off.");
    }
}

void 
do_score(dbref player, dbref cause, int key)
{
    if (FreeFlag(player) &&
        (Typeof(player) == TYPE_PLAYER) &&
        !DePriv(player, NOTHING, DP_FREE, POWER6, POWER_LEVEL_NA))
	notify(player,
	       unsafe_tprintf("You have FREE (%d) %s.", Pennies(player), mudconf.many_coins));
    else if ((Builder(player)) &&
	!DePriv(player, NOTHING, DP_FREE, POWER6, POWER_LEVEL_NA))
	notify(player,
	       unsafe_tprintf("You have UNLIMITED %s.", mudconf.many_coins));
    else
	notify(player,
	       unsafe_tprintf("You have %d %s.", Pennies(player),
		       (Pennies(player) == 1) ?
		       mudconf.one_coin : mudconf.many_coins));
}

void
do_wielded(dbref player, dbref cause, int key)
{
    dbref thing, aowner;
    char *buff, *tbuff, *tpr_buff, *tprp_buff;
    int cntr=0, aflags;

    if ( !mudconf.alt_inventories ) {
        notify(player, "Alternate inventories have been disabled.");
        return;
    }
    thing = Contents(player);
    if (thing == NOTHING) {
	notify(player, "You aren't wielding anything.");
    } else {
        tprp_buff = tpr_buff = alloc_lbuf("do_wielded");
	DOLIST(thing, thing) {
            if (!Wieldable(thing))
               continue;
            if ( NoAnsiName(thing) || NoAnsiName(Owner(thing)) ||
               ((NoAnsiPlayer(player) || !(mudconf.allow_ansinames & ANSI_PLAYER)) && isPlayer(thing)) ||
               ((NoAnsiThing(player) || !(mudconf.allow_ansinames & ANSI_THING)) && isThing(thing)) ||
               ((NoAnsiExit(player) || !(mudconf.allow_ansinames & ANSI_EXIT)) && isExit(thing)) ||
               ((NoAnsiRoom(player) || !(mudconf.allow_ansinames & ANSI_ROOM)) && isRoom(thing)) )
               if (isPlayer(thing) || isThing(thing))
                  buff = unparse_object_altname(player, thing, 1);
               else
	          buff = unparse_object(player, thing, 1);
            else
               if (isPlayer(thing) || isThing(thing))
                  buff = unparse_object_ansi_altname(player, thing, 1);
               else
                  buff = unparse_object_ansi(player, thing, 1);

	    if (!Cloak(thing) || Immortal(player) || 
                 (Wizard(player) && !(Immortal(thing) && SCloak(thing)))) {
                if ( cntr == 0 ) {
	           notify(player, "You are wielding:");
                   cntr++;
                }
                tbuff = atr_pget(thing, A_INVTYPE, &aowner, &aflags);
                tprp_buff = tpr_buff;
                if ( !*tbuff )
		   notify(player, safe_tprintf(tpr_buff, &tprp_buff, "Miscellaneous: %s", buff));
                else
		   notify(player, safe_tprintf(tpr_buff, &tprp_buff, "%.30s: %s", tbuff, buff));
                free_lbuf(tbuff);
            }
	    free_lbuf(buff);
	}
        free_lbuf(tpr_buff);
        if ( cntr == 0 )
	   notify(player, "You aren't wielding anything.");
    }
}
void
do_worn(dbref player, dbref cause, int key)
{
    dbref thing, aowner;
    char *buff, *tbuff, *tpr_buff, *tprp_buff;
    int cntr=0, aflags;

    if ( !mudconf.alt_inventories ) {
        notify(player, "Alternate inventories have been disabled.");
        return;
    }
    thing = Contents(player);
    if (thing == NOTHING) {
	notify(player, "You aren't wearing anything.");
    } else {
        tprp_buff = tpr_buff = alloc_lbuf("do_worn");
	DOLIST(thing, thing) {
            if (!Wearable(thing))
               continue;
            if ( NoAnsiName(thing) || NoAnsiName(Owner(thing)) ||
               ((NoAnsiPlayer(player) || !(mudconf.allow_ansinames & ANSI_PLAYER)) && isPlayer(thing)) ||
               ((NoAnsiThing(player) || !(mudconf.allow_ansinames & ANSI_THING)) && isThing(thing)) ||
               ((NoAnsiExit(player) || !(mudconf.allow_ansinames & ANSI_EXIT)) && isExit(thing)) ||
               ((NoAnsiRoom(player) || !(mudconf.allow_ansinames & ANSI_ROOM)) && isRoom(thing)) )
               if (isPlayer(thing) || isThing(thing))
                  buff = unparse_object_altname(player, thing, 1);
               else
	          buff = unparse_object(player, thing, 1);
            else
               if (isPlayer(thing) || isThing(thing))
                  buff = unparse_object_ansi_altname(player, thing, 1);
               else
                  buff = unparse_object_ansi(player, thing, 1);

	    if (!Cloak(thing) || Immortal(player) || 
                 (Wizard(player) && !(Immortal(thing) && SCloak(thing)))) {
                if ( cntr == 0 ) {
	           notify(player, "You are wearing:");
                   cntr++;
                }
                tbuff = atr_pget(thing, A_INVTYPE, &aowner, &aflags);
                tprp_buff = tpr_buff;
                if ( !*tbuff )
		   notify(player, safe_tprintf(tpr_buff, &tprp_buff, "Miscellaneous: %s", buff));
                else
		   notify(player, safe_tprintf(tpr_buff, &tprp_buff, "%.30s: %s", tbuff, buff));
                free_lbuf(tbuff);
            }
	    free_lbuf(buff);
	}
        free_lbuf(tpr_buff);
        if ( cntr == 0 )
	   notify(player, "You aren't wearing anything.");
    }
}

void 
do_inventory(dbref player, dbref cause, int key)
{
    dbref thing;
    char *buff, *s, *e, *buf2, invname[80];
    int cntr=0, wearcnt=0, wieldcnt=0;

    thing = Contents(player);
    if ( ! *(mudconf.invname) )
       strcpy(invname, "backpack");
    else
       strcpy(invname, strip_returntab(strip_ansi(mudconf.invname),3));

    if (thing == NOTHING) {
        if ( mudconf.alt_inventories && mudconf.altover_inv )
           notify(player, unsafe_tprintf("You are not carrying anything in your %s.", invname));
        else
	   notify(player, "You are not carrying anything.");
    } else {
	DOLIST(thing, thing) {
            if ( mudconf.alt_inventories && mudconf.altover_inv && 
                 (Wearable(thing) || Wieldable(thing)) ) {
	       if (!Cloak(thing) || Immortal(player) || 
                    (Wizard(player) && !(Immortal(thing) && SCloak(thing)))) {
                  if (Wearable(thing)) 
                     wearcnt++;
                  if (Wieldable(thing)) 
                     wieldcnt++;
               }
               continue;
            }
#ifdef REALITY_LEVELS
            if (!IsReal(player, thing))
               continue;
#endif /* REALITY_LEVELS */
            if ( NoAnsiName(thing) || NoAnsiName(Owner(thing)) ||
               ((NoAnsiPlayer(player) || !(mudconf.allow_ansinames & ANSI_PLAYER)) && isPlayer(thing)) ||
               ((NoAnsiThing(player) || !(mudconf.allow_ansinames & ANSI_THING)) && isThing(thing)) ||
               ((NoAnsiExit(player) || !(mudconf.allow_ansinames & ANSI_EXIT)) && isExit(thing)) ||
               ((NoAnsiRoom(player) || !(mudconf.allow_ansinames & ANSI_ROOM)) && isRoom(thing)) )
               if (isPlayer(thing) || isThing(thing))
                  buff = unparse_object_altname(player, thing, 1);
               else
	          buff = unparse_object(player, thing, 1);
            else
               if (isPlayer(thing) || isThing(thing))
                  buff = unparse_object_ansi_altname(player, thing, 1);
               else
                  buff = unparse_object_ansi(player, thing, 1);

	    if (!Cloak(thing) || Immortal(player) || 
                 (Wizard(player) && !(Immortal(thing) && SCloak(thing)))) {
                if ( cntr == 0 ) {
	           notify(player, "You are carrying:");
                   cntr++;
                }
		notify(player, buff);
            }
	    free_lbuf(buff);
	}
        if ( cntr == 0 ) {
           if ( mudconf.alt_inventories && mudconf.altover_inv )
	     notify(player, unsafe_tprintf("You are not carrying anything in your %s.",invname));
           else
	     notify(player, "You aren't carrying anything.");
	}
    }

    thing = Exits(player);
    if (thing != NOTHING) {
	e = buff = alloc_lbuf("look_exits");
	DOLIST(thing, thing) {
	    /* chop off first exit alias to display */
	    if (!Cloak(thing) || Immortal(player) || 
                 (Wizard(player) && !(Immortal(thing) && SCloak(thing)))) {
                if ( cntr == 0 ) {
	           notify(player, "Exits:");
                   cntr++;
                }
#ifdef REALITY_LEVELS
                if (!IsReal(player, thing))
                   continue;
#endif /* REALITY_LEVELS */

                buf2 = ansi_exitname(thing);
                if ( isExit(thing) && !NoAnsiExit(player) && (mudconf.allow_ansinames & ANSI_EXIT) &&
                     !NoAnsiName(thing) && !NoAnsiName(Owner(thing)) )
                   safe_str(buf2, buff, &e);
                free_lbuf(buf2);
	        for (s = Name(thing); *s && (*s != ';'); s++) 
	    	   safe_chr(*s, buff, &e);
                if ( isExit(thing) && !NoAnsiExit(player) && (mudconf.allow_ansinames & ANSI_EXIT) &&
                     !NoAnsiName(thing) && !NoAnsiName(Owner(thing)) )
                   safe_str(ANSI_NORMAL, buff, &e);
	        safe_str((char *) "  ", buff, &e);
           }
	}
	*e = 0;
        if ( cntr > 0 )
	   notify(player, buff);
	free_lbuf(buff);
    }
    do_score(player, player, 0);
    if ( mudconf.alt_inventories && mudconf.altover_inv ) {
       if (wieldcnt > 0 )
          notify(player, unsafe_tprintf("Currently Wielded: %d ite%s not displayed in inventory.", 
                 wieldcnt, wieldcnt > 1 ? "ms" : "m"));
       if (wearcnt > 0 )
          notify(player, unsafe_tprintf("Currently Worn: %d ite%s not displayed in inventory.", 
                 wearcnt, wearcnt > 1 ? "ms" : "m"));
    }
    
}

void 
do_entrances(dbref player, dbref cause, int key, char *name)
{
    dbref thing, i, j;
    char *exit, *message, *tpr_buff, *tprp_buff;
    int control_thing, count, low_bound, high_bound;
    FWDLIST *fp;

    parse_range(&name, &low_bound, &high_bound);
    if (!name || !*name) {
	if (Has_location(player))
	    thing = Location(player);
	else
	    thing = player;
	if (!Good_obj(thing))
	    return;
    } else {
	init_match(player, name, NOTYPE);
	match_everything(MAT_EXIT_PARENTS);
	thing = noisy_match_result();
	if (!Good_obj(thing))
	    return;
    }

    if (!payfor(player, mudconf.searchcost)) {
	notify(player,
	       unsafe_tprintf("You don't have enough %s.",
		       mudconf.many_coins));
	return;
    }
    message = alloc_lbuf("do_entrances");
    control_thing = Examinable(player, thing);
    count = 0;
    tprp_buff = tpr_buff = alloc_lbuf("do_entrances");
    for (i = low_bound; i <= high_bound; i++) {
	if (control_thing || Examinable(player, i)) {
	    switch (Typeof(i)) {
	    case TYPE_EXIT:
		if (Location(i) == thing) {
		    exit = unparse_object(player,
					  Exits(i), 0);
                    tprp_buff = tpr_buff;
		    notify(player,
			   safe_tprintf(tpr_buff, &tprp_buff, "%s (%s)", exit, Name(i)));
		    free_lbuf(exit);
		    count++;
		}
		break;
	    case TYPE_ROOM:
		if (Dropto(i) == thing) {
		    exit = unparse_object(player, i, 0);
                    tprp_buff = tpr_buff;
		    notify(player,
			   safe_tprintf(tpr_buff, &tprp_buff, "%s [dropto]", exit));
		    free_lbuf(exit);
		    count++;
		}
		break;
	    case TYPE_THING:
	    case TYPE_PLAYER:
		if (Home(i) == thing) {
		    exit = unparse_object(player, i, 0);
                    tprp_buff = tpr_buff;
		    notify(player,
			   safe_tprintf(tpr_buff, &tprp_buff, "%s [home]", exit));
		    free_lbuf(exit);
		    count++;
		}
		break;
	    }

	    /* Check for parents */

	    if (Parent(i) == thing) {
		exit = unparse_object(player, i, 0);
                tprp_buff = tpr_buff;
		notify(player,
		       safe_tprintf(tpr_buff, &tprp_buff, "%s [parent]", exit));
		free_lbuf(exit);
		count++;
	    }
	    /* Check for forwarding */

	    if (H_Fwdlist(i)) {
		fp = fwdlist_get(i);
		if (!fp)
		    continue;
		for (j = 0; j < fp->count; j++) {
		    if (fp->data[j] != thing)
			continue;
		    exit = unparse_object(player, i, 0);
                    tprp_buff = tpr_buff;
		    notify(player,
			   safe_tprintf(tpr_buff, &tprp_buff, "%s [forward]", exit));
		    free_lbuf(exit);
		    count++;
		}
	    }
	}
    }
    free_lbuf(tpr_buff);
    free_lbuf(message);
    notify(player, unsafe_tprintf("%d entrance%s found.", count,
			   (count == 1) ? "" : "s"));
}

/* check the current location for bugs */

static void 
sweep_check(dbref player, dbref what, int key, int is_loc)
{
    dbref aowner, parent, what2;
    int canhear, cancom, isplayer, ispuppet, isconnected, attr, aflags;
    int is_parent, lev;
    char *buf, *buf2, *bp, *as, *buff, *s, *tpr_buff, *tprp_buff;
    ATTR *ap;

    canhear = 0;
    cancom = 0;
    isplayer = 0;
    ispuppet = 0;
    isconnected = 0;
    is_parent = 0;

    if ( !Good_obj(what) ) 
       return;

    if ((key & SWEEP_LISTEN) &&
	(((Typeof(what) == TYPE_EXIT) || is_loc) && Audible(what))) {
	canhear = 1;
    } else if (key & SWEEP_LISTEN) {
	if ( H_Listen(what) && Bouncer(what))
	    canhear = 1;
	if (Monitor(what))
	    buff = alloc_lbuf("Hearer");
	else
	    buff = NULL;

	for (attr = atr_head(what, &as); attr; attr = atr_next(&as)) {
	    if (attr == A_LISTEN) {
		canhear = 1;
		break;
	    }
	    if (Monitor(what)) {
		ap = atr_num(attr);
		if (!ap || (ap->flags & AF_NOPROG))
		    continue;

		atr_get_str(buff, what, attr, &aowner,
			    &aflags);

		/* Make sure we can execute it */

		if ((buff[0] != AMATCH_LISTEN) ||
		    (aflags & AF_NOPROG))
		    continue;

		/* Make sure there's a : in it */

		for (s = buff + 1; *s && (*s != ':'); s++);
		if (s) {
		    canhear = 1;
		    break;
		}
	    }
	}
	if (buff)
	    free_lbuf(buff);
    }
    if ((key & SWEEP_COMMANDS) && (Typeof(what) != TYPE_EXIT)) {

	/* Look for commands on the object and parents too */

	ITER_PARENTS(what, parent, lev) {
	    if (Commer(parent)) {
		cancom = 1;
		if (lev) {
		    is_parent = 1;
		    break;
		}
	    }
	}
    }
    if (key & SWEEP_CONNECT) {
	if (Connected(what) ||
	    (Puppet(what) && Connected(Owner(what))) ||
	    (mudconf.player_listen && (Typeof(what) == TYPE_PLAYER) &&
	     canhear && Connected(Owner(what))))
	    isconnected = 1;
    }
    if (key & SWEEP_PLAYER || isconnected) {
	if (Typeof(what) == TYPE_PLAYER)
	    isplayer = 1;
	if (Puppet(what))
	    ispuppet = 1;
    }
    if (canhear || cancom || isplayer || ispuppet || isconnected) {
	buf = alloc_lbuf("sweep_check.types");
	bp = buf;

	if (cancom)
	    safe_str((char *) "commands ", buf, &bp);
	if (canhear)
	    safe_str((char *) "messages ", buf, &bp);
	if (isplayer)
	    safe_str((char *) "player ", buf, &bp);
	if (ispuppet) {
	    safe_str((char *) "puppet(", buf, &bp);
	    safe_str(Name(Owner(what)), buf, &bp);
	    safe_str((char *) ") ", buf, &bp);
	}
	if (isconnected)
	    safe_str((char *) "connected ", buf, &bp);
	if (is_parent)
	    safe_str((char *) "parent ", buf, &bp);
	if (Cloak(what))
	    what2 = what;
	else
	    what2 = Owner(what);
	if (Cloak(what2))
	    safe_str((char *) "cloaked ", buf, &bp);
	bp[-1] = '\0';
	if (!Cloak(what2) || (Cloak(what2) &&
			      ((!Immortal(what2) && (Wizard(player)) && (!SCloak(what2))) || Immortal(player)))) {
            tprp_buff = tpr_buff = alloc_lbuf("sweep_check");
	    if (Typeof(what) != TYPE_EXIT) {
		notify(player,
		       safe_tprintf(tpr_buff, &tprp_buff, "  %s is listening. [%s]",
			       Name(what), buf));
	    } else {
		buf2 = alloc_lbuf("sweep_check.name");
		sprintf(buf2, "%.3900s", Name(what));
		for (bp = buf2; *bp && (*bp != ';'); bp++);
		*bp = '\0';
		notify(player,
		       safe_tprintf(tpr_buff, &tprp_buff, "  %s is listening. [%s]", buf2, buf));
		free_lbuf(buf2);
	    }
            free_lbuf(tpr_buff);
	}
	free_lbuf(buf);
    }
}

void 
do_sweep(dbref player, dbref cause, int key, char *where)
{
    dbref here, sweeploc, start;
    int where_key, what_key;

    where_key = key & (SWEEP_ME | SWEEP_HERE | SWEEP_EXITS);
    what_key =
	key & (SWEEP_COMMANDS | SWEEP_LISTEN | SWEEP_PLAYER | SWEEP_CONNECT);

    if (where && *where) {
	sweeploc = match_controlled(player, where);
	if (!Good_obj(sweeploc))
	    return;
    } else {
	sweeploc = player;
    }

    if (!where_key)
	where_key = -1;
    if (!what_key)
	what_key = -1;
    else if (what_key == SWEEP_VERBOSE)
	what_key = SWEEP_VERBOSE | SWEEP_COMMANDS;

    /* Check my location.  If I have none or it is dark, check just me. */

    if (where_key & SWEEP_HERE) {
	notify(player, "Sweeping location...");
	if (Has_location(sweeploc)) {
	    here = Location(sweeploc);
	    if (!Good_obj(here) ||
		(Dark(here) && !mudconf.sweep_dark &&
		 !Examinable(player, here))) {
		notify_quiet(player,
		    "Sorry, it is dark here and you can't search for bugs");
		sweep_check(player, sweeploc, what_key, 0);
	    } else {
		sweep_check(player, here, what_key, 1);
		for (here = Contents(here); here != NOTHING; here = Next(here))
		    sweep_check(player, here, what_key, 0);
	    }
	} else {
	    sweep_check(player, sweeploc, what_key, 0);
	}
    }
    /* Check exits in my location */

    if ((where_key & SWEEP_EXITS) && Has_location(sweeploc)) {
	notify(player, "Sweeping exits...");
	start = Location(sweeploc);
	if (Good_obj(start)) {
	  for (here = Exits(start); here != NOTHING; here = Next(here))
	    sweep_check(player, here, what_key, 0);
	}
    }
    /* Check my inventory */

    if ((where_key & SWEEP_ME) && Has_contents(sweeploc)) {
	notify(player, "Sweeping inventory...");
	for (here = Contents(sweeploc); here != NOTHING; here = Next(here))
	    sweep_check(player, here, what_key, 0);
    }
    /* Check carried exits */

    if ((where_key & SWEEP_EXITS) && Has_exits(sweeploc)) {
	notify(player, "Sweeping carried exits...");
	for (here = Exits(sweeploc); here != NOTHING; here = Next(here))
	    sweep_check(player, here, what_key, 0);
    }
    notify(player, "Sweep complete.");
}

void 
do_whereall(dbref player, dbref cause, int key)
{
    DESC *d;
    char temp[1024], buf[1024], *membuff;
    int i, j;

    if (!payfor(player, Guest(player) ? 0 : mudconf.pagecost)) {
	notify(player, unsafe_tprintf("You don't have enough %s.", mudconf.many_coins));
	return;
    }
    DESC_ITER_CONN(d) {
	if ((((Findable(d->player) && mudconf.who_unfindable) || 
            !(Dark(d->player) && !mudconf.who_unfindable && !mudconf.player_dark && !Admin(player)) ||
            (!mudconf.who_unfindable && mudconf.player_dark)) &&
              !Cloak(d->player)) || Immortal(player) || (Wizard(player) && !Immortal(d->player))) {

	    if ( (Hidden(d->player) || Unfindable(d->player)) && !Immortal(player) && !(Wizard(player) && Wizard(d->player)))
		sprintf(buf, "%.100s wishes to have some privacy.", Name(d->player));
	    else {
		membuff = unparse_object(player, Location(d->player), 1);
		sprintf(buf, "%.100s is at the %.900s.", Name(d->player), membuff);
		free_lbuf(membuff);
	    }

	    i = 79 - strlen(buf);
	    i = i / 2;
	    if (i <= 0) {
		notify(player, buf);
	    } else {
		strcpy(&temp[i], buf);
		for (j = 0; j < i; j++)
		    temp[j] = ' ';
		notify(player, temp);
	    }
	}
    }
}

void 
do_whereis(dbref player, dbref cause, int key, char *name)
{
    dbref thing;
    char *buff;

    if (!name || !*name) {
	notify(player, "You must specify a valid player name.");
	return;
    }
    if ((thing = lookup_player(player, name, 1)) == NOTHING) {
	notify(player, "That player does not seem to exist.");
	return;
    }
    if (!(Connected(thing)) || (Immortal(thing) && Cloak(thing) && !Immortal(player) && SCloak(thing)) ||
	(!Wizard(player) && Cloak(thing)) || (Dark(thing) && !mudconf.who_unfindable && !mudconf.player_dark &&
         !Admin(player))) {
	notify(player, "That player is not currently logged on.");
	notify(thing, unsafe_tprintf("%s tried to locate you and was told your not logged on.",
			      Name(player)));
	return;
    }
    /*
     * init_match(player, name, TYPE_PLAYER); match_player(); match_exit();
     * match_neighbor(); match_possession(); match_absolute();      match_me();
     */

    if (!Wizard(player) && Unfindable(thing)) {
	notify(player, "That player wishes to have some privacy.");
	if (mudconf.whereis_notify && !Haven(player))
	    notify(thing, unsafe_tprintf("%s tried to locate you and failed.",
				  Name(player)));
	return;
    }
    buff = unparse_object(player, Location(thing), 1);
    notify(player, unsafe_tprintf("%s is in %s.", Name(thing), buff));
    if (mudconf.whereis_notify && !Haven(player)) {
        if ( Cloak(player) ) {
            if ( Controls(thing,player) )
	        notify(thing, unsafe_tprintf("%s has just located your position.", Name(player)));
        } else {
	    notify(thing, unsafe_tprintf("%s has just located your position.", Name(player)));
        }
    }
    free_lbuf(buff);
    return;
}

/* Output the sequence of commands needed to duplicate the specified
   object.  If you're moving things to another system, your milage
   will almost certainly vary.  (i.e. different flags, etc.)
 */

extern NAMETAB indiv_attraccess_nametab[];

static void
decomp_wildattrs(dbref player, dbref thing, OBLOCKMASTER * master, char *newname)
{
  int atr, aflags, got_any;
  char *buf, *ltext, *buff2, *tname, *tpr_buff, *tprp_buff;
  dbref aowner;
  ATTR *ap;
  BOOLEXP *bool;
  NAMETAB *np;

  got_any = 0;
  buff2 = alloc_mbuf("do_decomp.attr_name");
  tname = alloc_lbuf("decomp_wild");
  memset(tname, 0, sizeof(tname));
  memset(buff2, 0, sizeof(buff2));
  if (newname && *newname)
    strncpy(tname, newname, (LBUF_SIZE - 1));
  else
    strncpy(tname, Name(thing), (LBUF_SIZE - 1));
  tprp_buff = tpr_buff = alloc_lbuf("decomp_wildattrs");
  for (atr = olist_first(master); atr != NOTHING; atr = olist_next(master)) {
    ap = atr_num(atr);
    if (!ap)
      continue;
    buf = atr_get(thing, atr, &aowner, &aflags);
    if (Read_attr(player, thing, ap, aowner, aflags, 0)) {
      if (attr->flags & AF_IS_LOCK) {
        bool = parse_boolexp(player, buf, 1);
        ltext = unparse_boolexp_decompile(player, bool);
	free_boolexp(bool);
        tprp_buff = tpr_buff;
	noansi_notify(player,
		      safe_tprintf(tpr_buff, &tprp_buff, "@lock/%s %s=%s",
			           ap->name, tname, ltext));
      } else {
        strncpy(buff2, ap->name, (MBUF_SIZE - 1));
        tprp_buff = tpr_buff;
        raw_notify(player, safe_tprintf(tpr_buff, &tprp_buff, "%c%s %.3000s=",
                            ((ap->number < A_USER_START) ? '@' : '&'), buff2, tname), 0, 0);
        noansi_notify(player, buf);
	if (aflags & AF_LOCK) {
            tprp_buff = tpr_buff;
	    noansi_notify(player, safe_tprintf(tpr_buff, &tprp_buff, "@lock %s/%s",
				               tname, buff2));
	}
	for (np = indiv_attraccess_nametab;
	     np->name;
	     np++) {

	    if ((aflags & np->flag) &&
		check_access(player, np->perm, np->perm2, 0) &&
		(!(np->perm & CA_NO_DECOMP))) {

                tprp_buff = tpr_buff;
		noansi_notify(player,
		       safe_tprintf(tpr_buff, &tprp_buff, "@set %s/%s = %s",
			       tname,
			       buff2,
			       np->name));
	    }
	}
      }
    }
    free_lbuf(buf);
  }
  free_lbuf(tpr_buff);
  free_lbuf(tname);
  free_mbuf(buff2);
}

void 
do_decomp(dbref player, dbref cause, int key, char *name, char *qual)
{
    BOOLEXP *bool;
    char *got, *thingname, *as, *ltext, *buff, *tpr_buff, *tprp_buff;
    dbref aowner, thing;
    int val, aflags, ca, key_buff;
    ATTR *attr;
    NAMETAB *np;
    OBLOCKMASTER master;

    key_buff = key;
    key = 0;
    mudstate.outputflushed = 0;
    thing = NOTHING;
    if (!name || !*name)
      return;
    olist_init(&master);
    if (parse_attrib_wild(player, name, &thing, 0, 1, 0, &master, 0)) {
      if (!Examinable(player, thing)) {
	notify_quiet(player, "You can only decompile things you can examine.");
	olist_cleanup(&master);
	return;
      }
      decomp_wildattrs(player, thing, &master, qual);
      olist_cleanup(&master);
      if ( mudstate.outputflushed ) {
         notify_quiet(player, "WARNING: Output limited exceeded on @decompile.  Attributes cut off.");
      }
      return;
    }
    olist_cleanup(&master);
    init_match(player, name, TYPE_THING);
    match_everything(MAT_EXIT_PARENTS);
    thing = noisy_match_result();

    /* get result */
    if (thing == NOTHING)
	return;

    if (!Examinable(player, thing)) {
	notify_quiet(player,
		     "You can only decompile things you can examine.");
	return;
    }
    thingname = atr_get(thing, A_LOCK, &aowner, &aflags);
    bool = parse_boolexp(player, thingname, 1);

    /* Determine the name of the thing to use in reporting and then
     * report the command to make the thing.
     */

    if (qual && *qual) {
	strncpy(thingname, qual, LBUF_SIZE - 1);
        *(thingname + LBUF_SIZE - 1) = '\0';
    } else {
	switch (Typeof(thing)) {
	case TYPE_THING:
	    strncpy(thingname, Name(thing), LBUF_SIZE - 1);
            *(thingname + LBUF_SIZE - 1) = '\0';
	    val = OBJECT_DEPOSIT(Pennies(thing));
            if ( !key )
	       noansi_notify(player, unsafe_tprintf("@create %s=%d", thingname, val));
	    break;
	case TYPE_ROOM:
	    strcpy(thingname, "here");
            if ( !key )
	       noansi_notify(player, unsafe_tprintf("@dig/teleport %s", Name(thing)));
	    break;
	case TYPE_EXIT:
	    strncpy(thingname, Name(thing), LBUF_SIZE - 1);
            *(thingname + LBUF_SIZE - 1) = '\0';
            if ( !key )
	       noansi_notify(player, unsafe_tprintf("@open %s", Name(thing)));
	    for (got = thingname; *got; got++) {
		if (*got == EXIT_DELIMITER) {
		    *got = '\0';
		    break;
		}
	    }
	    break;
	case TYPE_PLAYER:
	    strcpy(thingname, "me");
	    break;
	}
    }

    /* Report the lock (if any) */

    if ( !key_buff || (key_buff & DECOMP_ATTRS) ) {
       if (bool != TRUE_BOOLEXP) {
          noansi_notify(player, unsafe_tprintf("@lock %s=%s", thingname,
                                         unparse_boolexp_decompile(player, bool)));
       }
    }
    free_boolexp(bool);

    /* Report attributes */

    buff = alloc_mbuf("do_decomp.attr_name");
    tprp_buff = tpr_buff = alloc_lbuf("do_decomp");
    if ( !key_buff || (key_buff & DECOMP_ATTRS)) {
       for (ca = atr_head(thing, &as); ca; ca = atr_next(&as)) {
	   if ((ca == A_NAME) || (ca == A_LOCK))
	       continue;
	   attr = atr_num(ca);
	   if (!attr)
	       continue;
	   if ((attr->flags & AF_NOCMD) && !(attr->flags & AF_IS_LOCK))
	       continue;
   
	   got = atr_get(thing, ca, &aowner, &aflags);
	   if (Read_attr(player, thing, attr, aowner, aflags, 0)) {
	       if (!Wizard(player) && ((attr->flags & AF_PINVIS) || (aflags & AF_PINVIS))) {
	         free_lbuf(got);
	         continue;
	       }
	       if (attr->flags & AF_IS_LOCK) {
		   bool = parse_boolexp(player, got, 1);
		   ltext = unparse_boolexp_decompile(player,
						     bool);
		   free_boolexp(bool);
		   tprp_buff = tpr_buff;
		   noansi_notify(player,
		          safe_tprintf(tpr_buff, &tprp_buff, "@lock/%s %s=%s",
			          attr->name, thingname, ltext));
	       } else {
                   strncpy(buff, attr->name, (MBUF_SIZE - 1));
                   *(buff + MBUF_SIZE - 1) = '\0';
		   tprp_buff = tpr_buff;
		   noansi_notify(player,
		          safe_tprintf(tpr_buff, &tprp_buff, "%c%s %s=%s",
			          ((ca < A_USER_START) ?
				   '@' : '&'),
			          buff, thingname, got));
   
		   if (aflags & AF_LOCK) {
		       tprp_buff = tpr_buff;
		       noansi_notify(player, safe_tprintf(tpr_buff, &tprp_buff, "@lock %s/%s",
					      thingname, buff));
		   }
		   for (np = indiv_attraccess_nametab;
		        np->name;
		        np++) {
   
		       if ((aflags & np->flag) &&
			   check_access(player, np->perm, np->perm2, 0) &&
			   (!(np->perm & CA_NO_DECOMP))) {
   
		           tprp_buff = tpr_buff;
			   noansi_notify(player,
			          safe_tprintf(tpr_buff, &tprp_buff, "@set %s/%s = %s",
				          thingname,
				          buff,
				          np->name));
		       }
		   }
	       }
	   }
   	   free_lbuf(got);
       }
    }
    free_lbuf(tpr_buff);
    free_mbuf(buff);

#ifdef REALITY_LEVELS
    if ( !key_buff || (key_buff & DECOMP_ATTRS) )
       decompile_rlevels(player, thing, thingname);
#endif /* REALITY_LEVELS */

    if ( !key_buff || (key_buff & DECOMP_FLAGS) )
       decompile_flags(player, thing, thingname);

    /* If the object has a parent, report it */

    if ( (!key_buff || (key_buff & DECOMP_ATTRS)) && (Parent(thing) != NOTHING) )
	noansi_notify(player, unsafe_tprintf("@parent %s=%s", thingname, Name(Parent(thing))));

    if ( mudstate.outputflushed ) {
        notify_quiet(player, "WARNING: Output limited exceeded on @decompile.  Attributes cut off.");
    }

    free_lbuf(thingname);
}
