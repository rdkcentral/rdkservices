/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2022 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
use std::ffi::CStr;
use std::ffi::CString;
use std::os::raw::{c_char, c_void};

type SendToFunction = unsafe extern "C" fn (u32, *const c_char, plugin_ctx: *const c_void);

pub trait PluginProtocol {
  fn send_to(&mut self,  channel_id: u32, json: String);
}

pub trait Plugin {
  fn on_message(&mut self, json: String, ctx: RequestContext);
  fn on_client_connect(&mut self, channel_id: u32);
  fn on_client_disconnect(&mut self, channel_id: u32);
}

pub struct RequestContext {
  pub channel_id: u32,
  pub auth_token: String
}

pub struct ServiceMetadata {
  pub name: &'static str,
  pub version: (u32, u32, u32),
  pub create: fn (Box<dyn PluginProtocol>) -> Box<dyn Plugin>
}

#[macro_export]
macro_rules! export_plugin {
  ($name:expr, $version:expr,  $create:expr) => {
    #[no_mangle]
    pub static thunder_service_metadata : $crate::ServiceMetadata =
      $crate::ServiceMetadata {
        name: $name,
        version: $version,
        create: $create
      };
  };
}

//===============================================================================
// Internal code only below here
//===============================================================================

#[repr(C)]
pub struct CRequestContext {
  channel_id: u32,
  auth_token: *const c_char
}

fn cstr_to_string(s : *const c_char) -> String {
  if s.is_null() {
    String::new()
  }
  else {
    let c_str: &CStr = unsafe{ CStr::from_ptr(s) };
    let slice: &str = c_str.to_str().unwrap();
    let t: String = slice.to_owned();
    t
  }
}

pub struct CPlugin {
  pub name: String,
  pub plugin: Box<dyn Plugin>
}

struct DefaultPluginProtocol {
  send_func: SendToFunction,
  send_ctx: *const c_void
}

impl PluginProtocol for DefaultPluginProtocol {
  fn send_to(&mut self, channel_id: u32, json: String) {
    let c_str = CString::new(json).unwrap();
    unsafe {
      (self.send_func)(channel_id, c_str.as_ptr(), self.send_ctx);
    }
  }
}

impl CPlugin {
  fn on_incoming_message(&mut self, json_req: *const c_char, ctx: CRequestContext) {
    let req = cstr_to_string(json_req);
    let req_ctx = RequestContext {
      channel_id: ctx.channel_id,
      auth_token: cstr_to_string(ctx.auth_token)
    };
    self.plugin.on_message(req, req_ctx);
  }
  fn on_client_connect(&mut self, channel_id: u32) {
    self.plugin.on_client_connect(channel_id);
  }
  fn on_client_disconnect(&mut self, channel_id: u32) {
    self.plugin.on_client_disconnect(channel_id);
  }
}

#[no_mangle]
pub extern fn wpe_rust_plugin_create(_name: *const c_char, send_func: SendToFunction,
  plugin_ctx: *const c_void, meta_data: *mut ServiceMetadata) -> *mut CPlugin
{
  assert!(!meta_data.is_null());

  let service_metadata = unsafe{ &*meta_data };
  let proto: Box<dyn PluginProtocol> = Box::new(DefaultPluginProtocol {
    send_func: send_func,
    send_ctx: plugin_ctx});

  let plugin: Box<dyn Plugin> = (service_metadata.create)(proto);
  let name: String = service_metadata.name.to_string();
  let c_plugin: Box<CPlugin> = Box::new(CPlugin {
    name: name,
    plugin: plugin 
  });
  
  /* Sorry for the lack of RUST knowledge, this was the only way we */
  /* could send back a list with the methods supported by this Rust */
  /* plugin. Prefer to send it so Thudner can already filter on non */
  /* existing methods. Suggest to return this string as a parameter */
  /* on this method call.. */ 
  let methods = CString::new("[ \"add\", \"mul\" ]").unwrap();
  unsafe {
	// let c_buf = methods.unwrap();
    //let c_str: &CStr = unsafe { CStr::from_ptr(c_buf) };
    //let str_slice: &str = c_str.to_str().unwrap();
    send_func(0, methods.as_ptr(), plugin_ctx);
  }

  Box::into_raw(c_plugin)
}

#[no_mangle]
pub extern fn wpe_rust_plugin_destroy(ptr: *mut CPlugin) {
  assert!(!ptr.is_null());

  unsafe {
    drop(Box::from_raw(ptr));
  }
}

#[no_mangle]
pub extern fn wpe_rust_plugin_init(_ptr: *mut CPlugin, _json: *const c_char) {
  // assert!(!ptr.is_null());

  // XXX: Create + Init doesn't seem to fit the Rust style. wpe_rust_plugin_create 
  // is probably enough. Consider getting rid of this function

  // let plugin = unsafe{ &mut *ptr };
  // println!("{}.init", plugin.name);
}

#[no_mangle]
pub extern fn wpe_rust_plugin_invoke(ptr: *mut CPlugin, json_req: *const c_char, req_ctx: CRequestContext) {
  assert!(!ptr.is_null());
  assert!(!json_req.is_null());

  let plugin = unsafe{ &mut *ptr };
  let uncaught_error = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
    plugin.on_incoming_message(json_req, req_ctx);
  }));

  match uncaught_error {
    Err(cause) => {
      println!("Error calling on_incoming_message");
      println!("{:?}", cause);
    }
    Ok(_) => { }
  }
}

#[no_mangle]
pub extern fn wpe_rust_plugin_on_client_connect(ptr: *mut CPlugin, channel_id: u32) {
  assert!(!ptr.is_null());

  let plugin = unsafe{ &mut *ptr };
  let uncaught_error = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
    plugin.on_client_connect(channel_id);
  }));

  match uncaught_error {
    Err(cause) => {
      println!("Error calling on_client_connect");
      println!("{:?}", cause);
    }
    Ok(_) => { }
  }
}

#[no_mangle]
pub extern fn wpe_rust_plugin_on_client_disconnect(ptr: *mut CPlugin, channel_id: u32) {
  assert!(!ptr.is_null());

  let plugin = unsafe{ &mut *ptr };
  let uncaught_error = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
    plugin.on_client_disconnect(channel_id);
  }));

  match uncaught_error {
    Err(cause) => {
      println!("Error calling on_client_disconnect");
      println!("{:?}", cause);
    }
    Ok(_) => { }
  }
}
