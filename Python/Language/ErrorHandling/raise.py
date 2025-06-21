def divide(a, b):
    if b == 0:
        raise ValueError("The denominator cannot be zero.")
    return a / b

try:
    result = divide(10, 0)
except ValueError as e:
    print(e)