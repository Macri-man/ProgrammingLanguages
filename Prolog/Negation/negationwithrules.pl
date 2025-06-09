% Facts
adult(john).
adult(mary).
citizen(john).
citizen(mary).
convicted_felon(john).

% Rule
eligible_to_vote(Person) :-
    adult(Person),
    citizen(Person),
    \+ convicted_felon(Person).

% ?- eligible_to_vote(mary).
% true.

% ?- eligible_to_vote(john).
% false.