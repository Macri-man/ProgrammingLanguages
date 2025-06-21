% Without a cut
max(X, Y, X) :- X >= Y.
max(_, Y, Y).

%?- max(5, 3, Max).
% Max = 3  incorrect

% with a cut
max2(X, Y, X) :- X >= Y, !.
max2(_, Y, Y).

% ?- max2(5, 3, Max).
% Max = 5.