data_processors_coding_exercise
===============================

Partial solution to Data Processors coding exercise

This is a depth-first solution to the DAG exploration part of the problem, written in C++. It doesn't address the socket stuff because that's fiddly and time-consuming to do in C/C++ (is there really no solution out there? Must look harder).

I have a feeling that a breadth-first solution might scale better (only a feeling though).

Watch for a Ruby or Python solution, if only because there's probably a handy gem or module or something to do the socket stuff.

Of course to test all this properly I need to provide a server as well, but for now the code reads an input file and implements a crude inline "server".
