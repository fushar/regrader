#!/bin/bash
# Create /etc/shadow, /etc/passwd and /etc/group records for accounts
# used by the evaluator and the contestants.

[ -f cf/mop ] || { echo "Missing config file, check cwd." ; exit 1 ; }
set -e
. cf/mop

mode=$1

case "$mode" in
  eval) uid=$EVAL_UID_MIN;;
  users) uid=$CT_UID_MIN;;
  *) echo "Usage: $0 [eval|users]!"; exit 1;;
esac

>>etcshadow >>etcpasswd >>etcgroup
expire=$((`date +%s` / 86400 - 1))

case $mode in
  eval)
    echo -n Enter password for evaluation users:
    read passwd
    passwd=`echo $passwd | bin/md5crypt`
    gid=$uid
    tgid=$(($gid+1))
    rgid=$(($gid+2))
    echo $EVAL_USER:x:$uid:$gid:MO Evaluator:$MO_ROOT/eval/eval:/bin/bash >> etcpasswd
    echo $EVAL_USER:$passwd:$expire:0:99999:7:::		>> etcshadow
    echo $EVAL_GROUP:x:$gid:					>> etcgroup
    echo $TEST_GROUP:x:$tgid:$EVAL_USER				>> etcgroup

    if [ -n "$REMOTE_SUBMIT" ] ; then
      uid=$(($uid + 1))
      echo $REMOTE_SUBMIT_USER:x:$uid:$rgid:MO Submitter:$MO_ROOT/eval/submit:/bin/bash	>> etcpasswd
      echo $REMOTE_SUBMIT_USER:$passwd:$expire:0:99999:7:::				>> etcshadow
      echo $REMOTE_SUBMIT_GROUP:x:$rgid:						>> etcgroup
    fi

    tuid=$(($uid + 1))
    for tester in $TEST_USERS; do
      echo $tester:x:$tuid:$tgid:MO Tester `expr $tuid - $uid`:$MO_ROOT/eval/$tester:/bin/bash	>> etcpasswd
      echo $tester:$passwd:$expire:0:99999:7:::				>> etcshadow
      tuid=$(($tuid + 1))
    done
  ;;

  users)
#   if [ -f logins.tex ]; then echo "File logins.tex exists! Bailing out!"; exit 1; fi
    cat > logins.tex <<- EOF
	\\nopagenumbers
	\\voffset=-1.5cm\\vsize=280mm\\hoffset=-0.75cm\\advance\\hsize by 3cm
	\\rightskip=0pt plus 3in\\parindent=0pt
	\\font\\ftt=cstt17
	\\font\\frm=csr17
	\\font\\fit=csti17
	\\ftt
	\\def\\user#1#2#3{\\vbox to 4.5cm{\\hsize=6cm\\vss\\vss{\\fit Practice Session}\\vss\\vss{\\frm #3}\\vss\\vss#1\\vss#2\\vss\\vss}}
	\\leavevmode
	EOF

    bin/mo-get-users --full | while read user name; do
      passwd=`apg -n1 -m6 -Mncl -E"01lO" | cut -d" " -f1`
      passwd_md5=`echo $passwd | bin/md5crypt`
      echo $user:x:$uid:$uid:$name:$MO_ROOT/users/$user/$user:/bin/bash			>> etcpasswd
      echo $user:x:$uid:								>> etcgroup
      echo $user:$passwd_md5:`expr \`date +%s\` / 86400 - 1`:0:99999:7:::		>> etcshadow
      echo "\\user{$user}{$passwd}{$name}"						>> logins.tex
      uid=$(($uid + 1))
    done

    cat >> logins.tex <<- EOF
	\\vfil
	\\bye
	EOF
  ;;
esac
