?- 5 > 3, 8 > 5. % Conjunction (AND)
% true, both conditions are true

?- 5 > 3; 2 > 5. % Disjunction (OR):
% true, at least one condition is true

?- 5 > 3 -> write('True'); write('False'). % If-Then (->):
% Writes 'True' because 5 > 3 is true

?- false *-> write('True'); write('False'). % Soft If-Then (*->):
% Writes 'False', but the query does not fail