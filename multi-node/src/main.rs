// use std::os::unix::net::{UnixListener, UnixStream};
use std::io::Result;
use std::io;
use std::sync::{Arc, Mutex};
use std::{env, mem};
use tokio::io::{AsyncWriteExt, AsyncReadExt};
use tokio::net::{UnixListener, UnixStream};
use tokio::task;
use getopts::Options;

#[repr(C)]
#[derive(Debug)]
struct SyncMessageDone {
    is_done: bool,
}

#[repr(C)]
#[derive(Debug)]
struct SyncMessageContinue {
    budget: usize,
}


async fn slave_message_handler(mut socket: UnixStream, count: i32, budget: usize, bitmap: Arc<Mutex<i32>>) -> Result<()> {
    let mut buffer: [u8; std::mem::size_of::<SyncMessageDone>()] = [0; std::mem::size_of::<SyncMessageDone>()];

    socket.read_exact(&mut buffer).await?;
    let msgDone: SyncMessageDone = unsafe { std::ptr::read(buffer.as_ptr() as *const SyncMessageDone) };
    println!("[{}]:Message received: {:?}", count, msgDone);
    let mut done_threads= bitmap.lock().unwrap();
    *done_threads += 1;

    Ok(())
}

async fn single_slave_listener(socket_path: &String, slave_idx: i32, budget: usize, bitmap: Arc<Mutex<i32>>) -> Result<()> {
    let mut counter = 0;

    let receiver_path = format!("{}_receiver_{:0>2}", socket_path, slave_idx);
    let listener = UnixListener::bind(receiver_path)?;
    println!("[{}]:Sync server setup", slave_idx);
    
    loop {
        let (socket, _) = listener.accept().await?;
        counter += 1;
        let bitmap_clone = Arc::clone(&bitmap);
        tokio::spawn(async move {
            slave_message_handler(socket, counter, budget, bitmap_clone);
        });
    }
}

async fn master_sync_server(socket_path: &String, slaves: i32, budget: usize, bitmap: Arc<Mutex<i32>>) -> Result<()> {
    let mut counter = 0;
    let mut streams: Vec<UnixStream> = vec![];
    
    for slave_idx in 0..slaves {
        let sender_path = format!("{}_sender_{:0>2}", socket_path, slave_idx);
        let stream= UnixStream::connect(sender_path).await?;
        println!("[m]:Connected to slave {}", slave_idx);
        streams.push(stream);
    }
    
    let msg = unsafe {
        let syncMsg: SyncMessageContinue = SyncMessageContinue {
            budget : budget,
        };
        std::slice::from_raw_parts( &syncMsg as *const SyncMessageContinue as *const u8, mem::size_of::<SyncMessageContinue>())
    };
    
    loop {
        {
            let mut done_slaves = bitmap.lock().unwrap();
            if *done_slaves == slaves {
                for mut stream in streams {
                    stream.write(msg);
                    println!("[m]: Iteration {} completed.", counter);
                    counter += 1;
                }
                *done_slaves = 0;
            }
        }
        task::yield_now().await;
    }
    
}

async fn master_sync_server_spawn(socket_path: &String, slaves: i32, budget: usize) -> Result<()> {
    let bitmap_src: Arc<Mutex<i32>> = Arc::new(Mutex::new(0));
    
    for slave_idx in 0..slaves {
        let path = format!("{}", socket_path);
        let bitmap = Arc::clone(&bitmap_src);
        tokio::spawn(async move {
            single_slave_listener(&path, slave_idx, budget, bitmap);
        });
    }
    
    let path = format!("{}", socket_path);
    let bitmap = Arc::clone(&bitmap_src);
    master_sync_server(&path, slaves, budget, bitmap);
    
    Ok(())
}

#[tokio::main]
async fn main() -> Result<()> {
    let args: Vec<String> = env::args().collect();
    let program = args[0].clone();
    
    let mut opts = Options::new();
    opts.optflag("h", "help", "Print this help menu");
    opts.optopt("f", "socket", "Socket file", "FILE");
    opts.optflag("m", "master", "Is master");
    opts.optflag("s", "slave", "Is slave");
    opts.optopt("id", "slave_id", "Index of current slave", "ID");
    opts.optopt("n", "num_slaves", "Number of slaves", "NUMBER");
    opts.optopt("b", "budget", "Budget in ms for synchronization", "NUMBER");
    
    let arguments = match opts.parse(&args[1..]) {
        Ok(m) => m,
        Err(e) => {
            eprintln!("Error: {}", e);
            std::process::exit(1);
        }
    };
    
    if arguments.opt_present("h") {
        let brief = format!("Usage: {} [options]", program);
        print!("{}", opts.usage(&brief));
        return Ok(());
    }
    
    let is_master = arguments.opt_present("m");
    let is_slave = arguments.opt_present("s");
    if is_master && is_slave {
        println!("Error: can't not be 'receiver' and 'slave' at the same time, please only pass either `-r` or `-s`.");
        std::process::exit(1);
    } else if !(is_master || is_slave) {
        println!("Error: Need to at least be 'receiver' or 'slave', please pass either `-r` or `-s`.");
        std::process::exit(1);
    }
    
    let socket_path = arguments.opt_str("f");
    if socket_path.is_none() {
        println!("Error: please pass `-f` or `--socket` as path for the socket.");
        std::process::exit(1);
    }
    
    // Use the command-line arguments in your program logic
    println!("Slocket file using: {:?}", socket_path);
    if is_master {
        let total_slaves = arguments.opt_str("n").unwrap().parse().unwrap();
        let budget: usize = arguments.opt_str("b").unwrap().parse().unwrap();
        return master_sync_server_spawn(&socket_path.unwrap(), total_slaves, budget);
    } else {
        let slave_idx =  arguments.opt_str("id").unwrap().parse().unwrap();
        return slave_program(&socket_path.unwrap(), slave_idx);
    }
}