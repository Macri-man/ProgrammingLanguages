reverse_list([], []). % Base case: The reverse of an empty list is an empty list
reverse_list([H | T], Rev) :-
    reverse_list(T, RevT), % Recursively reverse the tail
    append(RevT, [H], Rev). % Append the head to the reversed tail

?- reverse_list([1, 2, 3], Rev).
Rev = [3, 2, 1].