length/2: Determines the length of a list.
?- length([1, 2, 3], L).
L = 3.

member/2: Checks if an element is a member of a list.
?- member(2, [1, 2, 3]).
true.

append/3: Concatenates two lists.
?- append([1, 2], [3, 4], L).
L = [1, 2, 3, 4].

select/3: Selects and removes an element from a list.
?- select(2, [1, 2, 3], L).
L = [1, 3].

nth1/3: Accesses the N-th element of a list (1-based indexing).
?- nth1(2, [a, b, c], X).
X = b.