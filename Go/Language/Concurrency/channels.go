package main

import (
    "fmt"
    "time"
)

func count(thing string, ch chan<- string) {
    for i := 0; i < 5; i++ {
        time.Sleep(time.Millisecond * 500)
        ch <- thing
    }
    close(ch) // Close the channel when done
}

func main() {
    ch := make(chan string)

    go count("foo", ch)
    go count("bar", ch)

    for msg := range ch {
        fmt.Println(msg)
    }
}