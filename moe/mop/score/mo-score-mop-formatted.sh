#!/bin/sh

mop/mo-score-mop 3 2 policie rybka | column -c120 -ts"	" | cstocs il2 ascii
