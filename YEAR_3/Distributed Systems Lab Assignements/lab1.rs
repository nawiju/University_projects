pub struct Fibonacci {
    current: u128,
    next: u128,
}

impl Fibonacci {
    pub fn new() -> Fibonacci {
        Fibonacci {
            current: 0,
            next: 1,
        }
    }

    pub fn fibonacci(n: usize) -> u8 {
        let mut a: u8 = 0;
        let mut b: u8 = 1;

        for _ in 0..n {
            let temp = a.wrapping_add(b);
            a = b;
            b = temp;
        }

        a
    }
}

impl Iterator for Fibonacci {
    type Item = u128;

    fn next(&mut self) -> Option<Self::Item> {
        let result = self.current;

        if let Some(next_fib) = self.current.checked_add(self.next) {
            self.current = self.next;
            self.next = next_fib;
            Some(result)
        } else {
            None
        }
    }
}

