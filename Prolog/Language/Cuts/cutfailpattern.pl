test(X) :- X = 1, !, fail.
test(X) :- X = 2.

% ?- test(1).
% false.