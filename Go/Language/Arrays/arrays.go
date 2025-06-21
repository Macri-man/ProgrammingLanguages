package main

import "fmt"

func main() {

	var arr [5]int
	fmt.Println("Empty Array:", arr)

	arr[4] = 100
	fmt.Println("set:", arr)
	fmt.Println("get:", arr[4])

	fmt.Println("len:", len(arr))

	DeclareArray1 := [6]int{1, 2, 3, 4, 5, 6}
	fmt.Println(DeclareArray1)
	DeclareArray2 := [...]int{1, 2, 3, 4, 5, 6}
	fmt.Println(DeclareArray2)

	DeclareArray3 := [...]int{1, 3: 4, 5, 6}
	fmt.Println(DeclareArray3)

	var Declare2DArray1 [2][3]int
	for i := 0; i < 2; i++ {
		for j := 0; j < 3; j++ {
			Declare2DArray1[i][j] = i + j
		}
	}

	fmt.Println("2D: ", Declare2DArray1)

	Declare2DArray2 := [2][3]int{
		{1, 2, 3},
		{1, 2, 3},
	}

	fmt.Println("2d: ", Declare2DArray2)

}
