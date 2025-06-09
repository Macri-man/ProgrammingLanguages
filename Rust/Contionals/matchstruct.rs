struct Point {
    x: i32,
    y: i32,
}

fn main() {
    let point = Point { x: 5, y: 10 };

    match point {
        Point { x: 0, y } => println!("On the y-axis at y: {}", y),
        Point { x, y: 0 } => println!("On the x-axis at x: {}", x),
        Point { x, y } => println!("Point at ({}, {})", x, y),
    }
}