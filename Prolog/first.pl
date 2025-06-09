% Facts
parent(alice, bob).
parent(bob, carol).
parent(alice, dave).
parent(dave, eve).

% Rule to determine grandparent
grandparent(X, Z) :-
    parent(X, Y),
    parent(Y, Z).

% [first]
% grandparent(alice, Who).