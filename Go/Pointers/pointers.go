package main

import "fmt"

func main() {

	i := 1
	fmt.Println(i)

	var ptr *int = &i

	fmt.Println(ptr)

	fmt.Println(*ptr)

	*ptr = 5

	fmt.Println(i)

}
