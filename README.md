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
* Day 12 implements the worst queue you'll ever see.
* Day 13 uses a recursive parser that generates nested objects.
* Day 14 makes AOC a bit boring. Maybe the last I'll do? Note that here it was not needed to simulate every sand grain again, ad I did. It is possible to remember the last position of the previous sand grain before it rested and continue from there. But the execution time would not change *practically*, the problem is too small.
* Day 15 must show, otherwise no way to solve part 2, that it is possible to get smart when the search space is huge.
* Day 16 Worst solution so far (back home saturday night, half drunk, can't do better than that): shows how a terrible heuristic can help you brute-force the solution. Warning: the program is quite broken and works for sure only on my `input.txt`, that you can find inside the directory.
* Day 17 Skipped Tetris because of boredom & weekend.
* Day 18 3D flood filling with recursive flood() function.
