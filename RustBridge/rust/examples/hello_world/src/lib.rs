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
struct SamplePlugin { 
  proto: Box<dyn thunder_rs::PluginProtocol>
}

impl thunder_rs::Plugin for SamplePlugin {
  fn on_message(&mut self, json: String, ctx: thunder_rs::RequestContext) {
    println!("TODO: dispatch incoming message {}", json);
    println!("\tchannel_id:{0}", ctx.channel_id);
    println!("\tauth_token:{0}", ctx.auth_token);

    let res: String = String::from("{\"jsonrpc\":\"2.0\", \"id\":4, \"result\":\"hello from rust\"}");

    // the channel_id is unique per client. if you ever want to send a message to
    // client, whether that be synchronously responding to a request, asynchronously
    // responding to a request, or emitting an event, use this API
    self.proto.send_to(ctx.channel_id, res);
  }

  // TODO: we should probably add the auth_token to this call. At the current time
  // this isn't too useful. Applications will likely ignore and just lazily track
  // connected clients when the make calls that get delivered via on_message
  fn on_client_connect(&mut self, channel_id: u32) {
    println!("client_connect:{}", channel_id);
  }

  // TODO: If you're tracking state about a client, you also would like to know
  // when that client disconnects. you get that inication here
  fn on_client_disconnect(&mut self, channel_id: u32) {
    println!("client_disconnect:{}", channel_id);
  }
}

fn sample_plugin_init(proto: Box<dyn thunder_rs::PluginProtocol>) -> Box<dyn thunder_rs::Plugin> {
  Box::new(SamplePlugin{ proto: proto})
}

thunder_rs::export_plugin!("SampleRustPlugin", (1,0,0), sample_plugin_init);
