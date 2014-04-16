GCC 6502 bits
=============

Build tools, tiny C library, etc. for gcc-6502 port.

Prerequisites
-------------

You need to install cc65 in /usr (so ca65, ld65 and so on are present in /usr/bin/ca65 etc.) before building. On a
Debian system you should also have all the prerequisites for building GCC installed, e.g.:

    # apt-get build-dep gcc-4.8

or similar.

For semi65x (the included simulator), you also need Boost development libraries:

    # apt-get install libboost-dev libboost-regex-dev

Work in progress
----------------

So far you can only build binaries for the included simulator (semi65x). Support for real machines may appear in due
course. (This is a matter of adding linker scripts and OS support as appropriate.)

Building
--------

Check out this repository, and check out the GCC repository proper as a subdirectory named "gcc-src":

    $ git clone git@github.com:puppeh/gcc-6502-bits.git
    $ cd gcc-6502-bits
    $ git clone git@github.com:puppeh/gcc-6502.git gcc-src

Now build by running the build.sh script, e.g. as:

    $ ./build.sh 2>&1 | tee build.log

After a while, you should have a 6502 cross-compiler in a directory named 'prefix'.

Running code
------------

Set your PATH to include prefix/bin:

    $ export PATH=`pwd`/prefix/bin:$PATH

Now use the compiler like any other cross-compiler:

    $ 6502-gcc helloworld.c -O2 -o helloworld

If you build the enclosed simulator (needs Boost, cd semi65x && make) you can run the generated code:

    $ semi65x/semi65x -l 0x200 ./helloworld
    Hello world
    $

Running regression tests
------------------------

If you built semi65x, you can run GCC regression tests using:

    $ ./check.sh

Results will appear in a directory like testresults-$(date)-$(sequence). Results so far (2014-04-16) should look something like:

```
                === gcc Summary ===

# of expected passes            62283
# of unexpected failures        4165
# of unexpected successes       1
# of expected failures          104
# of unresolved testcases       2543
# of unsupported tests          2340
```
