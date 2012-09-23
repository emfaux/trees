Space conservative red black tree
=====

Implementation of a red black tree that uses the least significant bit
of parent pointers to store the color of nodes. This has the advantage
that it removes the need to waste memory on keeping track of the color
of nodes.

The implementation relies on the assumption that allocations are word
aligned and thereby leaves us a tiny bit of space at the lower end of
the pointer to store additional information. While this can't be used
for anything other than a few bits of information it is sufficient for
binary states such as node color.

The implementation is written in C with a subset of C++ features that
can easily be removed without changing the structure of the code.
