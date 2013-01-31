# Makefile for Perl MakeMaker  (c) 2003 Tomas Valla <tom@ucw.cz>

use ExtUtils::MakeMaker;
WriteMakefile(
    'NAME'		=> 'UCW::Ulimit',
    'VERSION_FROM'	=> 'Ulimit.pm',
    'INST_LIB'		=> 'lib',
    'INST_ARCHLIB'	=> 'arch',
);
