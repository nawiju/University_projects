use std::time::Duration;
use std::sync::Arc;
use std::collections::HashMap;
use tokio::sync::{Mutex, Notify};
use tokio::time;

pub trait Message: Send + 'static {}
impl<T: Send + 'static> Message for T {}

pub trait Module: Send + 'static {}
impl<T: Send + 'static> Module for T {}

/// A trait for modules capable of handling messages of type `M`.
#[async_trait::async_trait]
pub trait Handler<M: Message>: Module {
    /// Handles the message. A module must be able to access a `ModuleRef` to itself through `self_ref`.
    async fn handle(&mut self, self_ref: &ModuleRef<Self>, msg: M);
}

/// A handle returned by `ModuleRef::request_tick()`, can be used to stop sending further ticks.
// You can add fields to this struct
pub struct TimerHandle {
    is_active: Arc<Mutex<bool>>,
}

impl TimerHandle {
    /// Stops the sending of ticks resulting from the corresponding call to `ModuleRef::request_tick()`.
    /// If the ticks are already stopped, does nothing.
    pub async fn stop(&self) {
        let mut is_active = self.is_active.lock().await;
        *is_active = false;
    }
}

// You can add fields to this struct.
pub struct System {
    // Hashmap of modules that have been registered
    modules: Arc<Mutex<HashMap<i128, Box<dyn Module>>>>,
    // The id, aka the key in the modules hashmap, of the next module to be registered.
    id: i128,
    // Vector for all the TimerHandles in the system
    timer_handlers: Arc<Mutex<Vec<Arc<Mutex<bool>>>>>,
    // Vector holding the information whether the module's messages should continue to be sent
    send_message_handlers: Arc<Mutex<Vec<Arc<Mutex<bool>>>>>,
    // The Notify that all the tasks are waiting on for sending messages
    send_message_notifies: Arc<Mutex<Vec<Arc<Notify>>>>,
    task_handles: Vec<Arc<Mutex<Vec<tokio::task::JoinHandle<()>>>>>,
}

impl System {
    /// Registers the module in the system.
    /// Returns a `ModuleRef`, which can be used then to send messages to the module.
    pub async fn register_module<T: Module>(&mut self, module: T) -> ModuleRef<T> {
        let id = self.id;
        self.id += 1;
        
        // Insert the new module into the modules hashmap.
        let mut modules = self.modules.lock().await;
        let module = Arc::new(Mutex::new(module));
        modules.insert(id, Box::new(module.clone()));

        // If the boolean value protected by the mutex is true then the message should be sent.
        let send_message_active = Arc::new(Mutex::new(true));
        self.send_message_handlers.lock().await.push(send_message_active.clone());

        // Create new notify
        let send_message_notify = Arc::new(Notify::new());
        send_message_notify.notify_one();
        self.send_message_notifies.lock().await.push(send_message_notify.clone());

        // Add a task handle vector protected by a mutex so that tasks for this module do not have to 
        // compete for a global mutex with other modules' tasks
        let task_handles: Arc<Mutex<Vec<tokio::task::JoinHandle<()>>>> = Arc::new(Mutex::new(Vec::new()));
        self.task_handles.push(task_handles.clone());

        ModuleRef {
            module,
            timer_handlers: self.timer_handlers.clone(),
            send_message_notify,
            send_message_active: send_message_active.clone(),
            task_handles,
        }
    }

    /// Creates and starts a new instance of the system.
    pub async fn new() -> Self {
        System {
            modules: Arc::new(Mutex::new(HashMap::new())),
            id: 0,
            timer_handlers: Arc::new(Mutex::new(Vec::new())),
            send_message_handlers: Arc::new(Mutex::new(Vec::new())),
            send_message_notifies: Arc::new(Mutex::new(Vec::new())),
            task_handles: Vec::new(),
        }
    }

    /// Gracefully shuts the system down.
    pub async fn shutdown(&mut self) {
        let mut timer_handlers = self.timer_handlers.lock().await;
        let mut send_message_handlers = self.send_message_handlers.lock().await;

        // Shut down all timers
        for timer_handler in timer_handlers.iter_mut() {
            let mut timer_handler = timer_handler.lock().await;
            *timer_handler = false;
        }

        // Cancel any messages that are to be sent
        for send_message_handler in send_message_handlers.iter_mut() {
            let mut send_message_handler = send_message_handler.lock().await;
            *send_message_handler = false;
        }
        
        // Wake up all tasks that are waiting on a notify. Due to the above code, they will immediately 
        // exit the function as they will see that they are not to send messages.
        let send_message_notifies = self.send_message_notifies.lock().await;
        for send_message_notify in send_message_notifies.iter() {
            send_message_notify.notify_waiters();
        }

        // Wait for all tasks to finish
        for task_handles in self.task_handles.iter() {
            for task_handle in task_handles.lock().await.iter_mut() {
                task_handle.await.unwrap();
            }
        }
        
    }
}

/// A reference to a module used for sending messages.
// You can add fields to this struct.
pub struct ModuleRef<T: Module + ?Sized> {
    // A marker field required to inform the compiler about variance in T.
    // It can be removed if type T is used in some other field.
    module: Arc<Mutex<T>>,
    timer_handlers: Arc<Mutex<Vec<Arc<Mutex<bool>>>>>,
    send_message_notify: Arc<Notify>,
    send_message_active: Arc<Mutex<bool>>,
    task_handles: Arc<Mutex<Vec<tokio::task::JoinHandle<()>>>>,
}

impl<T: Module> ModuleRef<T> {
    /// Sends the message to the module.
    pub async fn send<M: Message>(&self, msg: M)
    where
        T: Handler<M>,
    {
        let self_copy = self.clone();

        let handle = tokio::task::spawn(async move {
            self_copy.send_message_notify.notified().await;
            
            // Check if the system is shutting down
            {
                let send_message_active = self_copy.send_message_active.lock().await;
                if !*send_message_active {
                    return;
                }
            }

            let mut module = self_copy.module.lock().await;
            
            // Do something with the module. This is to be done while holding the mutex so 
            // that no other task can take it
            module.handle(&self_copy, msg).await;

            // notify_one() wakes up the oldest task according to tokio docs
            self_copy.send_message_notify.notify_one();    
        }); 

        self.task_handles.lock().await.push(handle);
    }

    /// Schedules a message to be sent to the module periodically with the given interval.
    /// The first tick is sent after the interval elapses.
    /// Every call to this function results in sending new ticks and does not cancel
    /// ticks resulting from previous calls.
    pub async fn request_tick<M>(&self, message: M, delay: Duration) -> TimerHandle
    where
        M: Message + Clone,
        T: Handler<M>,
    {
        let timer = TimerHandle { is_active: Arc::new(Mutex::new(true)) };
        let is_active = timer.is_active.clone();
        let self_copy = self.clone();

        // Use interval instead of sleep as it measures the time elapsed between ticks
        let mut _interval = time::interval(delay);

        _interval.tick().await;

        tokio::task::spawn(async move {
            loop {
                _interval.tick().await;                

                {
                    let is_active = is_active.lock().await;
                    if !*is_active {
                        break;
                    }
                }

                self_copy.send(message.clone()).await;
            }
        });

        self.timer_handlers.lock().await.push(timer.is_active.clone()); 
        timer
    }
}

impl<T: Module> Clone for ModuleRef<T> {
    /// Creates a new reference to the same module.
    fn clone(&self) -> Self {
        ModuleRef {
            module: self.module.clone(),
            timer_handlers: self.timer_handlers.clone(),
            send_message_notify: self.send_message_notify.clone(),
            send_message_active: self.send_message_active.clone(),
            task_handles: self.task_handles.clone(),
        }
    }
}

