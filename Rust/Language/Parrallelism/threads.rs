use std::thread;

fn main() {
    let handle1 = thread::spawn(|| {
        for i in 1..10 {
            println!("Thread 1: {}", i);
        }
    });

    let handle2 = thread::spawn(|| {
        for i in 1..10 {
            println!("Thread 2: {}", i);
        }
    });

    handle1.join().unwrap(); // Wait for thread 1 to finish
    handle2.join().unwrap(); // Wait for thread 2 to finish
}
