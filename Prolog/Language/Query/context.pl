% Facts
parent(alice, bob).
parent(bob, carol).
parent(alice, dave).
parent(dave, eve).

% Rules
grandparent(X, Z) :- parent(X, Y), parent(Y, Z).
ancestor(X, Z) :- parent(X, Z).
ancestor(X, Z) :- parent(X, Y), ancestor(Y, Z).
