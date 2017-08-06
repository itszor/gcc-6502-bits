GCC 6502 bits
=============

Build tools, tiny C library, etc. for gcc-6502 port.

Prerequisites
-------------

**Install the cc65 compiler suite**

Most Linux distributions have a prepackaged cc65, but often it is outdated. Should you run into issues, build the latest version from https://github.com/cc65/cc65.git

**Install the prerequisites for building GCC**

    For Debian-based distros:
    # apt-get build-dep gcc-4.8

    For RedHat-based distros:
    # yum install -y gcc gcc-c++ mpfr-devel gmp-devel libmpc-devel flex

**Install Boost development libraries**

For semi65x (the included simulator), you also need Boost development libraries:

    For Debian-based distros:
    # apt-get install libboost-dev libboost-regex-dev
    
    For RedHat-based distros:
    # yum install -y boost boost-devel

**Install DejaGNU (optional)**

For running the GCC regression tests you will need to have DejaGNU installed:

    For Debian-based distros:
    # apt-get install dejagnu
    
    For RedHat-based distros:
    # yum install -y dejagnu

Work in progress
----------------

This is a work-in-progress compiler, and many bugs remain. You can build code for semi65x (the included simulator), and limited support also exists for Acorn BBC Model B or Master computers (you may see some references to the C64, but that's not properly implemented yet). Adding support for other systems shouldn't be too hard - contact me if you'd like to try that!

Building
--------

Check out this repository, and check out the GCC repository proper as a subdirectory named "gcc-src":

    $ git clone https://github.com/puppeh/gcc-6502-bits.git
    $ cd gcc-6502-bits
    $ git clone https://github.com/puppeh/gcc-6502.git gcc-src

Now build by running the build.sh script, e.g. as:

    $ ./build.sh 2>&1 | tee build.log

The default location for cc65 binaries is in /usr (so ca65, ld65 and so on are present in /usr/bin/ca65, etc). If installed in another location, set the CC65_PATH variable to the path where the binaries exist. For example:

    $ CC65_PATH=/usr/local/cc65/bin build.sh ...

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

To compile for other supported systems, e.g. the BBC Master computer, you can use a command like:

    # 6502-gcc -mmach=bbcmaster -mcpu=65C02 -O2 hello.c -o hello

By default, the compiler will use a software stack that starts at a suitable location for use in MODE 7 (i.e. 0x7BFF downwards). To use another screen mode or stack address, set the stack top at link time like this:

    # 6502-gcc -mmach=bbcmaster -mcpu=65C02 -O2 hello.c -Wl,-D,__STACKTOP__=0x2fff -o hello

The program start address will be set to 0xe00 for the Master, or 0x1900 for the Model B. You can probably use other load addresses if you supply your own linker script.

To obtain a memory map of the final executable from the linker, use a command such as:

    # 6502-gcc [...] -Wl,-m,hello.map

The semi65x simulator can read this map file in order to display symbolic addresses in trace output, which might be helpful for debugging.

Running regression tests
------------------------

If you built semi65x, you can run GCC regression tests using:

    $ ./check.sh

Results will appear in a directory like testresults-*date*-*sequence*. Results so far (2014-04-16) should look something like:

```
                === gcc Summary ===

# of expected passes            62283
# of unexpected failures        4165
# of unexpected successes       1
# of expected failures          104
# of unresolved testcases       2543
# of unsupported tests          2340
```
