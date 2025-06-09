package main

import "fmt"

func describe(i interface{}) {
	if str, ok := i.(string); ok {
		fmt.Println("String:", str)
	} else if num, ok := i.(int); ok {
		fmt.Println("Integer:", num)
	} else {
		fmt.Println("Unknown type")
	}
}

func main() {
	describe("hello")
	describe(42)
	describe(3.14)
}
