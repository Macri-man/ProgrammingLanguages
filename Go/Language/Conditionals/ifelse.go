package main

import "fmt"

func main() {

	if true {
		fmt.Println("true")
	} else {
		fmt.Println("false")
	}

	if 3%2 == 0 {
		fmt.Println("3 is even")
	} else {
		fmt.Println("3 is odd")
	}

	if 2%2 == 0 || 1%2 == 0 && 0 == 0 {
		fmt.Println("either 2 or 1 are even and 0 equals 0")
	}

	if num := 9; num < 0 {
		fmt.Println(num, "is negative")
	} else if num < 10 {
		fmt.Println(num, "has 1 digit")
	} else {
		fmt.Println(num, "has multiple digits")
	}

}
