fn draw_all(items: &mut [Box<dyn Drawable>]) {
    for item in items {
        item.draw();
    }
}