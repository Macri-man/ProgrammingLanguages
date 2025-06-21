:- op(Precedence, Type, Operator).

:- op(500, xfx, likes).
% This defines 'likes' as an infix operator

?- john likes pizza.
% Equivalent to likes(john, pizza).

Precedence: A number (1–1200) indicating the precedence level.
Type: The associativity of the operator (xfx, xfy, etc.).
Operator: The name of the operator.