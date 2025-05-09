// WARNING: Do not modify definitions of public types or function names in this
// file – your solution will be tested automatically! Implement all missing parts.

use crossbeam_channel::{unbounded, Receiver, Sender};
use rand::Rng;
use std::thread;
use std::thread::JoinHandle;

type Num = u64;
type Ident = u128;

pub(crate) struct FibonacciModule {
    /// Currently held number from the sequence.
    num: Num,
    /// Index of the required Fibonacci number (the `n`).
    limit: usize,
    /// Identifier of the module.
    id: Ident,
    /// Identifier of the other module.
    other: Option<Ident>,
    /// Queue for outgoing messages.
    queue: Sender<FibonacciSystemMessage>,
}

impl FibonacciModule {
    /// Create the module and register it in the system.
    pub(crate) fn create(
        initial_number: Num,
        limit: usize,
        queue: Sender<FibonacciSystemMessage>,
    ) -> Ident {
        // For the sake of simplicity, generate a random number and use it
        // as the module's identifier:
        let id: u128 = rand::thread_rng().gen();

        let registration = FibonacciSystemMessage::RegisterModule(FibonacciModule {
            num: initial_number,
            limit,
            id,
            other: None,
            queue: queue.clone(),
        });
        
        queue.send(registration).unwrap();
        id
    }

    /// Handle the step-message from the other module.
    ///
    /// Here the next number of the Fibonacci sequence is calculated.
    pub(crate) fn message(&mut self, idx: usize, num: Num) {
        if idx >= self.limit {
            // The calculation is done.
            self.queue.send(FibonacciSystemMessage::Done).unwrap();
            return;
        }

        self.num += num;

        // Put the following `println!()` statement after performing
        // the update of `self.num`:
        println!("Inside {}, value: {}", self.id, self.num);

        // Send the message to the other module:
        self.queue
            .send(FibonacciSystemMessage::Message {
                id: self.other.unwrap(),
                idx: idx + 1,
                num: self.num,
            })
            .unwrap();
    }

    /// Handle the init-message.
    ///
    /// The module finishes its initialization and initiates the calculation
    /// if it is the first to go.
    pub(crate) fn init(&mut self, other: Ident) {
        self.other = Some(other);      

        if self.num == 1 {
            // The first module initiates the calculation.
            self.queue
                .send(FibonacciSystemMessage::Message {
                    id: self.other.unwrap(),
                    idx: 1,
                    num: 1,
                })
                .unwrap();
        } 
    }
}

/// Messages sent to/from the modules.
///
/// The `id` field denotes which module should receive the message.
pub(crate) enum FibonacciSystemMessage {
    /// Register the module in the engine.
    ///
    /// Note that this is a struct without named fields: a tuple struct.
    RegisterModule(FibonacciModule),

    /// Finalize module initialization and initiate the calculations.
    ///
    /// `Init` messages should be sent only by the user of the executor system
    /// (in your solution: the `fib()` function).
    Init { id: Ident, other: Ident },

    /// Initiate the next step of the calculations.
    ///
    /// `idx` is the current index in the sequence.
    /// `num` is the current number of the sequence.
    Message { id: Ident, idx: usize, num: Num },

    /// Indicate the end of calculations.
    Done,
}

/// Run the executor.
pub(crate) fn run_executor(rx: Receiver<FibonacciSystemMessage>) -> JoinHandle<()> {
    thread::spawn(move || {
        let mut modules = std::collections::HashMap::new();

        while let Ok(msg) = rx.recv() {
            match msg {
                FibonacciSystemMessage::RegisterModule(module) => {
                    if modules.contains_key(&module.id) {
                        continue;
                    }

                    modules.insert(module.id, module);
                }
                FibonacciSystemMessage::Init { id, other } => {
                    if let Some(module) = modules.get_mut(&id) {
                        module.init(other);
                    }
                }
                FibonacciSystemMessage::Message { id, idx, num } => {
                    if let Some(module) = modules.get_mut(&id) {
                        module.message(idx, num);
                    }
                }
                FibonacciSystemMessage::Done => { 
                    break;
                }
            }
        }
    })
}

/// Calculate the `n`-th Fibonacci number.
pub(crate) fn fib(n: usize) {
    // Create the queue and two modules:
    let (tx, rx): (
        Sender<FibonacciSystemMessage>,
        Receiver<FibonacciSystemMessage>,
    ) = unbounded();
    let fib1_id = FibonacciModule::create(0, n, tx.clone());
    let fib2_id = FibonacciModule::create(1, n, tx.clone());

    // Tests will be rerun in case the assertion fails:
    assert_ne!(fib1_id, fib2_id);

    // Initialize the modules by sending `Init` messages:
    tx.send(FibonacciSystemMessage::Init {
        id: fib1_id,
        other: fib2_id,
    }).unwrap();

    tx.send(FibonacciSystemMessage::Init {
        id: fib2_id,
        other: fib1_id,
    }).unwrap();

    // Run the executor:
    run_executor(rx).join().unwrap();
}
