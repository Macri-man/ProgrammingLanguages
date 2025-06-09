% Facts
male(bob).
female(alice).
parent(alice, bob).

% Query
?- male(X), parent(alice, X).