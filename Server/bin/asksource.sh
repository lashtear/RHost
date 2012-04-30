#!/bin/sh
###################################################################
# Test shell capabilities
###################################################################
TST[1]=test
if [ $? -ne 0 ]
then
   echo "/bin/sh does not accept array variables."
   echo "please run the ./bin/script_setup.sh script in attempts to fix this."
   echo "You must have a KSH or BASH compatable shell."
   exit 1
fi
###################################################################
# Fix for save
###################################################################
if [ -f asksource.save ]
then
   mv -f asksource.save asksource.save0
fi
###################################################################
# Environments
###################################################################
BETAOPT=0
DEFS="-Wall"
DATE="$(date +"%m%d%y")"
MORELIBS=""
OPTIONS="1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23"
C_OPTIONS=$(echo $OPTIONS|wc -w)
BOPTIONS="1 2 3"
C_BOPTIONS=$(echo $BOPTIONS|wc -w)
DOPTIONS="1 2 3"
C_DOPTIONS=$(echo $DOPTIONS|wc -w)
REPEAT=1
for i in ${OPTIONS}
do
   X[${i}]=" "
done
for i in ${BOPTIONS}
do
   XB[${i}]=" "
done
for i in ${DOPTIONS}
do
   XD[${i}]=" "
done
# Load default options
if [ -f ./asksource.default ]
then
   . ./asksource.default
fi
DEF[1]="-DUSE_SIDEEFFECT"
DEF[2]="-DTINY_U"
DEF[3]="-DMUX_INCDEC"
DEF[4]="-DSOFTCOM"
DEF[5]="-DTINY_SUB"
DEF[6]="-DUSECRYPT"
DEF[7]="-DPLUSHELP"
DEF[8]="-DPROG_LIKEMUX"
DEF[9]="-DENABLE_COMMAND_FLAG"
DEF[10]="-DATTR_HACK"
DEF[11]="-DREALITY_LEVELS"
DEF[12]="-DEXPANDED_QREGS"
DEF[13]="-DZENTY_ANSI"
DEF[14]="-DMARKER_FLAGS"
DEF[15]="-DBANGS"
DEF[16]="-DPARIS"
DEF[17]="-DOLD_SETQ"
DEF[18]="-DSECURE_SIDEEFFECT"
DEF[19]="-DNODEBUGMONITOR"
DEF[20]="-DIGNORE_SIGNALS"
DEF[21]="-DOLD_REALITIES"
DEF[22]="-DMUXCRYPT"
DEFB[1]="\$(MYSQL_DEFS)"
DEFB[2]="\$(DR_DEF)"
DEFB[3]="-DSBUF64"
DEFD[1]="-DMUSH_DOORS"
DEFD[2]="-DEMPIRE_DOORS"
DEFD[3]="-DPOP3_DOORS"
###################################################################
# MENU - Main Menu for RhostMUSH Configuration Utility
###################################################################
doormenu() {
clear
echo "                    RhostMUSH Door Source Configuration Utility"
echo "------------------------------------------------------------------------------"
echo ""
echo "[${XD[1]}]   1. Mush Doors         [${XD[2]}]  2. Empire Doors       [${XD[3]}]  3. POP3 Doors"
echo ""
echo "------------------------------------------------------------------------------"
echo "[Q]   Go Back to Previous Menu"
echo "------------------------------------------------------------------------------"
echo ""
echo "Keys: [h]elp [i]nfo"
echo "      Or, you may select a number to toggle on/off"
echo ""
echo "Please Enter selection: "|tr -d '\012'
}

menu() {
clear
echo "                       RhostMUSH Source Configuration Utility"
echo ""
echo "------------------------------------------------------------------------------"
echo "[${X[1]}]  1. Sideeffects        [${X[2]}]  2. MUSH/MUX u()/zfun  [${X[3]}]  3. MUX inc()/dec()"
echo "[${X[4]}]  4. Disabled Comsys    [${X[5]}]  5. ansi %x not %c     [${X[6]}]  6. crypt()/decrypt()"
echo "[${X[7]}]  7. +help hardcoded    [${X[8]}]  8. MUX @program       [${X[9]}]  9. COMMAND flag"
echo "[${X[10]}] 10. ~/_ attributes     [${X[11]}] 11. Reality Levels     [${X[12]}] 12. a-z setq support"
echo "[${X[13]}] 13. Enhanced ANSI      [${X[14]}] 14. Marker Flags       [${X[15]}] 15. Bang support"
echo "[${X[16]}] 16. Alternate WHO      [${X[17]}] 17. Old SETQ/SETR      [${X[18]}] 18. Secured Sideeffects"
echo "[${X[19]}] 19. Disable DebugMon   [${X[20]}] 20. Disable SIGNALS    [${X[21]}] 21. Old Reality Lvls" 
echo "[${X[22]}] 22. Read Mux Passwds   [${X[23]}] 23. Low-Mem Compile"
echo "--------------------------- Beta/Unsupported Additions -----------------------"
echo "[${XB[1]}] B1. 3rd Party MySQL    [${XB[2]}] B2. Door Support(Menu) [${XB[3]}] B3. 64 Char attribs"
echo "------------------------------------------------------------------------------"
echo ""
echo "Keys: [h]elp [i]nfo [s]ave [l]oad [d]elete [c]lear [m]ark [b]rowse [r]un [q]uit"
echo "      [x]tra default cores (MUX, TinyMUSH3, Penn, Rhost-Default)"
echo "      Or, you may select a numer to toggle on/off"
echo ""
echo "Please Enter selection: "|tr -d '\012'
}

###################################################################
# HELP - Main help interface for configuration utility
###################################################################
help() {
echo ""
echo "The following options are available..."
echo "h - help   : This shows this screen"
echo "i - info <option> : This gives information on the specified numeric toggle."
if [ $BETAOPT -eq 0 ]
then
   echo "s - save   : This optionally saves your configuration to one of 4 slots."
   echo "l - load   : This loads the options that you previously saved from a slot."
   echo "r - run    : This 'runs' the current configuration as selected."
   echo "d - delete : This deletes the saved configuration file."
   echo "m - mark   : This labels the saved file with a specified marker"
   echo "b - browse : This will allow you to browse your config files"
   echo "x - extras : This will allow you to load optional default configurations."
   echo "q - quit   : This quits/aborts the configurator without writing options."
fi
echo ""
echo "----------------------------------------------------------------------------"
echo "The selection screen allows you to toggle any option with [ ].  You may also"
echo "type 'i' for info which will give you detailed information of the number of"
echo "choice.  For example: i 15   (this will give you information on number 15)"
echo "----------------------------------------------------------------------------"
}

###################################################################
# INFO <arg> - Information for all the toggle options available
###################################################################
info() {
   if [ $BETAOPT -eq 1 ]
   then
      CNTCHK=$(expr $1 + 0 2>/dev/null)
      if [ -z "$CNTCHK" -o -z "$1" ]
      then
         INFOARG=0
      elif [ "$CNTCHK" -lt 1 -o "$CNTCHK" -gt 4 ]
      then
         INFOARG=0
      else
         INFOARG=$1
      fi
   else
      INFOARG=$1
   fi
   echo ""
   echo "----------------------------------------------------------------------------"
   case $INFOARG in
      1) if [ $BETAOPT -eq 1 ]
         then
            echo "This will enable the MUSH Doors portion of @door connectivity."
            echo "This is used with the doors.txt/doors.indx files for sites."
         elif [ $RUNBETA -eq 1 ]
         then
            echo "This enables you to automatically include the MYSQL definitions"
            echo "to the DEFs line in the Makefile.  This is to support if you have"
            echo "this package installed and should only be used then.  Otherwise,"
            echo "it's safe to leave it at the default."
         else
            echo "Sideeffects are functions like set(), pemit(), create(), and the like."
            echo "This must be enabled to be able to use these functions.  In addition,"
            echo "when enabeled, the SIDEFX flag must be individually set on anything"
            echo "you wish to use the sideeffects."
         fi
         ;;
      2) if [ $BETAOPT -eq 1 ]
         then
            echo "This will enable the Empire (Unix Game) door.  You must have"
            echo "A currently running unix game server running and listening on a port."
         elif [ $RUNBETA -eq 1 ]
         then
            echo "This option drills down to various door options that you can enable."
            echo "The currently supported production doors are the ones listed."
            echo "Keep in mind each one does require tweeking files."
         else
            echo "RhostMUSH, by default, have u(), ufun(), and zfun() functions parse by"
            echo "relation of the enactor instead of by relation of the target.  This is"
            echo "more MUSE compatable than MUSH.  If you wish to have it more compatable"
            echo "to MUX/TinyMUSH/Penn, then you need this enabled."
         fi
         ;;
      3) if [ $BETAOPT -eq 1 ]
         then
            echo "This enables the POP3 door interface.  This is still very limited"
            echo "in functionality and currently only displays total mail counts."
         elif [ $RUNBETA -eq 1 ]
         then
            echo "This allows you to have 64 character attribute length.  This in effect"
            echo "will set all SBUF's to 64 bit length instead of 32.  This has not been"
            echo "fully tested or regression tested so there could be some concern if"
            echo "you choose to use this.  If you find errors, report to us immediately."
         else
            echo "RhostMUSH, by default, uses inc() and dec() to increment and decrement"
            echo "setq registers.  This is, unfortunately, not the default behavior for"
            echo "MUX, Penn, or TinyMUSH.  By enabling this options, you switch the"
            echo "functionality of inc() and dec() to be like MUX/Penn/TinyMUSH."
         fi
         ;;
      4) echo "RhostMUSH has a very archiac and obtuse comsystem.  It does work, and"
         echo "is very secure and solid, but it lacks significant functionality."
         echo "You probably want to toggle this on and use a softcoded comsystem."
         ;;
      5) echo "RhostMUSH by default allows %c to be ansi.  However, on TinyMUSH3,"
         echo "%x is used for ANSI.  Use this to switch how %c and %x is used."
         ;;
      6) echo "RhostMUSH supports crypt() and decrypt() functions.  Toggle this"
         echo "if you wish to use them."
         ;;
      7) echo "RhostMUSH allows you to use a plushelp.txt file for +help.  This"
         echo "supports MUX/TinyMUSH3 in how +help is hardcoded to a text file."
         echo "Enable this if you wish to have a plushelp.txt file be used."
         ;;
      8) echo "RhostMUSH, by default, allows multiple arguments to be passed"
         echo "to @program.  This, unfortunately, is not how MUX does it, so"
         echo "if you wish a MUX compatable @program, enable this."
         ;;
      9) echo "RhostMUSH, optionally, allows you to use the COMMAND flag to"
         echo "specify what objects are allowed to run $comands.  If you wish"
         echo "to have this flag enabled, toggle this option."
         ;;
     10) echo "RhostMUSH allows 'special' attributes that start with _ or ~."
         echo "By default, _ attributes are wiz settable and seeable only."
         echo "If you wish to have these special attributes, enable this option."
         ;;
     11) echo "Reality Levels is a document onto itself.  In essence, reality"
         echo "levels allow a single location to have multiple 'realities' where"
         echo "you can belong to one or more realities and interact with one or"
         echo "more realities.  It's an excellent way to handle invisibility,"
         echo "shrouding, or the like.  Enable this if you want it."
         ;;
     12) echo "Sometimes, the standard 10 (0-9) setq registers just arn't enough."
         echo "If you, too, find this to be the case, enable this option and it"
         echo "will allow you to have a-z as well."
         ;;
     13) echo "RhostMUSH handles ansi totally securely.  It does this by stripping"
         echo "ansi from all evaluation.  This does, however, have the unfortunate"
         echo "effect of making ansi very cumbersome and difficult to use.  If you"
         echo "have this uncontrollable desire to have color everywhere, then you"
         echo "probably want this enabled as it allows you to use ansi in most"
         echo "places without worry of stripping."
         ;;
     14) echo "Marker flags are your 10 dummy flags (marker0 to marker9).  These"
         echo "flags, are essentially 'markers' that you can rename at leasure."
         echo "If you have a desire for marker flags, enable this option."
         ;;
     15) echo "Bang support.  Very cool stuff.  It allows you to use ! for false"
         echo "and !! for true.  An example would be [!match(this,that)].  It"
         echo "also allows $! for 'not a string' and $!! for 'is a string'."
         echo "Such an example would be [$!get(me/blah)].  If you like this"
         echo "feature, enable this option.  You want it.  Really."
         ;;
     16) echo "This is an alternate WHO listing.  It's a tad longer for the"
         echo "display and will switch ports to Total Cmds on the WHO listings."
         ;;
     17) echo "This goes back to an old implementation of the setq/setr register"
         echo "emplementation.  This may make it more difficult to use ansi but"
         echo "the newer functions would still exist.  In essence, this will"
         echo "switch SETQ with SETQ_OLD and SETR with SETR_OLD in the code."
         ;;
     18) echo "This shores up sideeffects for pemit(), remit(), zemit(), and"
         echo "others to avoid a double evaluation.  You probably want this"
         echo "enabled as it mimics MUX/PENN, but if you have an old Rhost"
         echo "that used sideeffects, this can essentially break compatabiliity."
         ;;
     19) echo "This disables the debug monitor (debugmon) from working within Rhost."
         echo "There is really no need to disable this, unless you are in a chroot"
         echo "jail or in some way are on a restricted unix shell that does not allow"
         echo "you to use shared memory segments."
         ;;
     20) echo "This is intended to disable signal handling in RhostMUSH.  Usually,"
         echo "you can send TERM, USR1, and USR2 signals to reboot, flatfile, or"
         echo "shutdown the mush."
         ;;
     21) echo "The Reality Levels in use prior required the chkreality toggle to"
         echo "be used for options 2 & 3 in locktypes.  If the lock did not exist"
         echo "it would assume failure even if no chkreality toggle was set.  This"
         echo "was obviously an error.  However, if this is enabled, then this"
         echo "behavior will remain so that any preexisting code will work as is."
         ;;
     22) echo "This allows you to natively read MUX2 set SHA1 passwords in a"
         echo "converted database.  If you change the password, it will use the"
         echo "Rhost specific password system and overwrite the SHA1 password."
         echo "You only need to use this if you are using a MUX2 converted flatfile."
         ;;
     23) echo "This is a low memory compile option.  If you are running under a"
         echo "Virtual Machine, or have low available memory, the compile may"
         echo "error out saying out of memory, unable to allocate memory, or"
         echo "similiar messages.  Enabling this option should bypass this."
         ;;
     B*|b*) RUNBETA=1
         info $(echo $1|cut -c2-)
         RUNBETA=0
         ;;
      *) echo "Not a valid number.  "
         if [ $BETAOPT -eq 1 ]
         then
            echo "Please select between 1 and ${C_DOPTIONS} for information."
         else
            echo "Please select between 1 and ${C_OPTIONS} for information."
            echo "Please select between B1 and B${C_BOPTIONS} for beta."
         fi
         ;;
   esac
   echo "----------------------------------------------------------------------------"
}

###################################################################
# PARSE - Parse the argument the player types
###################################################################
parse() {
   ARG=$(echo $1|tr '[:upper:]' '[:lower:]')
   ARGNUM=$(expr ${ARG} + 0 2>/dev/null)
   if [ $BETAOPT -eq 1 ]
   then
      if [ "$ARG" != "q" -a "$ARG" != "i" -a "$ARG" != "h" ]
      then
         if [ -z "$ARGNUM" ]
         then
            ARG="NULL"
         elif [ $ARGNUM -lt 1 -o $ARGNUM -gt 3 ]
         then
            ARG="NULL"
         fi
      fi
   fi
   case ${ARG} in
      x) xtraopts
         echo "< HIT RETURN KEY TO CONTINUE >"
         read ANS
         ;;
      r) REPEAT=0 
         echo "Running with current configuration..."
         echo "< HIT RETURN KEY TO CONTINUE >"
         read ANS
         ;;
      l) loadopts
         echo "< HIT RETURN KEY TO CONTINUE >"
         read ANS
         ;;
      q) if [ ${BETAOPT} -eq 1 ]
         then
            BETACONTINUE=0
         else
            echo "Aborting"
            echo "< HIT RETURN KEY TO CONTINUE >"
            read ANS
            exit 1
         fi
         ;;
      s) saveopts
         echo "< HIT RETURN KEY TO CONTINUE >"
         read ANS
         ;;
      m) markopts
         echo "< HIT RETURN KEY TO CONTINUE >"
         read ANS
         ;;
      b) browseopts
         echo "< HIT RETURN KEY TO CONTINUE >"
         read ANS
         ;;
      c) clearopts
         echo "< HIT RETURN KEY TO CONTINUE >"
         read ANS
         ;;
      d) deleteopts
         echo "< HIT RETURN KEY TO CONTINUE >"
         read ANS
         ;;
      i) info $2
         echo "< HIT RETURN KEY TO CONTINUE >"
         read ANS
         ;;
      h) help
         echo "< HIT RETURN KEY TO CONTINUE >"
         read ANS
         ;;
      b*) TST=$(expr $(echo $1|cut -c2-) + 0 2>/dev/null)
         BETAOPT=0
         if [ -z "$TST" ]
         then
            echo "ERROR: Invalid option '$1'"
            echo "< HIT RETURN KEY TO CONTINUE >"
            read ANS
         elif [ "$TST" -gt 0 -a "$TST" -le ${C_BOPTIONS} ]
         then  
            if [ $TST -eq 2 ]
            then
               BETAOPT=1
            else
               if [ "${XB[${TST}]}" = "X" ]
               then
                  XB[${TST}]=" "
               else
                  XB[${TST}]="X"
               fi
            fi
         else
            echo "ERROR: Invalid option '$1'"
            echo "< HIT RETURN KEY TO CONTINUE >"
            read ANS
         fi
         ;;
      *) TST=$(expr $1 + 0 2>/dev/null)
         if [ -z "$TST" ]
         then
            echo "ERROR: Invalid option '$1'"
            echo "< HIT RETURN KEY TO CONTINUE >"
            read ANS
         elif [ ${BETAOPT} -eq 1 -a "$TST" -gt 0 -a "$TST" -le ${C_DOPTIONS} ]
         then
            if [ "${XD[$1]}" = "X" ]
            then
               XD[$1]=" "
            else
               XD[$1]="X"
            fi
         elif [ ${BETAOPT} -eq 0 -a "$TST" -gt 0 -a "$TST" -le ${C_OPTIONS} ]
         then  
            if [ "${X[$1]}" = "X" ]
            then
               X[$1]=" "
            else
               X[$1]="X"
            fi
         else
            echo "ERROR: Invalid option '$1'"
            echo "< HIT RETURN KEY TO CONTINUE >"
            read ANS
         fi
         ;;
   esac
}

###################################################################
# CLEAROPTS - clear current configurations to blank slate
###################################################################
clearopts() {
   for i in ${OPTIONS}
   do
      X[${i}]=" "
   done
   for i in ${BOPTIONS}
   do
      XB[${i}]=" "
   done
   for i in ${DOPTIONS}
   do
      XD[${i}]=" "
   done
   echo "Options have been cleared."
}

###################################################################
# DELETEOPTS - Delete previous config options
###################################################################
deleteopts() {
   LOOPIE=1
   VALID=""
   echo "Delete which slot:"
   while [ ${LOOPIE} -eq 1 ]
   do
      for i in 0 1 2 3
      do
         if [ -f asksource.save${i}.mark ]
         then
            MARKER=$(cat asksource.save${i}.mark)
         else
            MARKER=""
         fi
         if [ -s asksource.save${i} ]
         then
            echo "${i}. Slot #${i} In-Use [${MARKER:-GENERIC}]"
            VALID="${VALID} ${i}"
         fi
      done
      VALID=$(echo ${VALID})
      if [ -z "${VALID}" ]
      then
         echo "You have no options to delete."
         LOOPIE=0
         VALID=XXX
         break
      fi
      VALID="${VALID} Q q"
      echo "Q. Abort screen."
      echo ""
      echo "Please enter selection: "|tr -d '\012'
      read SAVEANS
      CHK=$(echo ${VALID}|grep -c "${SAVEANS}")
      if [ ${CHK} -eq 0 ]
      then
         SAVEANS=XXX
      fi
      LOOPIE=0
      case ${SAVEANS} in
         0) DUMPFILE=asksource.save0
            ;;
         1) DUMPFILE=asksource.save1
            ;;
         2) DUMPFILE=asksource.save2
            ;;
         3) DUMPFILE=asksource.save3
            ;;
         Q) SAVEANS=XXX
            ;;
         q) SAVEANS=XXX
            ;;
         *) echo "Invalid option.  Please choose one of '${VALID}'"
            LOOPIE=1
            ;;
      esac
   done
   if [ "${SAVEANS}" != "XXX" -a "${VALID}" != "XXX" ]
   then
      MARKER=$(cat asksource.save${SAVEANS}.mark)
      echo "You want to delete slot ${SAVEANS} [${MARKER:-GENERIC}]"
      LOOPIE=1
      while [ ${LOOPIE} -eq 1 ]
      do
         echo "Is this correct? (Y/N) : "|tr -d '\012'
         read ANS2
         if [ "${ANS2}" = "y" -o "${ANS2}" = "Y" ]
         then
            rm -f ${DUMPFILE}
            rm -f ${DUMPFILE}.mark
            LOOPIE=0
            echo "Deleted."
         elif [ "${ANS2}" = "n" -o "${ANS2}" = "N" ]
         then
            LOOPIE=0
            echo "Deletion aborted."
         else
            echo "Please enter Y or N."
         fi 
      done
   fi
}

###################################################################
# BROWSEOPTS - Browse previous config options
###################################################################
browseopts() {
   for i in 0 1 2 3
   do
      if [ -f asksource.save${i}.mark ]
      then
         MARKER=$(cat asksource.save${i}.mark)
      else
         MARKER=""
      fi
      if [ -s asksource.save${i} ]
      then
         echo "${i}. Slot #${i} In-Use [${MARKER:-GENERIC}]"
         VALID="${VALID} ${i}"
      fi
   done
   VALID=$(echo ${VALID})
   if [ -z "${VALID}" ]
   then
      echo "You have no options."
   fi
}

###################################################################
# MARKOPTS - Load previous config options
###################################################################
markopts() {
   LOOPIE=1
   VALID=""
   echo "Label which slot:"
   while [ ${LOOPIE} -eq 1 ]
   do
      for i in 0 1 2 3
      do
         if [ -f asksource.save${i}.mark ]
         then
            MARKER=$(cat asksource.save${i}.mark)
         else
            MARKER=""
         fi
         if [ -s asksource.save${i} ]
         then
            echo "${i}. Slot #${i} In-Use [${MARKER:-GENERIC}]"
            VALID="${VALID} ${i}"
         fi
      done
      VALID=$(echo ${VALID})
      if [ -z "${VALID}" ]
      then
         echo "You have no options to mark."
         LOOPIE=0
         break
      fi
      VALID="${VALID} Q q"
      echo "Q. Abort screen."
      echo ""
      echo "Please enter selection: "|tr -d '\012'
      read SAVEANS
      CHK=$(echo ${VALID}|grep -c "${SAVEANS}")
      if [ ${CHK} -eq 0 ]
      then
         SAVEANS=XXX
      fi
      LOOPIE=0
      case ${SAVEANS} in
         0) DUMPFILE=asksource.save0.mark
            ;;
         1) DUMPFILE=asksource.save1.mark
            ;;
         2) DUMPFILE=asksource.save2.mark
            ;;
         3) DUMPFILE=asksource.save3.mark
            ;;
         q) SAVEANS=XXX
            ;;
         Q) SAVEANS=XXX
            ;;
         *) echo "Invalid option.  Please choose one of '${VALID}'"
            LOOPIE=1
            ;;
      esac
   done
   if [ "${SAVEANS}" = "XXX" ]
   then
      LOOPIE=0
   else
      LOOPIE=1
   fi
   while [ ${LOOPIE} -eq 1 ]
   do
      echo "Enter label now: "|tr -d '\012'
      read ANS
      echo ""
      echo "You want to label slot ${SAVEANS} with ${ANS}"
      echo "Is this correct? (Y/N) : "|tr -d '\012'
      read ANS2
      if [ "${ANS2}" = "y" -o "${ANS2}" = "Y" ]
      then
         LOOPIE=0
         echo "${ANS}" > ${DUMPFILE}
         echo "Label marked."
      fi
   done
}

###################################################################
# XTRAOPTS - Load extra/default config options
###################################################################
xtraopts() {
   LOOPIE=1
   VALID=""
   echo "Load which default configuration:"
   while [ ${LOOPIE} -eq 1 ]
   do
      CNTR=0
      for i in mux penn tm3 default
      do
         CNTR=$(expr $CNTR + 1)
         if [ -f asksource.${i}.mark ]
         then
            MARKER=$(cat asksource.${i}.mark)
         else
            MARKER=""
         fi
         if [ -s asksource.${i} ]
         then
            echo "${CNTR}. Slot #${CNTR} [${MARKER:-GENERIC}]"
            VALID="${VALID} ${CNTR}"
         fi
      done
      VALID=$(echo ${VALID})
      if [ -z "${VALID}" ]
      then
         LOOPIE=0
         break
      fi
      VALID="${VALID} Q q"
      echo "Q. Abort screen."
      echo ""
      echo "Please enter selection: "|tr -d '\012'
      read SAVEANS
      CHK=$(echo ${VALID}|grep -c "${SAVEANS}")
      if [ ${CHK} -eq 0 ]
      then
         SAVEANS=XXX
      fi
      LOOPIE=0
      case ${SAVEANS} in
         1) DUMPFILE=asksource.mux
            ;;
         2) DUMPFILE=asksource.penn
            ;;
         3) DUMPFILE=asksource.tm3
            ;;
         4) DUMPFILE=asksource.default
            ;;
         Q) DUMPFILE="/"
            ;;
         q) DUMPFILE="/"
            ;;
         *) echo "Invalid option.  Please choose one of '${VALID}'"
            LOOPIE=1
            ;;
      esac
   done
   if [ -f "${DUMPFILE}" ]
   then
      if [ -f "${DUMPFILE}.mark" ]
      then
         MARKER=$(cat ${DUMPFILE}.mark)
      else
         MARKER=""
      fi
      . ${DUMPFILE} 2>/dev/null
      echo "Loading default config options from slot ${SAVEANS} [${MARKER:-GENERIC}]"
   elif [ "${DUMPFILE}" = "/" ]
   then
      echo ""
   else
      echo "ERROR: No previous config options found to load."
   fi
   DUMPFILE=""
}

###################################################################
# LOADOPTS - Load previous config options
###################################################################
loadopts() {
   LOOPIE=1
   VALID=""
   echo "Load from which slot:"
   while [ ${LOOPIE} -eq 1 ]
   do
      for i in 0 1 2 3
      do
         if [ -f asksource.save${i}.mark ]
         then
            MARKER=$(cat asksource.save${i}.mark)
         else
            MARKER=""
         fi
         if [ -s asksource.save${i} ]
         then
            echo "${i}. Slot #${i} In-Use [${MARKER:-GENERIC}]"
            VALID="${VALID} ${i}"
         fi
      done
      VALID=$(echo ${VALID})
      if [ -z "${VALID}" ]
      then
         LOOPIE=0
         break
      fi
      VALID="${VALID} Q q"
      echo "Q. Abort screen."
      echo ""
      echo "Please enter selection: "|tr -d '\012'
      read SAVEANS
      CHK=$(echo ${VALID}|grep -c "${SAVEANS}")
      if [ ${CHK} -eq 0 ]
      then
         SAVEANS=XXX
      fi
      LOOPIE=0
      case ${SAVEANS} in
         0) DUMPFILE=asksource.save0
            ;;
         1) DUMPFILE=asksource.save1
            ;;
         2) DUMPFILE=asksource.save2
            ;;
         3) DUMPFILE=asksource.save3
            ;;
         Q) DUMPFILE="/"
            ;;
         q) DUMPFILE="/"
            ;;
         *) echo "Invalid option.  Please choose one of '${VALID}'"
            LOOPIE=1
            ;;
      esac
   done
   if [ -f "${DUMPFILE}" ]
   then
      if [ -f "${DUMPFILE}.mark" ]
      then
         MARKER=$(cat ${DUMPFILE}.mark)
      else
         MARKER=""
      fi
      . ${DUMPFILE} 2>/dev/null
      echo "Loading previous config options from slot ${SAVEANS} [${MARKER:-GENERIC}]"
   elif [ "${DUMPFILE}" = "/" ]
   then
      echo ""
   else
      echo "ERROR: No previous config options found to load."
   fi
   DUMPFILE=""
}

###################################################################
# SAVEOPTS - Save the options to the DEFS to a specified slot
###################################################################
saveopts() {
   LOOPIE=1
   echo "Save in which slot:"
   while [ ${LOOPIE} -eq 1 ]
   do
      for i in 0 1 2 3
      do
         if [ -f asksource.save${i}.mark ]
         then
            MARKER=$(cat asksource.save${i}.mark)
         else
            MARKER=""
         fi
         if [ -s asksource.save${i} ]
         then
            echo "${i}. Slot #${i} In-Use [${MARKER:-GENERIC}]"
         else
            echo "${i}. Slot #${i} - EMPTY"
         fi
      done
      echo "Q. Abort screen."
      echo ""
      echo "Please enter selection: "|tr -d '\012'
      read SAVEANS
      LOOPIE=0
      case ${SAVEANS} in
         0) DUMPFILE=asksource.save0
            ;;
         1) DUMPFILE=asksource.save1
            ;;
         2) DUMPFILE=asksource.save2
            ;;
         3) DUMPFILE=asksource.save3
            ;;
         Q) DUMPFILE="/"
            ;;
         q) DUMPFILE="/"
            ;;
         *) echo "Invalid option.  Please choose one of '0 1 2 3'"
            LOOPIE=1
            ;;
      esac
   done
   if [ "${DUMPFILE}" != "/" ]
   then
      cat /dev/null > ${DUMPFILE}
      for i in ${OPTIONS}
      do
         echo "X[$i]=\"${X[$i]}\"" >> ${DUMPFILE}
      done
      for i in ${BOPTIONS}
      do
         echo "XB[$i]=\"${XB[$i]}\"" >> ${DUMPFILE}
      done
      for i in ${DOPTIONS}
      do
         echo "XD[$i]=\"${XD[$i]}\"" >> ${DUMPFILE}
      done
      if [ -f "${DUMPFILE}.mark" ]
      then
         MARKER=$(cat ${DUMPFILE}.mark)
      else
         MARKER=""
      fi
      echo "Options saved to slot ${SAVEANS} [${MARKER:-GENERIC}]"
   fi
}

###################################################################
# SETOPTS - Set options for compiletime run for makefile mod
###################################################################
setopts() {
   for i in ${OPTIONS}
   do
      if [ "${X[$i]}" = "X" ]
      then
         DEFS="${DEF[$i]} ${DEFS}"
      fi
   done
   for i in ${BOPTIONS}
   do
      if [ "${XB[$i]}" = "X" ]
      then
         DEFS="${DEFB[$i]} ${DEFS}"
      fi
   done
   for i in ${DOPTIONS}
   do
      if [ "${XD[$i]}" = "X" ]
      then
         DEFS="${DEFD[$i]} ${DEFS}"
      fi
   done
}

###################################################################
# SETDEFAULTS - Assign default DEFS required for platform
###################################################################
setdefaults() {
  echo "Configuring default definitions..."
  gcc ../src/intsize.c -o ../src/intsize >/dev/null 2>&1
  if [ $? -eq 0 ]
  then
     if [ "$(../src/intsize)" -gt 4 ]
     then
        echo "Patching for 64bit platform..."
        DEFS="-DBIT64 ${DEFS}"
     fi
  fi
  if [ "$(uname -a|grep -ci aix)" -ne 0 ]
  then
     echo "Patching for AIX..."
     DEFS="-DFIX_AIX ${DEFS}"
  fi

  if [ -f /usr/include/sys/errno.h ]
  then
     echo  "Patching for errno.h <sys>..."
     DEFS="-DBROKEN_ERRNO_SYS ${DEFS}"
     if [ -f "/usr/include/errno.h" ]
     then
       echo "   # It may not work.  If it doesn't, first remove -DBROKEN_ERRNO_SYS from the DEFS line."
       echo "   # If that doesn't work, change to -DBROKEN_ERRNO"
     fi
  elif [ -f /usr/include/asm/errno.h ]
  then
     echo  "Patching for errno.h <asm>..."
     if [ -f /usr/include/errno.h ]
     then
       echo "   # It may not work.  If it doesn't, Remove -DBROKEN_ERRNO from the DEFS line."
     fi
     DEFS="-DBROKEN_ERRNO ${DEFS}"
  fi
  DEFS="-DBROKEN_NDBM ${DEFS}"
  if [ ! -f /usr/include/wait.h ]
  then
     echo "Patching for wait.h <sys>..."
     DEFS="-DSYSWAIT ${DEFS}"
  fi
  if [ -f /usr/include/sys/malloc.h ]
  then
     echo "Patching for weird malloc.h..."
     DEFS="-DSYS_MALLOC ${DEFS}"
  fi
  if [ -f /usr/include/bsd/bsd.h -o ! -f /usr/include/values.h ]
  then
     echo "BSD identified.  Configuring..."
     DEFS="-DBSD_LIKE ${DEFS}"
  fi
  BOB1=$(uname -r|cut -f1 -d".")
  BOB2=$(uname -s)
  if [ -d /usr/ucbinclude -a "${BOB2}" = "SunOS" ]
  then
     if [ "${BOB1}" -lt 5 ]
     then
        echo "SunOS 4.x compatable.  Configuring..."
        DEFS="${DEFS} -I\/usr\/ucbinclude -DSOLARIS"
     else
        echo "SunOS 5.x compatable.  Configuring..."
        DEFS="${DEFS} -DSOLARIS"
     fi
  fi
  if [ -f /usr/include/openssl/sha.h -a -f /usr/include/openssl/evp.h ]
  then
     echo "OpenSSL identified.  Configuring..."
     DEFS="${DEFS} -DHAS_OPENSSL"
  fi
  DEFS="DEFS = ${DEFS}"
}

###################################################################
# SETLIBS - Set the libraries required for platform
###################################################################
setlibs() {
   echo "Configuring default libraries..."
   Z1=$(ls /lib/libcrypt.* 2>/dev/null|wc -l)
   Z2=$(ls /usr/lib/libcrypt.* 2>/dev/null|wc -l)
   Z3=$(ls /usr/local/lib/libcrypt.* 2>/dev/null|wc -l)
   if [ "$Z1" -gt 0 -o "$Z2" -gt 0 -o "$Z3" -gt 0 ]
   then
      echo "Patching -lcrypt..."
      MORELIBS="-lcrypt ${MORELIBS}"
   fi
   Z1=$(ls /lib/libsocket.* 2>/dev/null|wc -l)
   Z2=$(ls /usr/lib/libsocket.* 2>/dev/null|wc -l)
   Z3=$(ls /usr/local/lib/libsocket.* 2>/dev/null|wc -l)
   if [ "$Z1" -gt 0 -o "$Z2" -gt 0 -o "$Z3" -gt 0 ]
   then
      echo "Patching -lsocket..."
      MORELIBS="-lsocket ${MORELIBS}"
   fi
   Z1=$(ls /lib/libresolv.* 2>/dev/null|wc -l)
   Z2=$(ls /usr/lib/libresolv.* 2>/dev/null|wc -l)
   Z3=$(ls /usr/local/lib/libresolv.* 2>/dev/null|wc -l)
   if [ "$Z1" -gt 0 -o "$Z2" -gt 0 -o "$Z3" -gt 0 ]
   then
      echo "Patching -lresolv..."
      MORELIBS="-lresolv ${MORELIBS}"
   fi
   Z1=$(ls /lib/libnsl.* 2>/dev/null|wc -l)
   Z2=$(ls /usr/lib/libnsl.* 2>/dev/null|wc -l)
   Z3=$(ls /usr/local/lib/libnsl.* 2>/dev/null|wc -l)
   if [ "$Z1" -gt 0 -o "$Z2" -gt 0 -o "$Z3" -gt 0 ]
   then
      echo "Patching -lnsl..."
      MORELIBS="-lnsl ${MORELIBS}"
   fi
   Z1=$(ls /lib/libm.* 2>/dev/null|wc -l)
   Z2=$(ls /usr/lib/libm* 2>/dev/null|wc -l)
   Z3=$(ls /usr/local/lib/libm* 2>/dev/null|wc -l)
   if [ "$Z1" -gt 0 -o "$Z2" -gt 0 -o "$Z3" -gt 0 ]
   then
      echo "Patching -lm..."
      MORELIBS="-lm ${MORELIBS}"
   fi
   BOB1=$(uname -r|cut -f1 -d".")
   BOB2=$(uname -s)
   if [ -d /usr/ucblib -a "${BOB2}" = "SunOS" -a "${BOB1}" -lt 5 ]
   then
      echo "MIPS SunOS 4.x identified.  Configuring..."
      MORELIBS="${MORELIBS} -L\/usr\/ucblib -l ucb"
   fi
   if [ -f /usr/include/openssl/sha.h -a -f /usr/include/openssl/evp.h ]
   then
      ls /usr/lib/libcrypto* >/dev/null 2>&1
      errchk=$?
      if [ ${errchk} -eq 0 ]
      then
         echo "Configuring libcrypto..."
         MORELIBS="${MORELIBS} -lcrypto"
      fi
      echo "Configuring libssl..."
      MORELIBS="${MORELIBS} -lssl"
   fi
   MORELIBS="MORELIBS = ${MORELIBS}"
}

###################################################################
# UPDATEMAKEFILE - Update the makefile with the changes
###################################################################
updatemakefile() {
   echo "Updating the DEFS section of the Makefile now.  Please wait..."
   cat ../src/Makefile|sed "s/$(grep ^DEF ../src/Makefile|sed "s/\//\\\\\//g")/${DEFS}/g" > /tmp/$$CONF$$
   mv -f ../src/Makefile ../src/Makefile.${DATE} 2>/dev/null
   mv -f /tmp/$$CONF$$ ../src/Makefile 2>/dev/null
   rm -f /tmp/$$CONF$$ 2>/dev/null

#  Let's do the door/compiletime additions here
   if [ "${XB[2]}" = "X" ]
   then
      cat ../src/Makefile|sed "s/^#DR_DEF/DR_DEF/g" > /tmp/$$CONF$$
      mv -f /tmp/$$CONF$$ ../src/Makefile 2>/dev/null
      rm -f /tmp/$$CONF$$ 2>/dev/null
      if [ "${XD[1]}" = "X" ]
      then
         cat ../src/Makefile|sed "s/^#DRMUSH/DRMUSH/g" > /tmp/$$CONF$$
      else
         cat ../src/Makefile|sed "s/^DRMUSH/#DRMUSH/g" > /tmp/$$CONF$$
      fi
      mv -f /tmp/$$CONF$$ ../src/Makefile 2>/dev/null
      rm -f /tmp/$$CONF$$ 2>/dev/null
      if [ "${XD[2]}" = "X" ]
      then
         cat ../src/Makefile|sed "s/^#DREMPIRE/DREMPIRE/g"|sed "s/^#DR_HDR/DR_HDR/g" > /tmp/$$CONF$$
      else
         cat ../src/Makefile|sed "s/^DREMPIRE/#DREMPIRE/g"|sed "s/^DR_HDR/#DR_HDR/g" > /tmp/$$CONF$$
      fi
      mv -f /tmp/$$CONF$$ ../src/Makefile 2>/dev/null
      rm -f /tmp/$$CONF$$ 2>/dev/null
      if [ "${XD[3]}" = "X" ]
      then
         cat ../src/Makefile|sed "s/^#DRMAIL/DRMAIL/g" > /tmp/$$CONF$$
      else
         cat ../src/Makefile|sed "s/^DRMAIL/#DRMAIL/g" > /tmp/$$CONF$$
      fi
      mv -f /tmp/$$CONF$$ ../src/Makefile 2>/dev/null
      rm -f /tmp/$$CONF$$ 2>/dev/null
   else
      cat ../src/Makefile|sed "s/^DR_DEF/#DR_DEF/g"|sed "s/^DRMUSH/#DRMUSH/g"| \
         sed "s/^DREMPIRE/#DREMPIRE/g"|sed "s/^DRMAIL/#DRMAIL/g" > /tmp/$$CONF$$
      mv -f /tmp/$$CONF$$ ../src/Makefile 2>/dev/null
      rm -f /tmp/$$CONF$$ 2>/dev/null
   fi
   echo "...completed."
   # add CFLAGS for low memory
   if [ "${X[23]}" = "X" ]
   then
      echo "Adding CFLAG option for low memory compile..."
      cat ../src/Makefile|sed "s/^#CFLAG/CFLAG/g" > /tmp/$$CONF$$
   else
      cat ../src/Makefile|sed "s/^CFLAG/#CFLAG/g" > /tmp/$$CONF$$
   fi
   mv -f /tmp/$$CONF$$ ../src/Makefile 2>/dev/null
   rm -f /tmp/$$CONF$$ 2>/dev/null
   echo "Updating the MORELIBS section of the Makefile now.  Please wait..."
   cat ../src/Makefile|sed "s/$(grep ^MORELIBS ../src/Makefile| \
       sed "s/\//\\\\\//g")/${MORELIBS}/g" > /tmp/$$CONF$$
   mv -f ../src/Makefile ../src/Makefile.${DATE} 2>/dev/null
   mv -f /tmp/$$CONF$$ ../src/Makefile 2>/dev/null
   rm -f /tmp/$$CONF$$ 2>/dev/null
   echo "...completed."
}

###################################################################
# MAIN - Main system call and loop
###################################################################
main() {
   while [ ${REPEAT} -eq 1 ]
   do
      menu
      read ANS
      parse $ANS
      if [ ${BETAOPT} -eq 1 ]
      then
          BETACONTINUE=1
          while [ $BETACONTINUE -eq 1 ]
          do
             doormenu
             read ANS
             parse $ANS
          done
          BETAOPT=0
          if [ "${XD[1]}" != "X" -a "${XD[2]}" != "X" -a "${XD[3]}" != "X" ]
          then
             XB[2]=" "
          else
             XB[2]="X"
          fi
      fi
   done
   setopts
   setdefaults
   setlibs
   updatemakefile
   echo "< HIT RETURN KEY TO CONTINUE >"
   read ANS
}

###################################################################
# Ok, let's run it
###################################################################
main
