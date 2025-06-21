
package main

import (
    "fmt"
    "errors"
)


// function with parameters and multiple return values of same type

func quotientremainder(dividend, divisor int) (int, int) {
	quotient := dividend / divisor
	remainder := dividend % divisor
	return quotient, remainder
}


// function with parameters and multiple return values of different type
func info(num int) (string, int, bool) {
	if num == 1 {
		return "One", 1, true
	}

	if num == 5 {
		return "Hello John", 15, false
	}

	return "Wrong", 0, false
}


func divides(a, b float64) (float64, error) {
	if b == 0 {
		return 0, errors.New("division by zero Error")
	}

	return a / b, nil
}



func main() {
    result, err := divide(10, 2)
    if err != nil {
        fmt.Println("Error:", err)
    } else {
        fmt.Println("Result:", result) // Output: Result: 5
    }
}