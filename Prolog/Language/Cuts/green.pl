% Green Cut: Used to optimize the search by eliminating unnecessary backtracking, without changing the logical meaning of the program.

max(X, Y, X) :- X >= Y, !. % This is a green cut
max(_, Y, Y).