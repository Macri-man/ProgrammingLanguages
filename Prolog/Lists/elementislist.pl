member(X, [X | _]). % X is the head
member(X, [_ | T]) :- member(X, T). % X is in the tail

?- member(2, [1, 2, 3]).
true.

?- member(5, [1, 2, 3]).
false.