def outer_function(name):
    def inner_function1():
        print(f"Hello from inner function1 {name}")
    
    def inner_function2():
        print("Hello from inner function2")
    
    inner_function1()
    inner_function2()