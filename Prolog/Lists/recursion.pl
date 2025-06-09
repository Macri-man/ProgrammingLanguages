sum_list([], 0). % Base case: The sum of an empty list is 0
sum_list([H | T], Sum) :-
    sum_list(T, TailSum), % Recursively sum the tail
    Sum is H + TailSum.   % Add the head to the sum of the tail

?- sum_list([1, 2, 3, 4], Sum).
Sum = 10.

even(X) :- X mod 2 =:= 0.

filter_even([], []). % Base case: Empty list
filter_even([H | T], [H | Evens]) :-
    even(H), % If H is even, keep it in the list
    filter_even(T, Evens).
filter_even([_ | T], Evens) :- 
    filter_even(T, Evens). % If H is odd, skip it

?- filter_even([1, 2, 3, 4, 5], Evens).
Evens = [2, 4].