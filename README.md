### veer - ECS using reflection in C++26

I am too lazy right now to right up a document.

The system is very flexible, allowing the same property (component) to be of different types across tables. Elements can be packed tightly or padded to fill a cache line without aliasing. Tables can be iterated on chosen properties easily.

Example found in ./tests/arena.

Can only be built with GCC16 for now since it is the only compiler that currently supports reflection.

