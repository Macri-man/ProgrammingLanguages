use std::sync::mpsc;
use std::thread;

fn main() {
    let (tx, rx) = mpsc::channel(); // Create a channel

    thread::spawn(move || {
        let message = String::from("Hello from thread");
        tx.send(message).unwrap(); // Send the message
    });

    let received = rx.recv().unwrap(); // Receive the message
    println!("Received: {}", received);
}
