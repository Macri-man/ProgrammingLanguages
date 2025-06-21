package main

import "fmt"

func main() {
	// Initialize a map
	myMap := make(map[string]int)

	// Add elements to the map
	myMap["apple"] = 5
	myMap["banana"] = 3
	myMap["cherry"] = 7

	// Retrieve and print an element
	quantity := myMap["banana"]
	fmt.Println("Quantity of bananas:", quantity) // Output: Quantity of bananas: 3

	// Check for existence
	value, exists := myMap["apple"]
	if exists {
		fmt.Println("Apple exists with quantity:", value) // Output: Apple exists with quantity: 5
	} else {
		fmt.Println("Apple does not exist")
	}

	// Delete an element
	delete(myMap, "cherry")

	// Iterate over the map
	for key, value := range myMap {
		fmt.Println("Key:", key, "Value:", value)
	}
}
