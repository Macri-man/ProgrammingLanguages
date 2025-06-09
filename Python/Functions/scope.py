global_var = 10

def modify_global():
    global global_var
    global_var += 5

modify_global()
print(global_var) 



def outer():
    x = 10

    def inner():
        nonlocal x  
        x = 20      
        print(f"Inner x: {x}")

    inner()
    print(f"Outer x: {x}")