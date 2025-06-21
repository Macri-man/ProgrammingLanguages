use proc_macro;

#[proc_macro]
pub fn hello_macro(input: proc_macro::TokenStream) -> proc_macro::TokenStream {
    let input_str = input.to_string();
    let output = format!("Hello, {}!", input_str);
    output.parse().unwrap()
}