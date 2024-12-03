# Lab 1 
Implements a Fibonacci struct that executes calculations of the numbers in the Fibonacci sequence. Provides an iterator which iterates over the Fibonacci sequence.

# Lab 2
Implements a thread pool which owns a few threads, called workers, and allows for submitting tasks that the workers execute. Every worker operates in a loop: it waits for a task, executes it, waits for the next task, and so on. The thread pool manages the workers and distributes the tasks to the workers. The thread pool will process the tasks concurrently. A `Drop` trait is implemented that stops all workers and waits for all threads to finish. It also waits until all submitted tasks are executed.

# Lab 3
Implements a modular executor system in Rust that calculates Fibonacci numbers by exchanging messages between modules.A single executor runs in its own thread, managing the registration of modules and the delivery of messages. Modules dynamically register themselves with the executor, allowing flexibility in the system.

The calculation of Fibonacci numbers is handled by two FibonacciModules. One module, A, starts with the value 0, while the other, B, starts with 1. The process begins when Module B receives an Init message, after which the modules take turns exchanging numbers and computing the next Fibonacci number in the sequence. This back-and-forth communication continues until the desired Fibonacci index is reached.

Modules communicate using a defined message type, FibonacciSystemMessage, which includes messages like Init to start the process, RegisterModule for dynamic registration, and Done to terminate the system gracefully. Once the target Fibonacci number is calculated, the process ends with a Done message that stops the executor.

The system supports any number of modules, although only two are required for the Fibonacci calculation. This design ensures modularity, thread safety, and scalability, while adhering to an event-driven architecture.

# Lab 5
This project implements a secure communication link in Rust, providing two types: SecureClient and SecureServer. The SecureClient accepts plaintext messages, encrypts them, and sends them to the SecureServer over a provided TLS link. The SecureServer decrypts the messages, verifies their integrity using HMAC, and returns the original plaintext content.

Messages are formatted with three components before encryption. First, the length of the message content (4 bytes in network byte order) is added. Then, the actual content is included, followed by a 32-byte HMAC tag generated using the SHA-256 algorithm. This HMAC ensures the integrity and authenticity of the message content.

The SecureClient uses TLS for encryption, leveraging the Rust rustls crate. The same HMAC key, provided during initialization, is used for all messages exchanged between the client and the server. The SecureServer blocks on receiving messages until data is available. Upon receipt, it decrypts the message, validates the HMAC tag, and returns the plaintext content. If the HMAC validation fails, the server returns an error, ensuring secure communication.

# Lab 6
This project implements a crash-resilient key-value storage system in Rust, adhering to the StableStorage trait. It ensures atomic operations and data integrity by writing values to a temporary file (tmpfile) with a checksum before finalizing the write to the destination file (dstfile). Each write is followed by fsyncdata calls to ensure data is safely persisted to disk.

On recovery, the presence of tmpfile determines the next steps. If tmpfile exists and its checksum is valid, the write can be resumed. If the checksum is invalid, the incomplete file is removed. This guarantees that only fully completed writes are preserved.

The system supports keys up to 255 bytes and values up to 65535 bytes. It validates inputs and ensures that invalid operations do not compromise the storage. All keys are kept in memory, while values are stored on disk to enable efficient recovery. Operations such as put(), get(), and remove() are independent of the number of stored keys, making the system scalable and efficient.

This implementation guarantees reliability across crashes, ensures atomicity for writes and deletes, and maintains data integrity using checksums.
