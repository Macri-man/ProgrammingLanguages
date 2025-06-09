use tokio;

async fn task1() {
    println!("Task 1 is running");
}

async fn task2() {
    println!("Task 2 is running");
}

#[tokio::main]
async fn main() {
    let handle1 = tokio::spawn(task1());
    let handle2 = tokio::spawn(task2());

    let _ = tokio::join!(handle1, handle2); // Run tasks in parallel
}