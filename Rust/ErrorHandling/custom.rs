use std::fmt;

#[derive(Debug)]
enum MyError {
    NotFound,
    Unauthorized,
}

impl fmt::Display for MyError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{:?}", self)
    }
}

impl std::error::Error for MyError {}

fn perform_action(user_id: usize) -> Result<(), MyError> {
    if user_id == 0 {
        Err(MyError::Unauthorized)
    } else {
        Ok(())
    }
}

fn main() {
    match perform_action(0) {
        Ok(_) => println!("Action performed successfully"),
        Err(e) => println!("Error: {}", e),
    }
}