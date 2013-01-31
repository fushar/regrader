# Test cases for judge-tok.c 2>&1

# Identical input and output
Name:	ok
Run:	bin/judge-tok $1 $2 2>&1
In1:	abc d e fgh
	ijk lmn
In2:	abc d e fgh
	ijk lmn

# Differences in whitespaces are OK
Name:	ws
Run:	bin/judge-tok $1 $2 2>&1
In1:	abc 	  d e fgh  
	   ijk  lmn
In2:	abc d  e fgh
	ijk lmn

# Differences in line breaks are not
Name:	lines1
Run:	bin/judge-tok $1 $2 2>&1
In1:	abc  d
	e fgh  
	   ijk  lmn
In2:	abc d e fgh
	ijk lmn
Exit:	1

# ... unless the -n switch is given
Name:	lines2
Run:	bin/judge-tok -n $1 $2 2>&1
In1:	abc   d
	e fgh  
	   ijk  lmn
In2:	abc d e fgh
	ijk lmn

# Trailing empty lines are also not OK
Name:	trail1
Run:	echo >>$1 && bin/judge-tok $1 $2 2>&1
In1:	abc d e fgh
	ijk lmn
In2:	abc d e fgh
	ijk lmn
Exit:	1

# ... unless -t is given
Name:	trail1
Run:	echo >>$1 && bin/judge-tok -t $1 $2 2>&1
In1:	abc d e fgh
	ijk lmn
In2:	abc d e fgh
	ijk lmn

# Differences in case are not
Name:	case1
Run:	bin/judge-tok $1 $2 2>&1
In1:	abc d e FGH
	IJK lmn
In2:	abc d e fgh
	ijk lmn
Exit:	1

# ... unless -i is given
Name:	case2
Run:	bin/judge-tok -i $1 $2 2>&1
In1:	abc d e FGH
	IJK lmn
In2:	abc d e fgh
	ijk lmn

# By default, we compare everything literal
Name:	real1
Run:	bin/judge-tok $1 $2 2>&1
In1:	0.1000001
	1.
	1e-50
	1230000001
In2:	0.1
	1.
	0
	1230000000
Exit:	1

# ... but if -r is given, we allow small differences
Name:	real2
Run:	bin/judge-tok -r $1 $2 2>&1
In1:	0.1000001
	1.
	1e-50
	1230000001
In2:	0.1
	1.
	0
	1230000000
