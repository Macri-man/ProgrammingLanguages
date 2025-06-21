package main

import "fmt"

func main() {

	for i := range 3 {
		fmt.Println("range", i)
	}

	for n := range 6 {
		if n%2 == 0 {
			continue
		}
		fmt.Println(n)
	}

	stringslice := []string{"Hello", "today", "tomorrow"}
	for index, value := range stringslice {
		fmt.Printf("Index %d, Value %s \n", index, value)
	}

	numbers := []int{1, 2, 3, 4}
	for _, value := range numbers {
		fmt.Println(value)
	}

}
