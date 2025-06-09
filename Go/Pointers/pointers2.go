package main

import "fmt"

func main() {
    x := 10
    p := &x // p is a pointer to x

    fmt.Println("Value of x:", x)      // Output: Value of x: 10
    fmt.Println("Address of x:", p)    // Output: Address of x: [memory address]
    fmt.Println("Value at p:", *p)     // Output: Value at p: 10

    *p = 20 // Change the value at the address p points to

    fmt.Println("New value of x:", x)  // Output: New value of x: 20
}