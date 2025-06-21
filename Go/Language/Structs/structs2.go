package main

import "fmt"

type Person struct {
    Name string
    Age  int
}

func main() {
		//literal
    p := Person{Name: "Alice", Age: 30}
    fmt.Println(p)
		//use new keyword
		p2 := new(Person) // p is a pointer to a Person struct
    p2.Name = "Bob"
    p2.Age = 25
    fmt.Println(*p2) // Dereference the pointer to access the struct
		//pointer
		p3 := &Person{Name: "Charlie", Age: 35}
    fmt.Println(p3.Name) // Access fields directly
		//access and modify
		p4 := Person{Name: "Diana", Age: 40}
    fmt.Println("Name:", p4.Name)
    fmt.Println("Age:", p4.Age)

    p4.Age = 41
    fmt.Println("Updated Age:", p4.Age)
}