% Red Cut: Used to change the logical meaning of the program by eliminating unwanted alternatives, often leading to different results compared to a version without the cut.

eligible(Person) :- student(Person), !.
eligible(Person) :- employed(Person).