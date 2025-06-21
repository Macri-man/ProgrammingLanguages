% Facts
parent(alice, bob).     % Atoms as terms
parent(bob, carol).

% Compound Terms
likes(john, pizza).     % Compound term
location(office, buildingA, cityX).

% Lists
friends([alice, bob, carol]).
likes_multiple(john, [pizza, sushi, pasta]).

% Variables in Rules
grandparent(X, Z) :- parent(X, Y), parent(Y, Z).