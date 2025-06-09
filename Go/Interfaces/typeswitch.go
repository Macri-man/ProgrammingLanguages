package main

import "fmt"

func typeSwitch(i interface{}) {
	switch v := i.(type) {
	case string:
		fmt.Println("String:", v)
	case int:
		fmt.Println("Integer:", v)
	default:
		fmt.Println("Unknown type")
	}
}

func main() {
	typeSwitch("hello")
	typeSwitch(42)
	typeSwitch(3.14)
}
