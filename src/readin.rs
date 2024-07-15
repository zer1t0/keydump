use std::fs::File;
use std::io::Lines;
use std::io::{self, BufRead, BufReader, Stdin};

/// Function to read the inputs from files, strings or stdin
/// in a normalized iterator of strings.
pub fn read_inputs(
    inputs: Vec<String>,
    use_stdin_if_none: bool,
    use_stdin_if_minus: bool,
) -> impl Iterator<Item = String> {
    let input_iter: Box<dyn Iterator<Item = String>>;
    input_iter = if inputs.len() == 0 && use_stdin_if_none {
        Box::new(StdinIter::new())
    } else {
        Box::new(FileStringIter::new(inputs, use_stdin_if_minus))
    };
    return input_iter
        .map(|s| s.trim().to_string())
        .filter(|s| s.len() != 0 && !s.starts_with("#"));
}

/// Class to get an iterator of stdin lines, since
/// io::stdin().lock().lines() gives problems with lifetimes.
pub struct StdinIter {
    stdin: Stdin,
}

impl StdinIter {
    pub fn new() -> Self {
        Self::default()
    }
}

impl Iterator for StdinIter {
    type Item = String;

    fn next(&mut self) -> Option<Self::Item> {
        let mut line = String::new();
        match self.stdin.read_line(&mut line) {
            Ok(n) => {
                if n == 0 {
                    // EOF
                    return None;
                }
                return Some(line);
            }
            Err(_) => return None,
        }
    }
}

impl Default for StdinIter {
    fn default() -> Self {
        Self { stdin: io::stdin() }
    }
}

/// Class to read a bunch of strings that could be filenames.
/// If string is a valid path, then the lines of the file are retrieved,
/// in other case, the string itself is retrieved.
pub struct FileStringIter {
    items: Vec<String>,
    lines: Option<Lines<BufReader<File>>>,
    stdin: Option<Stdin>,
    using_stdin: bool,
}

impl FileStringIter {
    pub fn new(mut items: Vec<String>, use_stdin_if_minus: bool) -> Self {
        items.reverse();
        let stdin = if use_stdin_if_minus {
            Some(io::stdin())
        } else {
            None
        };
        return Self {
            items,
            lines: None,
            stdin,
            using_stdin: false,
        };
    }
}

impl Iterator for FileStringIter {
    type Item = String;

    fn next(&mut self) -> Option<String> {
        loop {
            if let Some(lines) = &mut self.lines {
                if let Some(line) = lines.next() {
                    return Some(line.expect("Error reading input"));
                } else {
                    self.lines = None;
                }
            }

            if self.using_stdin {
                let mut line = String::new();
                match self.stdin.as_ref().unwrap().read_line(&mut line) {
                    Ok(n) => {
                        if n == 0 {
                            self.using_stdin = false;
                        } else {
                            return Some(line);
                        }
                    }
                    Err(_) => return None,
                }
            }

            let item = self.items.pop()?;

            match File::open(&item) {
                Ok(file) => {
                    self.lines = Some(BufReader::new(file).lines());
                }
                Err(_) => {
                    if self.stdin.is_some() && item == "-" {
                        self.using_stdin = true;
                    } else {
                        return Some(item);
                    }
                }
            }
        }
    }
}
