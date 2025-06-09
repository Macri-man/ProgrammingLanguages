enum Option<T> {
    Some(T),
    None,
}

fn find_item(index: usize) -> Option<&'static str> {
    let items = ["apple", "banana", "cherry"];
    if index < items.len() {
        Some(items[index])
    } else {
        None
    }
}

fn main() {
    match find_item(1) {
        Some(item) => println!("Found: {}", item),
        None => println!("Item not found"),
    }
}