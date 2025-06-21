% \+/1 is used to check if a goal cannot be proven true.
% \+ (Negation, pronounced "not")

?- \+ (5 > 10).
% true, because 5 is not greater than 10

?- \+ parent(alice, charlie).
% true, if there is no fact stating that Alice is Charlie's parent