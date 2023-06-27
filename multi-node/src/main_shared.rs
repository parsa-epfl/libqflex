use nix::sys::mman::{mmap, MapFlags, ProtFlags};
use nix::unistd::{fork, ForkResult};
use std::mem;

pub struct SyncMessage {
    pub is_done: bool,
    pub process: usize,
}

pub struct SyncMessageStart {
    pub is_continue: bool,
    pub process: usize,
    pub budget: usize,
}

fn main() {
    const SHARED_MEM_SIZE: usize = mem::size_of::<SyncMessage>();

    // Create a shared memory region
    let shared_mem = mmap(
        None,
        SHARED_MEM_SIZE,
        ProtFlags::PROT_READ | ProtFlags::PROT_WRITE,
        MapFlags::MAP_SHARED | MapFlags::MAP_ANONYMOUS,
        -1,
        0,
    ) 
    .expect("Failed to create shared memory");

    // Fork a child process
    match unsafe { fork() }{
        Ok(ForkResult::Parent { child }) => {
            let shared_mem_ptr = shared_mem as *mut SyncMessage;
            let shared_data = unsafe { std::slice::from_raw_parts_mut(shared_mem_ptr, SHARED_MEM_SIZE) };


            shared_data[0].is_done;
            // Parent process
            let mut msg_count = 0;

            while (msg_count < 10) {
                if  shared_data[0].is_done {
                    shared_data[0].is_done = false;
                    println!("Process {:?} completed.", shared_data[0].process);
                    msg_count += 1;
                }
            }

            // Wait for the child process to finish
            match child.wait() {
                Ok(status) => println!("Child process exited with status: {:?}", status),
                Err(err) => println!("Failed to wait for child process: {:?}", err),
            }

        }

        Ok(ForkResult::Child) => {
            // Child process

            // Access the shared memory
            let shared_mem_ptr = shared_mem as *mut SyncMessage;
            let shared_data = unsafe { std::slice::from_raw_parts_mut(shared_mem_ptr, SHARED_MEM_SIZE) };

            // Modify the shared data
            shared_data[0] = 42;

            // Exit the child process
            std::process::exit(0);
        }
        Err(err) => println!("Failed to fork process: {:?}", err),
    }
}
