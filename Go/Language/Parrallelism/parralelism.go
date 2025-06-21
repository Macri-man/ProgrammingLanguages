package main

import (
	"fmt"
	"runtime"
	"sync"
	"time"
)

func doWork(id int, wg *sync.WaitGroup) {
	defer wg.Done()
	fmt.Printf("Worker %d started\n", id)
	time.Sleep(2 * time.Second) // Simulate work
	fmt.Printf("Worker %d finished\n", id)
}

func main() {
	runtime.GOMAXPROCS(4) // Set the number of CPU cores to 4

	var wg sync.WaitGroup

	for i := 1; i <= 8; i++ {
		wg.Add(1)
		go doWork(i, &wg)
	}

	wg.Wait()
	fmt.Println("All workers completed")
}
