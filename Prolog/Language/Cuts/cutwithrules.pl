age_category(Age, child) :- Age < 12, !.
age_category(Age, teenager) :- Age < 18, !.
age_category(_, adult).

?- age_category(10, Category).
Category = child.