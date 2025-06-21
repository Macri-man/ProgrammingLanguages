

x = 10
x = "hello"
x = [1,2,3]


a = 10               # a is an integer
print(type(a))       # Output: <class 'int'>

a = "Dynamic typing" # a is now a string
print(type(a))       # Output: <class 'str'>


def add_numbers(x: int, y: int) -> int:
    return x + y


x = "Hello"

print(type(x))  # Output: <class 'str'>


if isinstance(x, str):
    print("x is a string")


# Types

x = 10 # int
x = 10.5 # float
x = 2 +3j # complex number

x = "hello" # string
x = [1,2,"apple"] # list
x = (1,3,"peaches") # tuple
x = range(0,10) # range


x = {"name":"Fred","age":30} # Dictionary
x = {1,2,3,4} # Set


x = True # Boolean

x = b"hello" # Bytes immutable
x = bytearray(b"hello") # Bytes mutable

x = memoryview(bytes(5)) # view of memory of an object

x = None # None 

def foo():
    return "hello"

callable(foo) # Check if callable

x = 10 
type(x) # check type

x = "10"
y = int(x) # covert Type

class Person:
    def __init__(self, name, age):
        self.name = name
        self.age = age

p = Person("George", 15)
print(type(p))  # check type of custom class


from enum import Enum

class Color(Enum):
    RED = 1
    GREEN = 2
    BLUE = 3

# Accessing enum members
print(Color.RED)        # Output: Color.RED
print(Color.RED.name)   # Output: RED
print(Color.RED.value)  # Output: 1

# Comparison
if Color.RED == Color(1):
    print("Red is 1")   # Output: Red is 1


# Structs
from dataclasses import dataclass

@dataclass
class Point:
    x: int
    y: int

# Creating an instance of Point
p = Point(3, 4)

# Accessing attributes
print(p.x)  # Output: 3
print(p.y)  # Output: 4

# Automatic __repr__ method
print(p)    # Output: Point(x=3, y=4)

class Point:
    def __init__(self, x, y):
        self.x = x
        self.y = y

# Creating an instance of Point
p = Point(3, 4)

# Accessing attributes
print(p.x)  # Output: 3
print(p.y)  # Output: 4