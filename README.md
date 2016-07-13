# tbostrings
Portable strings implementation for dumping printable ASCII/UNICODE strings from a given file in a single pass.

It is the equivalent to running the UNIX strings program multiple times with the following switches, and should be much faster:

- strings -s
- strings -el
- strings -eL
- strings -eb
- strings -eB

