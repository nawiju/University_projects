use std::sync::{Arc, Condvar, Mutex};
use std::thread::JoinHandle;
use std::thread;

type Task = Box<dyn FnOnce() + Send>;

// You can define new types (e.g., structs) if you need.
// However, they shall not be public (i.e., do not use the `pub` keyword).

/// The thread pool.
pub struct Threadpool {
    // Add here any fields you need.
    // We suggest storing handles of the worker threads, submitted tasks,
    // and information whether the pool is running or is shutting down.

    is_running: Arc<Mutex<bool>>,
    workers: Vec<JoinHandle<()>>,
    tasks: Arc<(Mutex<Vec<Task>>, Condvar)>,
}

impl Threadpool {
    /// Create new thread pool with `workers_count` workers.
    pub fn new(workers_count: usize) -> Self {
        let tasks: Arc<(Mutex<Vec<Task>>, Condvar)> = Arc::new((Mutex::new(Vec::new()), Condvar::new()));
        let is_running = Arc::new(Mutex::new(true));
        let mut workers: Vec<JoinHandle<()>> = Vec::with_capacity(workers_count);

        for _ in 0..workers_count {
            let tasks = Arc::clone(&tasks);
            let is_running = Arc::clone(&is_running);

            let worker = thread::spawn(move || {
                Threadpool::worker_loop(tasks, is_running);  
            });

            workers.push(worker);
        }

        Threadpool {
            is_running,
            workers,
            tasks,
        }
    }

    /// Submit a new task.
    pub fn submit(&self, task: Task) {
        let (lock, cvar) = &*self.tasks;
        let is_running = self.is_running.lock().unwrap();
        if !*is_running {
            return;
        }

        let mut tasks = lock.lock().unwrap();
        tasks.push(task);
        cvar.notify_one();
    }

    // We suggest extracting the implementation of the worker to an associated
    // function, like this one (however, it is not a part of the public
    // interface, so you can delete it if you implement it differently):
    fn worker_loop(tasks: Arc<(Mutex<Vec<Task>>, Condvar)>, is_running: Arc<Mutex<bool>>) {
        loop {
            let task;
            {
                let (lock, cvar) = &*tasks;
                let mut tasks = lock.lock().unwrap();

                while tasks.is_empty() && *is_running.lock().unwrap() {
                    tasks = cvar.wait(tasks).unwrap();
                }

                if tasks.is_empty() && !*is_running.lock().unwrap() {
                    break;
                }

                task = tasks.pop();
            }

            if let Some(task) = task {
                task();
            }
        }
    }
}

impl Drop for Threadpool {
    /// Gracefully end the thread pool.
    ///
    /// It waits until all submitted tasks are executed,
    /// and until all threads are joined.
    fn drop(&mut self) {
        {
            let mut is_running = self.is_running.lock().unwrap();
            *is_running = false;

            let (_, cvar) = &*self.tasks;
            cvar.notify_all();
        }

        for worker in self.workers.drain(..) {
            worker.join().unwrap();
        }
    }
}
