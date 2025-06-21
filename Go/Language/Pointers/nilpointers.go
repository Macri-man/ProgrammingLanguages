package main

import "fmt"

func main() {
    var p *int
    if p == nil {
        fmt.Println("p is nil")
    }

    // Uncommenting the next line will cause a runtime error
    // fmt.Println(*p)
}