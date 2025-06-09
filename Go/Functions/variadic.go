package main

import "fmt"

// Variadic function that sums up all its arguments
func sum(numbers ...int) int {
    total := 0
    for _, number := range numbers {
        total += number
    }
    return total
}

func concat(pieces ...string) string {
	return strings.Join(pieces, ",")
}


func main() {
		
    result := sum(1, 2, 3, 4, 5)
    fmt.Println("Sum:", result) // Output: Sum: 15
}