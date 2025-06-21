fn main() {
    let change = if num < 5 || num > 10 {
        println!("Change is Coming!");
        num * 5
    } else {
        println!("No Change");
        num
    };

    println!("{} -> {}", num, change)
}
