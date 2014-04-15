GCC 6502 bits
=============

Build tools, tiny C library, etc. for gcc-6502 port.

Prerequisites
=============

You need to install cc65 in /usr (so ca65, ld65 and so on are present in /usr/bin/ca65 etc.) before building. On a
Debian system you should also have all the prerequisites for building GCC installed, e.g.:

  # apt-get build-dep gcc

or similar.

Work in progress
================

So far you can only build binaries for the included simulator (sim65x). Support for real machines may appear to due
course. (This is a matter of adding linker scripts and OS support as appropriate.)

Building
========

Check out this repository, and check out the GCC repository proper as a subdirectory named "gcc-src":

  $ git clone git@github.com:puppeh/gcc-6502-bits.git
  $ cd gcc-6502-bits
  $ git clone git@github.com:puppeh/gcc-6502.git gcc-src

Now build by running the build.sh script, e.g. as:

  $ ./build.sh 2>&1 | tee build.log

After a while, you should have a 6502 cross-compiler in a directory named 'prefix'.

Running code
============

Set your PATH to include prefix/bin:

  $ export PATH=`pwd`/prefix/bin:$PATH

Now use the compiler like any other cross-compiler:

  $ 6502-gcc helloworld.c -O2 -o helloworld

If you build the enclosed simulator (needs Boost, cd semi65x && make) you can run the generated code:

  $ semi65x/semi65x -l 0x200 ./helloworld
  Hello world
  $

Running regression tests
========================

If you built semi65x, you can run GCC regression tests using:

  $ ./check.sh

Results will appear in a directory like testresults-$(date)-$(sequence).
