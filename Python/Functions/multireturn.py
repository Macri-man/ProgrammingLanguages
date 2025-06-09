def get_user_info():
    name = "Alice"
    age = 30
    city = "New York"
    return name, age, city

user_name, user_age, user_city = get_user_info()
print(f"Name: {user_name}")  
print(f"Age: {user_age}")  
print(f"City: {user_city}")  

def calculate_sum_and_product(x, y):
    sum_result = x + y
    product_result = x * y
    return sum_result, product_result

sum_value, product_value = calculate_sum_and_product(3, 4)
print(f"Sum: {sum_value}")        
print(f"Product: {product_value}") 