trait Drawable {
    fn draw(&self);
}

struct Circle {
    radius: f64,
}

impl Drawable for Circle {
    fn draw(&self) {
        println!("Drawing a circle with radius {}", self.radius);
    }
}

fn main() {
    let circle = Circle { radius: 5.0 };
    circle.draw(); // Prints: Drawing a circle with radius 5
}