## Asynchronous Module System in Rust
Class: Distributed Systems

This project implements a library for an asynchronous module system that supports user-defined modules and message types. The system is designed for flexibility and efficiency, enabling modules to communicate and schedule tasks in an asynchronous environment.

The system allows users to register modules that implement the Module trait. Each registered module is associated with a ModuleRef, which can be used to send messages to the module. Messages of a given type can be sent to a module if the module implements the Handler<M> trait for that message type. Modules handle messages sequentially in the order they are received, ensuring reliable message processing.

The module system interface provides the following functionality:

- Creating and starting new instances of the system (System::new()).

- Registering modules in the system (System::register_module()). The Module trait specifies bounds that must be satisfied by a module. Registering a module yields a ModuleRef, which can be then used to send messages to the module.

- Sending messages to registered modules (ModuleRef::send()). A message of type M can be sent to a module of type T if T implements the Handler<M> trait. A module handles messages in the order in which it receives them.

- A message is considered as delivered after the corresponding ModuleRef::send() has finished. In other words, the system behaves as if ModuleRef::send() inserted a message at the end of the receiving module’s message queue.

- Creating new references to registered modules (<ModuleRef as Clone>::clone()).

- Scheduling a message to be sent to a registered module periodically with a given interval (ModuleRef::request_tick()). The first tick should be sent after the interval elapsed. Requesting a tick yields a TimerHandle, which can be used to stop the sending of further ticks resulting from this request (TimerHandle::stop()).

- ModuleRef::request_tick() can be called multiple times. Every call results in sending more ticks and does not cancel ticks resulting from previous calls. For example, if ModuleRef::request_tick() is called at time 0 with interval 2 and at time 1 with interval 3, ticks should arrive at times 2, 4, 4 (two ticks at time 4), 6, 7, …

- Shutting the system down gracefully (System::shutdown()). The shutdown should wait for all already started handlers to finish and for all registered modules to be dropped. It should not wait for all enqueued messages to be handled. It does not have to wait for all Tokio tasks to finish, but it must cause all of them to finish (e.g., it is acceptable if a task handling ModuleRef::request_tick() finishes an interval after the shutdown).

(The above is taken directly from the task description written by KI, FP, MB, WC, MM).
