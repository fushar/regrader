# Makefile for Perl MakeMaker  (c) 2007 Pavel Charvat <pchar@ucw.cz>

use ExtUtils::MakeMaker;
WriteMakefile(
    'NAME'		=> 'UCW::Filelock',
    'VERSION_FROM'	=> 'Filelock.pm',
    'INST_LIB'		=> 'lib',
    'INST_ARCHLIB'	=> 'arch',
);
