max(X, Y, X) :- X >= Y, !.
max(_, Y, Y).

?- max(5, 3, Max).
Max = 5.