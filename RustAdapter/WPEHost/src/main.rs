use std::env;
use std::ptr;
use std::num::ParseIntError;
use std::{thread, time};
use std::net::{TcpStream};
use std::io::{Read, Write};
use byteorder::{ByteOrder, NetworkEndian};

pub const ID_INVOKE:      u32 = 1;
pub const ID_ATTACH:      u32 = 2;
pub const ID_EXIT:        u32 = 3;

#[derive(Debug)]
pub struct InvokeRequest {
  pub channel: u32,
  pub token: String,
  pub json: String
}

#[derive(Debug)]
pub struct AttachRequest {
  pub channel: u32,
  pub attach: bool
}

pub enum Request {
  Invoke(InvokeRequest),
  Attach(AttachRequest),
  Exit(),
  Err(String)
}

pub fn read_request(stream: &mut TcpStream) -> Request {
  let mut buf = [0; 4];

  stream.read(&mut buf).expect("read_request failed to read command_id");
  let command_id = NetworkEndian::read_u32(&buf);
  println!("RUST REMOTE: read command_id {}", command_id);

  if command_id == ID_INVOKE {

    stream.read(&mut buf).expect("read_request failed to read channel");
    let channel = NetworkEndian::read_u32(&buf);
    println!("RUST REMOTE: read channel {}", channel);
  
    stream.read(&mut buf).expect("read_request failed to read token_len");
    let token_len = NetworkEndian::read_u32(&buf);
    println!("RUST REMOTE: read token_len {}", token_len);
  
    stream.read(&mut buf).expect("read_request failed to read json_len");
    let json_len = NetworkEndian::read_u32(&buf);
    println!("RUST REMOTE: read json_len {}", json_len);
  
    let mut token = String::new();
  
    if token_len > 0 {
      let mut jbuf = vec![0u8; token_len as usize];
      stream.read_exact(&mut jbuf).expect("read_request failed to read token");
      token = String::from_utf8(jbuf).expect("read_request failed to read token");
      println!("RUST REMOTE: read token {}", token);
    }
  
    let mut json = String::new();
  
    if json_len > 0 {
      let mut jbuf = vec![0u8; json_len as usize];
      stream.read_exact(&mut jbuf).expect("read_request failed to read json");
      json = String::from_utf8(jbuf).expect("read_request failed to read json");
      println!("RUST REMOTE: read json {}", json);
    }
  
    let req = InvokeRequest {
      channel: channel,
      token: token,
      json: json
    };
  
    println!("RUST REMOTE: read invoke request: {:?}", req);

    Request::Invoke(req)

  } else if command_id == ID_ATTACH {
    
    stream.read(&mut buf).expect("read_request failed to read channel");
    let channel = NetworkEndian::read_u32(&buf);
    println!("RUST REMOTE: read channel {}", channel);

    let mut buf1 = [0; 1];
    stream.read(&mut buf1).expect("read_request failed to read attach");
    let attach = buf1[0] != 0;
    println!("RUST REMOTE: read attach {}", attach);

    let req = AttachRequest {
      channel: channel,
      attach: attach
    };
  
    println!("RUST REMOTE: read attach request: {:?}", req);

    Request::Attach(req)

  } else if command_id == ID_EXIT {
  
    Request::Exit()
  
  } else {

    Request::Err(format!("Invalid command_id {}", command_id))
  
  }
}

pub fn send_response(stream: &mut TcpStream, channel: u32, json: String) {
  let mut buf = [0; 4];

  println!("RUST REMOTE: sending response: channel={} json={}", channel, json);

  println!("RUST REMOTE: send channel {}", channel);
  NetworkEndian::write_u32(&mut buf, channel);
  stream.write(&buf).expect("send_response failed to write channel");

  println!("RUST REMOTE: send json_len {}", json.len());
  NetworkEndian::write_u32(&mut buf, json.len() as u32);
  stream.write(&buf).expect("send_response failed to write json_len");

  if json.len() > 0 {
    println!("RUST REMOTE: send json {}", json);
    stream.write(json.as_bytes()).expect("send_response failed to write json");
  }
}

/*
 */
struct RemotePluginProtocol  {
  stream: TcpStream
}

impl thunder_rs::PluginProtocol for RemotePluginProtocol{
  
  fn send_to(&mut self, channel: u32, json: String) {
    send_response(&mut self.stream, channel, json);
  }

}

fn load_library(shared_lib_name: &str) -> Box<libloading::Library> {
  println!("RUST REMOTE: load_library {}", shared_lib_name);
  unsafe {
    Box::new(libloading::Library::new(shared_lib_name).unwrap())
  }
}

fn load_plugin(lib: &Box<libloading::Library>, stream: TcpStream) -> Box<dyn thunder_rs::Plugin> {
  unsafe {
    let sym : libloading::Symbol< *mut thunder_rs::ServiceMetadata > = lib.get(b"thunder_service_metadata\0").unwrap();
    let service_metadata = ptr::NonNull::new(*sym as *mut thunder_rs::ServiceMetadata).unwrap().as_mut();
    let proto: Box<dyn thunder_rs::PluginProtocol> = Box::new(RemotePluginProtocol{stream: stream});
    println!("RUST REMOTE: load_plugin = {}", service_metadata.name);
    (service_metadata.create)(proto)
  }
}

fn connect_stream(addr: String) -> TcpStream {
  
  let mut retries: u32 = 20;

  let stream = loop {

    println!("RUST REMOTE: rust remote trying connect {}", addr);
    
    match TcpStream::connect(&addr) {
      Ok(stream) => {
        println!("RUST REMOTE: rust remote connected to {}", addr);
        break stream
      },
      Err(error) => {
        println!("RUST REMOTE: rust remote failed to connec to {}, error:{:?}", addr, error);
        retries = retries - 1;
        if retries == 0 {
          panic!("rust remote failed to connect tcp stream");
        }
        thread::sleep(time::Duration::from_millis(100));
        continue;
      }
    }
  };

  stream
}


fn main() -> Result<(), ParseIntError> {

  println!("RUST REMOTE: rust remote adapter process start");

  let args : Vec<String> = env::args().collect();
  println!("RUST REMOTE: {:?}", args);

  if args.len() != 4 {
    panic!("RUST REMOTE: Invalid command line.  Expected 4 arguments.  Got {}", args.len());
  }

  let lib = load_library(&args[1]);

  let addr = format!("{}:{}", args[2], args[3]);
  let mut stream = connect_stream(addr);

  let mut plugin = load_plugin(&lib, stream.try_clone().expect("Cannot clone stream"));

  let mut running = true;
  
  while running {

    match read_request(&mut stream) {
      Request::Invoke(req) => {
        println!("RUST REMOTE: invoking");
        plugin.on_message(req.json, 
          thunder_rs::RequestContext {
            channel_id: req.channel,
            auth_token: req.token
          }
        );
      },
      Request::Attach(req) => {
        println!("RUST REMOTE: attaching");
        if req.attach {
          plugin.on_client_connect(req.channel);
        } else {
          plugin.on_client_disconnect(req.channel);
        }
      },
      Request::Exit() => {
        println!("RUST REMOTE: exiting");
        running = false;
      },
      Request::Err(e) => {
        println!("RUST REMOTE: Failed to read request: {}", e);
      }
    }
  }

  drop(stream);

  println!("RUST REMOTE: rust remote adapter process end");
  Ok(())
}

