head_and_tail([H | T], H, T).

?- head_and_tail([1, 2, 3], H, T).
H = 1,
T = [2, 3].