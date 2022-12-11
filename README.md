Just a couple of Advent of Code puzzles solved in C. When possible I try to use non obvious ways to solve the puzzle, to make things more creative:

* Day 1 makes use of a simple streaming algorithm to retain top-N max entries.
* Day 2 turns the logic of Rock, Paper, Scissor into a simple equation modulo N.
* Day 3 makes use of bitmaps.
* Day 4 shows abstracting with C structures.
* Day 5 shows linked lists are useful.
* Day 6 will solve in `O(N)` instead of `O(N*M)`.
* Day 7 uses recursion and building ad-hoc data structures.
* Day 8 walks data structures using a velocity vector.
* Day 9 has nothing special. Just plain C code to simulate the rope.
* Day 10 Simple but fun. Nothing special. Hints at cycles-exact emulation.
* Day 11 Is an example of how you should not do parsing, assuming tons of stuff about your input. The program uses the LCM to avoid overflow. Yet I had to change int to long, and I did it brutally in `2.c`, with vim search and replace.
