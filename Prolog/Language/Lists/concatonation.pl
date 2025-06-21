concatenate([], L, L).
concatenate([H | T], L2, [H | L3]) :-
    concatenate(T, L2, L3).

?- concatenate([1, 2], [3, 4], L).
L = [1, 2, 3, 4].