% Facts
bird(sparrow).
bird(pigeon).
mammal(tiger).
mammal(elephant).

% Rule
% not_a_bird(X) :- \+ bird(X).

% ?- not_a_bird(tiger).
% true.

% ?- not_a_bird(sparrow).
% false.