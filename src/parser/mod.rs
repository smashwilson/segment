use pest::Parser;

#[derive(Parser)]
#[grammar = "parser/grammar.pest"]
struct Language;

pub fn parse(input: &str) -> bool {
    let result = Language::parse(Rule::source, input);
    println!("{:?}", result);
    result.is_ok()
}
