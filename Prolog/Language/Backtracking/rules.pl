% Facts
parent(alice, bob).
parent(bob, dave).
parent(carol, eve).

% Rule
grandparent(X, Y) :- parent(X, Z), parent(Z, Y).

% ?- grandparent(alice, X).