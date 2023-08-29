// use std::os::unix::net::{UnixListener, UnixStream};
use std::io::{Result, ErrorKind};
use std::sync::{Arc, Mutex};
use std::{env, mem, fs};
use tokio::io::{AsyncWriteExt, AsyncReadExt};
use tokio::net::{UnixListener, UnixStream};
use tokio::task;
use tokio::time::{sleep, Duration};
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

async fn single_slave_listener(socket_path: &String, slave_idx: i32, bitmap: Arc<Mutex<i32>>) -> Result<()> {
    // let mut counter = 0;
    
    let m2s_path = format!("{}/m2s_{:0>2}", socket_path, slave_idx);

    let _ = fs::remove_file(&m2s_path);
    let listener = UnixListener::bind(m2s_path)?;
    //println!("M:[{}]:Sync server setup", slave_idx);
    
    loop {
        let res = listener.accept().await;
        //println!("M:[{}][{}]:Listener socket accepted", slave_idx, counter);
        if res.is_err() {
            //println!("M:[{}]:Listener socket err", slave_idx);
        }

        let (mut socket, _) = res?;
        let mut buffer: [u8; std::mem::size_of::<SyncMessageDone>()] = [0; std::mem::size_of::<SyncMessageDone>()];
        socket.read_exact(&mut buffer).await?;

        // counter += 1;
        // let msg_done: SyncMessageDone = unsafe { std::ptr::read(buffer.as_ptr() as *const SyncMessageDone) };
        //println!("M:[{}][{}]:Message received: {:?}", slave_idx, counter, msg_done);
        (*bitmap.lock().unwrap())+=1;
    }
}

async fn master_sync_server(socket_path: &String, slaves: i32, budget: usize, bitmap: Arc<Mutex<i32>>) -> Result<()> {
    // let mut counter = 0;
    let mut streams: Vec<UnixStream> = vec![];
    for slave_idx in 0..slaves {
        let s2m_path = format!("{}/s2m_{:0>2}", socket_path, slave_idx);
        let _ = fs::remove_file(&s2m_path);
    }
    
    for slave_idx in 0..slaves {
        let s2m_path = format!("{}/s2m_{:0>2}", socket_path, slave_idx);
        let mut res = UnixStream::connect(&s2m_path).await;
        while let Err(e) = res {
            match e.kind() {
                ErrorKind::NotFound => {
                    //println!("M:Socket not found, retrying in 2 seconds");
                    sleep(Duration::from_secs(2)).await;
                    res = UnixStream::connect(&s2m_path).await;
                }
                _ => {
                    //println!("M:Error with socket slave {}: {}", slave_idx, e);
                    std::process::exit(1);
                }
            }
        }
        
        streams.push(res.unwrap());
    }
    
    let msg = unsafe {
        let msg_sync: SyncMessageContinue = SyncMessageContinue {
            budget : budget,
        };
        std::slice::from_raw_parts( &msg_sync as *const SyncMessageContinue as *const u8, mem::size_of::<SyncMessageContinue>())
    };
    
    loop {
        let done_slaves = *bitmap.lock().unwrap();
        //println!("M:[m]:Checking for slaves in Iteration {}", counter);
        if done_slaves == slaves {
            (*bitmap.lock().unwrap()) = 0;
            for slave_idx in 0..slaves {
                let s2m_path = format!("{}/s2m_{:0>2}", socket_path, slave_idx);
                streams[slave_idx as usize].write(msg).await?;
                //println!("M:[{}]:Iteration {} completed.", slave_idx, counter);
                //counter += 1;
                streams[slave_idx as usize] = UnixStream::connect(&s2m_path).await?;
                //println!("M:[m]:Reconnected");
            }
            //println!("M:[m]:Done with Iteration {}", counter);
        }
        task::yield_now().await;
    }
    
}

async fn master_sync_server_spawn(socket_path: &String, slaves: i32, budget: usize) -> Result<()> {
    let bitmap_src: Arc<Mutex<i32>> = Arc::new(Mutex::new(0));
    
    for slave_idx in 0..slaves {
        let path = format!("{}", socket_path);
        let bitmap = bitmap_src.clone();
        tokio::spawn(async move {
            let ret = single_slave_listener(&path, slave_idx, bitmap).await;
            if ret.is_err() {
                //println!("M[{}]:Listener crashed: {}", slave_idx, ret.err().unwrap());
            }
        });
    }
    
    let path = format!("{}", socket_path);
    let bitmap = bitmap_src.clone();
    master_sync_server(&path, slaves, budget, bitmap).await?;
    
    Ok(())
}


async fn slave_program(socket_path: &String, slave_idx: i32) -> Result<()> {
    let s2m_path = format!("{}/s2m_{:0>2}", socket_path, slave_idx);
    let m2s_path = format!("{}/m2s_{:0>2}", socket_path, slave_idx);

    let listener= UnixListener::bind(&s2m_path)?;
 
    let mut socket_slave_m2s_res= UnixStream::connect(&m2s_path).await;
    while let Err(e) = socket_slave_m2s_res {
        match e.kind() {
            ErrorKind::NotFound => {
                //println!("S:[{}]:Socket not found, retrying in 2 seconds", slave_idx);
                sleep(Duration::from_secs(2)).await;
                socket_slave_m2s_res = UnixStream::connect(&m2s_path).await;
            }
            _ => {
                //println!("S:Error with socket slave {}: {}", slave_idx, e);
                std::process::exit(1);
            }
        }
    }
    //println!("S:[{}]:Connected to master", slave_idx);
    
    let msg_done = unsafe {
        let sync_msg: SyncMessageDone = SyncMessageDone {
            is_done : true,
        };
        std::slice::from_raw_parts( &sync_msg as *const SyncMessageDone as *const u8, mem::size_of::<SyncMessageDone>())
    };
   
    // let mut count = 0;
    let mut socket_slave_m2s = socket_slave_m2s_res.unwrap();
    loop {
        socket_slave_m2s.write(msg_done).await?;
        let (mut socket_budget, _) = listener.accept().await?;
        
        let mut buffer: [u8; std::mem::size_of::<SyncMessageContinue>()] = [0; std::mem::size_of::<SyncMessageContinue>()];
        
        socket_budget.read_exact(&mut buffer).await?;
        let msg_continue: SyncMessageContinue = unsafe { std::ptr::read(buffer.as_ptr() as *const SyncMessageContinue) };
        //println!("S:[{}]:[{}]:Message received: {:?}", slave_idx, count, msg_continue);
        
        sleep(Duration::from_millis(msg_continue.budget as u64)).await;
        
        // count += 1;
        socket_slave_m2s = UnixStream::connect(&m2s_path).await?;
    }
    
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
    opts.optopt("i", "slave_id", "Index of current slave", "ID");
    opts.optopt("n", "num_slaves", "Number of slaves", "NUMBER");
    opts.optopt("b", "budget", "Budget in ms for synchronization", "NUMBER");
    
    let arguments = match opts.parse(&args[1..]) {
        Ok(m) => m,
        Err(_e) => {
            //println!("Error: {}", e);
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
        //println!("Error: can't not be 'master' and 'slave' at the same time, please only pass either `-r` or `-s`.");
        std::process::exit(1);
    } else if !(is_master || is_slave) {
        //println!("Error: Need to at least be 'master' or 'slave', please pass either `-r` or `-s`.");
        std::process::exit(1);
    }
    
    let socket_path = arguments.opt_str("f");
    if socket_path.is_none() {
        //println!("Error: please pass `-f` or `--socket` as path for the socket.");
        std::process::exit(1);
    }
    
    // Use the command-line arguments in your program logic
    //println!("Socket file using: {:?}", socket_path);
    if is_master {
        let total_slaves = arguments.opt_str("n").unwrap().parse().unwrap();
        let budget: usize = arguments.opt_str("b").unwrap().parse().unwrap();
        master_sync_server_spawn(&socket_path.unwrap(), total_slaves, budget).await?;
    } else {
        let slave_idx =  arguments.opt_str("i").unwrap().parse().unwrap();
        slave_program(&socket_path.unwrap(), slave_idx).await?;
    }
    
    Ok(())
}