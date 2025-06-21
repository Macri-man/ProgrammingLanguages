class CustomError(Exception):
    pass

try:
    raise CustomError("This is a custom exception.")
except CustomError as e:
    print(e)