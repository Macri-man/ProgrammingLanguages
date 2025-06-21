?- X = 10.
% X is unified with 10

?- X = alice, X == alice.
% true, strict equality check

?- X = alice, X \= bob.
% true, because alice is not equal to bob

= (Unifies two terms)
\= (Terms do not unify)
== (Strict equality, no variable bindings)
\== (Strict inequality, no variable bindings)

The difference between = and ==:

= tries to unify variables and terms.
== checks for strict equality without attempting to bind variables.