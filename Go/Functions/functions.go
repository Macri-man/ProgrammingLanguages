package main

import (
	"errors"
	"fmt"
	"strings"
)

// function with no parameters and no return value
func printSomething() {
	fmt.Println("Something")
}

// function with parameters and one return value

func mathEquation(a, b, c int) int {
	return a + b - c
}

// function with parameters and multiple return values of same type

func quotientremainder(dividend, divisor int) (int, int) {
	quotient := dividend / divisor
	remainder := dividend % divisor
	return quotient, remainder
}

// function with varadic parameters

func concat(pieces ...string) string {
	return strings.Join(pieces, ",")
}

func sum(numbers ...int) int {
	total := 0
	for _, num := range numbers {
		total += num
	}
	return total
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

// function with a function parameter and return values

func applyFunction(a, b int, function func(int, int) int) int {
	return function(a, b)
}

// lambda function or  anonymous function

var appendHello = func(x string) string {
	return x + "hello"
}

// Closures

func nextInt() func() int {
	i := 0
	return func() int {
		i++
		return i
	}
}

// Recursion
func fibonacci(num int) int {
	if num < 2 {
		return num
	}

	return fibonacci(num-1) + fibonacci(num-2)
}

func main() {

	fmt.Println()
}
