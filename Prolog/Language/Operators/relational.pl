?- 5 > 3.        % true
?- 5 < 10.       % true
?- X = 10, X =< 20.  % true, X is bound to 10
?- 7 =\= 8.      % true, because 7 is not equal to 8

= (Unifies two terms)
\= (Terms do not unify)
== (Strict equality, no variable bindings)
\== (Not strictly equal, no variable bindings)
< (Less than)
> (Greater than)
=< (Less than or equal to)
>= (Greater than or equal to)