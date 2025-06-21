package main

import "fmt"

type Address struct {
    Street string
    City   string
}

type Person struct {
    Name    string
    Age     int
    Address // Embedded field
}

func main() {
    p := Person{
        Name:    "Eve",
        Age:     45,
        Address: Address{Street: "123 Main St", City: "Anytown"},
    }
    fmt.Println("Name:", p.Name)
    fmt.Println("Street:", p.Street) // Access embedded field directly
    fmt.Println("City:", p.City)
}