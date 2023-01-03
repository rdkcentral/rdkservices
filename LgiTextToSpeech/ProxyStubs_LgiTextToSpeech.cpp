//
// generated automatically from "ILgiTextToSpeech.h"
//
// implements RPC proxy stubs for:
//   - class ILgiTextToSpeech
//   - class ILgiTextToSpeech::INotification
//

#include "ILgiTextToSpeech.h"
#include "Module.h"

namespace WPEFramework {

namespace ProxyStubs {

    using namespace Exchange;

    // -----------------------------------------------------------------
    // STUB
    // -----------------------------------------------------------------

    //
    // ILgiTextToSpeech interface stub definitions
    //
    // Methods:
    //  (0) virtual uint32_t Configure(PluginHost::IShell*) = 0
    //  (1) virtual void Register(ILgiTextToSpeech::INotification*) = 0
    //  (2) virtual void Unregister(ILgiTextToSpeech::INotification*) = 0
    //  (3) virtual uint32_t Enable(const string&, string&) = 0
    //  (4) virtual uint32_t ListVoices(const string&, string&) = 0
    //  (5) virtual uint32_t SetConfiguration(const string&, string&) = 0
    //  (6) virtual uint32_t GetConfiguration(const string&, string&) = 0
    //  (7) virtual uint32_t IsEnabled(const string&, string&) = 0
    //  (8) virtual uint32_t Speak(const string&, string&) = 0
    //  (9) virtual uint32_t Cancel(const string&, string&) = 0
    //  (10) virtual uint32_t Pause(const string&, string&) = 0
    //  (11) virtual uint32_t Resume(const string&, string&) = 0
    //  (12) virtual uint32_t IsSpeaking(const string&, string&) = 0
    //  (13) virtual uint32_t GetSpeechState(const string&, string&) = 0
    //

    ProxyStub::MethodHandler LgiTextToSpeechStubMethods[] = {
        // virtual uint32_t Configure(PluginHost::IShell*) = 0
        //
        [](Core::ProxyType<Core::IPCChannel>& channel, Core::ProxyType<RPC::InvokeMessage>& message) {
            RPC::Data::Input& input(message->Parameters());

            // read parameters
            RPC::Data::Frame::Reader reader(input.Reader());
            RPC::instance_id param0 = reader.Number<RPC::instance_id>();
            PluginHost::IShell* param0_proxy = nullptr;
            ProxyStub::UnknownProxy* param0_proxy_inst = nullptr;
            if (param0 != 0) {
                param0_proxy_inst = RPC::Administrator::Instance().ProxyInstance(channel, param0, false, param0_proxy);
                ASSERT((param0_proxy_inst != nullptr) && (param0_proxy != nullptr) && "Failed to get instance of PluginHost::IShell proxy");

                if ((param0_proxy_inst == nullptr) || (param0_proxy == nullptr)) {
                    TRACE_L1("Failed to get instance of PluginHost::IShell proxy");
                }
            }

            // write return value
            RPC::Data::Frame::Writer writer(message->Response().Writer());

            // call implementation
            ILgiTextToSpeech* implementation = reinterpret_cast<ILgiTextToSpeech*>(input.Implementation());
            ASSERT((implementation != nullptr) && "Null ILgiTextToSpeech implementation pointer");
            const uint32_t output = implementation->Configure(param0_proxy);
            writer.Number<const uint32_t>(output);

            if (param0_proxy_inst != nullptr) {
                RPC::Administrator::Instance().Release(param0_proxy_inst, message->Response());
            }
        },

        // virtual void Register(ILgiTextToSpeech::INotification*) = 0
        //
        [](Core::ProxyType<Core::IPCChannel>& channel, Core::ProxyType<RPC::InvokeMessage>& message) {
            RPC::Data::Input& input(message->Parameters());

            // read parameters
            RPC::Data::Frame::Reader reader(input.Reader());
            RPC::instance_id param0 = reader.Number<RPC::instance_id>();
            ILgiTextToSpeech::INotification* param0_proxy = nullptr;
            ProxyStub::UnknownProxy* param0_proxy_inst = nullptr;
            if (param0 != 0) {
                param0_proxy_inst = RPC::Administrator::Instance().ProxyInstance(channel, param0, false, param0_proxy);
                ASSERT((param0_proxy_inst != nullptr) && (param0_proxy != nullptr) && "Failed to get instance of ILgiTextToSpeech::INotification proxy");

                if ((param0_proxy_inst == nullptr) || (param0_proxy == nullptr)) {
                    TRACE_L1("Failed to get instance of ILgiTextToSpeech::INotification proxy");
                }
            }

            // call implementation
            ILgiTextToSpeech* implementation = reinterpret_cast<ILgiTextToSpeech*>(input.Implementation());
            ASSERT((implementation != nullptr) && "Null ILgiTextToSpeech implementation pointer");
            implementation->Register(param0_proxy);

            if (param0_proxy_inst != nullptr) {
                RPC::Administrator::Instance().Release(param0_proxy_inst, message->Response());
            }
        },

        // virtual void Unregister(ILgiTextToSpeech::INotification*) = 0
        //
        [](Core::ProxyType<Core::IPCChannel>& channel, Core::ProxyType<RPC::InvokeMessage>& message) {
            RPC::Data::Input& input(message->Parameters());

            // read parameters
            RPC::Data::Frame::Reader reader(input.Reader());
            RPC::instance_id param0 = reader.Number<RPC::instance_id>();
            ILgiTextToSpeech::INotification* param0_proxy = nullptr;
            ProxyStub::UnknownProxy* param0_proxy_inst = nullptr;
            if (param0 != 0) {
                param0_proxy_inst = RPC::Administrator::Instance().ProxyInstance(channel, param0, false, param0_proxy);
                ASSERT((param0_proxy_inst != nullptr) && (param0_proxy != nullptr) && "Failed to get instance of ILgiTextToSpeech::INotification proxy");

                if ((param0_proxy_inst == nullptr) || (param0_proxy == nullptr)) {
                    TRACE_L1("Failed to get instance of ILgiTextToSpeech::INotification proxy");
                }
            }

            // call implementation
            ILgiTextToSpeech* implementation = reinterpret_cast<ILgiTextToSpeech*>(input.Implementation());
            ASSERT((implementation != nullptr) && "Null ILgiTextToSpeech implementation pointer");
            implementation->Unregister(param0_proxy);

            if (param0_proxy_inst != nullptr) {
                RPC::Administrator::Instance().Release(param0_proxy_inst, message->Response());
            }
        },

        // virtual uint32_t Enable(const string&, string&) = 0
        //
        [](Core::ProxyType<Core::IPCChannel>& channel VARIABLE_IS_NOT_USED, Core::ProxyType<RPC::InvokeMessage>& message) {
            RPC::Data::Input& input(message->Parameters());

            // read parameters
            RPC::Data::Frame::Reader reader(input.Reader());
            const string param0 = reader.Text();
            string param1{}; // storage

            // call implementation
            ILgiTextToSpeech* implementation = reinterpret_cast<ILgiTextToSpeech*>(input.Implementation());
            ASSERT((implementation != nullptr) && "Null ILgiTextToSpeech implementation pointer");
            const uint32_t output = implementation->Enable(param0, param1);

            // write return values
            RPC::Data::Frame::Writer writer(message->Response().Writer());
            writer.Number<const uint32_t>(output);
            writer.Text(param1);
        },

        // virtual uint32_t ListVoices(const string&, string&) = 0
        //
        [](Core::ProxyType<Core::IPCChannel>& channel VARIABLE_IS_NOT_USED, Core::ProxyType<RPC::InvokeMessage>& message) {
            RPC::Data::Input& input(message->Parameters());

            // read parameters
            RPC::Data::Frame::Reader reader(input.Reader());
            const string param0 = reader.Text();
            string param1{}; // storage

            // call implementation
            ILgiTextToSpeech* implementation = reinterpret_cast<ILgiTextToSpeech*>(input.Implementation());
            ASSERT((implementation != nullptr) && "Null ILgiTextToSpeech implementation pointer");
            const uint32_t output = implementation->ListVoices(param0, param1);

            // write return values
            RPC::Data::Frame::Writer writer(message->Response().Writer());
            writer.Number<const uint32_t>(output);
            writer.Text(param1);
        },

        // virtual uint32_t SetConfiguration(const string&, string&) = 0
        //
        [](Core::ProxyType<Core::IPCChannel>& channel VARIABLE_IS_NOT_USED, Core::ProxyType<RPC::InvokeMessage>& message) {
            RPC::Data::Input& input(message->Parameters());

            // read parameters
            RPC::Data::Frame::Reader reader(input.Reader());
            const string param0 = reader.Text();
            string param1{}; // storage

            // call implementation
            ILgiTextToSpeech* implementation = reinterpret_cast<ILgiTextToSpeech*>(input.Implementation());
            ASSERT((implementation != nullptr) && "Null ILgiTextToSpeech implementation pointer");
            const uint32_t output = implementation->SetConfiguration(param0, param1);

            // write return values
            RPC::Data::Frame::Writer writer(message->Response().Writer());
            writer.Number<const uint32_t>(output);
            writer.Text(param1);
        },

        // virtual uint32_t GetConfiguration(const string&, string&) = 0
        //
        [](Core::ProxyType<Core::IPCChannel>& channel VARIABLE_IS_NOT_USED, Core::ProxyType<RPC::InvokeMessage>& message) {
            RPC::Data::Input& input(message->Parameters());

            // read parameters
            RPC::Data::Frame::Reader reader(input.Reader());
            const string param0 = reader.Text();
            string param1{}; // storage

            // call implementation
            ILgiTextToSpeech* implementation = reinterpret_cast<ILgiTextToSpeech*>(input.Implementation());
            ASSERT((implementation != nullptr) && "Null ILgiTextToSpeech implementation pointer");
            const uint32_t output = implementation->GetConfiguration(param0, param1);

            // write return values
            RPC::Data::Frame::Writer writer(message->Response().Writer());
            writer.Number<const uint32_t>(output);
            writer.Text(param1);
        },

        // virtual uint32_t IsEnabled(const string&, string&) = 0
        //
        [](Core::ProxyType<Core::IPCChannel>& channel VARIABLE_IS_NOT_USED, Core::ProxyType<RPC::InvokeMessage>& message) {
            RPC::Data::Input& input(message->Parameters());

            // read parameters
            RPC::Data::Frame::Reader reader(input.Reader());
            const string param0 = reader.Text();
            string param1{}; // storage

            // call implementation
            ILgiTextToSpeech* implementation = reinterpret_cast<ILgiTextToSpeech*>(input.Implementation());
            ASSERT((implementation != nullptr) && "Null ILgiTextToSpeech implementation pointer");
            const uint32_t output = implementation->IsEnabled(param0, param1);

            // write return values
            RPC::Data::Frame::Writer writer(message->Response().Writer());
            writer.Number<const uint32_t>(output);
            writer.Text(param1);
        },

        // virtual uint32_t Speak(const string&, string&) = 0
        //
        [](Core::ProxyType<Core::IPCChannel>& channel VARIABLE_IS_NOT_USED, Core::ProxyType<RPC::InvokeMessage>& message) {
            RPC::Data::Input& input(message->Parameters());

            // read parameters
            RPC::Data::Frame::Reader reader(input.Reader());
            const string param0 = reader.Text();
            string param1{}; // storage

            // call implementation
            ILgiTextToSpeech* implementation = reinterpret_cast<ILgiTextToSpeech*>(input.Implementation());
            ASSERT((implementation != nullptr) && "Null ILgiTextToSpeech implementation pointer");
            const uint32_t output = implementation->Speak(param0, param1);

            // write return values
            RPC::Data::Frame::Writer writer(message->Response().Writer());
            writer.Number<const uint32_t>(output);
            writer.Text(param1);
        },

        // virtual uint32_t Cancel(const string&, string&) = 0
        //
        [](Core::ProxyType<Core::IPCChannel>& channel VARIABLE_IS_NOT_USED, Core::ProxyType<RPC::InvokeMessage>& message) {
            RPC::Data::Input& input(message->Parameters());

            // read parameters
            RPC::Data::Frame::Reader reader(input.Reader());
            const string param0 = reader.Text();
            string param1{}; // storage

            // call implementation
            ILgiTextToSpeech* implementation = reinterpret_cast<ILgiTextToSpeech*>(input.Implementation());
            ASSERT((implementation != nullptr) && "Null ILgiTextToSpeech implementation pointer");
            const uint32_t output = implementation->Cancel(param0, param1);

            // write return values
            RPC::Data::Frame::Writer writer(message->Response().Writer());
            writer.Number<const uint32_t>(output);
            writer.Text(param1);
        },

        // virtual uint32_t Pause(const string&, string&) = 0
        //
        [](Core::ProxyType<Core::IPCChannel>& channel VARIABLE_IS_NOT_USED, Core::ProxyType<RPC::InvokeMessage>& message) {
            RPC::Data::Input& input(message->Parameters());

            // read parameters
            RPC::Data::Frame::Reader reader(input.Reader());
            const string param0 = reader.Text();
            string param1{}; // storage

            // call implementation
            ILgiTextToSpeech* implementation = reinterpret_cast<ILgiTextToSpeech*>(input.Implementation());
            ASSERT((implementation != nullptr) && "Null ILgiTextToSpeech implementation pointer");
            const uint32_t output = implementation->Pause(param0, param1);

            // write return values
            RPC::Data::Frame::Writer writer(message->Response().Writer());
            writer.Number<const uint32_t>(output);
            writer.Text(param1);
        },

        // virtual uint32_t Resume(const string&, string&) = 0
        //
        [](Core::ProxyType<Core::IPCChannel>& channel VARIABLE_IS_NOT_USED, Core::ProxyType<RPC::InvokeMessage>& message) {
            RPC::Data::Input& input(message->Parameters());

            // read parameters
            RPC::Data::Frame::Reader reader(input.Reader());
            const string param0 = reader.Text();
            string param1{}; // storage

            // call implementation
            ILgiTextToSpeech* implementation = reinterpret_cast<ILgiTextToSpeech*>(input.Implementation());
            ASSERT((implementation != nullptr) && "Null ILgiTextToSpeech implementation pointer");
            const uint32_t output = implementation->Resume(param0, param1);

            // write return values
            RPC::Data::Frame::Writer writer(message->Response().Writer());
            writer.Number<const uint32_t>(output);
            writer.Text(param1);
        },

        // virtual uint32_t IsSpeaking(const string&, string&) = 0
        //
        [](Core::ProxyType<Core::IPCChannel>& channel VARIABLE_IS_NOT_USED, Core::ProxyType<RPC::InvokeMessage>& message) {
            RPC::Data::Input& input(message->Parameters());

            // read parameters
            RPC::Data::Frame::Reader reader(input.Reader());
            const string param0 = reader.Text();
            string param1{}; // storage

            // call implementation
            ILgiTextToSpeech* implementation = reinterpret_cast<ILgiTextToSpeech*>(input.Implementation());
            ASSERT((implementation != nullptr) && "Null ILgiTextToSpeech implementation pointer");
            const uint32_t output = implementation->IsSpeaking(param0, param1);

            // write return values
            RPC::Data::Frame::Writer writer(message->Response().Writer());
            writer.Number<const uint32_t>(output);
            writer.Text(param1);
        },

        // virtual uint32_t GetSpeechState(const string&, string&) = 0
        //
        [](Core::ProxyType<Core::IPCChannel>& channel VARIABLE_IS_NOT_USED, Core::ProxyType<RPC::InvokeMessage>& message) {
            RPC::Data::Input& input(message->Parameters());

            // read parameters
            RPC::Data::Frame::Reader reader(input.Reader());
            const string param0 = reader.Text();
            string param1{}; // storage

            // call implementation
            ILgiTextToSpeech* implementation = reinterpret_cast<ILgiTextToSpeech*>(input.Implementation());
            ASSERT((implementation != nullptr) && "Null ILgiTextToSpeech implementation pointer");
            const uint32_t output = implementation->GetSpeechState(param0, param1);

            // write return values
            RPC::Data::Frame::Writer writer(message->Response().Writer());
            writer.Number<const uint32_t>(output);
            writer.Text(param1);
        },

        nullptr
    }; // LgiTextToSpeechStubMethods[]

    //
    // ILgiTextToSpeech::INotification interface stub definitions
    //
    // Methods:
    //  (0) virtual void StateChanged(const string&) = 0
    //  (1) virtual void VoiceChanged(const string&) = 0
    //  (2) virtual void WillSpeak(const string&) = 0
    //  (3) virtual void SpeechStart(const string&) = 0
    //  (4) virtual void SpeechPause(const string&) = 0
    //  (5) virtual void SpeechResume(const string&) = 0
    //  (6) virtual void SpeechCancelled(const string&) = 0
    //  (7) virtual void SpeechInterrupted(const string&) = 0
    //  (8) virtual void NetworkError(const string&) = 0
    //  (9) virtual void PlaybackError(const string&) = 0
    //  (10) virtual void SpeechComplete(const string&) = 0
    //

    ProxyStub::MethodHandler LgiTextToSpeechNotificationStubMethods[] = {
        // virtual void StateChanged(const string&) = 0
        //
        [](Core::ProxyType<Core::IPCChannel>& channel VARIABLE_IS_NOT_USED, Core::ProxyType<RPC::InvokeMessage>& message) {
            RPC::Data::Input& input(message->Parameters());

            // read parameters
            RPC::Data::Frame::Reader reader(input.Reader());
            const string param0 = reader.Text();

            // call implementation
            ILgiTextToSpeech::INotification* implementation = reinterpret_cast<ILgiTextToSpeech::INotification*>(input.Implementation());
            ASSERT((implementation != nullptr) && "Null ILgiTextToSpeech::INotification implementation pointer");
            implementation->StateChanged(param0);
        },

        // virtual void VoiceChanged(const string&) = 0
        //
        [](Core::ProxyType<Core::IPCChannel>& channel VARIABLE_IS_NOT_USED, Core::ProxyType<RPC::InvokeMessage>& message) {
            RPC::Data::Input& input(message->Parameters());

            // read parameters
            RPC::Data::Frame::Reader reader(input.Reader());
            const string param0 = reader.Text();

            // call implementation
            ILgiTextToSpeech::INotification* implementation = reinterpret_cast<ILgiTextToSpeech::INotification*>(input.Implementation());
            ASSERT((implementation != nullptr) && "Null ILgiTextToSpeech::INotification implementation pointer");
            implementation->VoiceChanged(param0);
        },

        // virtual void WillSpeak(const string&) = 0
        //
        [](Core::ProxyType<Core::IPCChannel>& channel VARIABLE_IS_NOT_USED, Core::ProxyType<RPC::InvokeMessage>& message) {
            RPC::Data::Input& input(message->Parameters());

            // read parameters
            RPC::Data::Frame::Reader reader(input.Reader());
            const string param0 = reader.Text();

            // call implementation
            ILgiTextToSpeech::INotification* implementation = reinterpret_cast<ILgiTextToSpeech::INotification*>(input.Implementation());
            ASSERT((implementation != nullptr) && "Null ILgiTextToSpeech::INotification implementation pointer");
            implementation->WillSpeak(param0);
        },

        // virtual void SpeechStart(const string&) = 0
        //
        [](Core::ProxyType<Core::IPCChannel>& channel VARIABLE_IS_NOT_USED, Core::ProxyType<RPC::InvokeMessage>& message) {
            RPC::Data::Input& input(message->Parameters());

            // read parameters
            RPC::Data::Frame::Reader reader(input.Reader());
            const string param0 = reader.Text();

            // call implementation
            ILgiTextToSpeech::INotification* implementation = reinterpret_cast<ILgiTextToSpeech::INotification*>(input.Implementation());
            ASSERT((implementation != nullptr) && "Null ILgiTextToSpeech::INotification implementation pointer");
            implementation->SpeechStart(param0);
        },

        // virtual void SpeechPause(const string&) = 0
        //
        [](Core::ProxyType<Core::IPCChannel>& channel VARIABLE_IS_NOT_USED, Core::ProxyType<RPC::InvokeMessage>& message) {
            RPC::Data::Input& input(message->Parameters());

            // read parameters
            RPC::Data::Frame::Reader reader(input.Reader());
            const string param0 = reader.Text();

            // call implementation
            ILgiTextToSpeech::INotification* implementation = reinterpret_cast<ILgiTextToSpeech::INotification*>(input.Implementation());
            ASSERT((implementation != nullptr) && "Null ILgiTextToSpeech::INotification implementation pointer");
            implementation->SpeechPause(param0);
        },

        // virtual void SpeechResume(const string&) = 0
        //
        [](Core::ProxyType<Core::IPCChannel>& channel VARIABLE_IS_NOT_USED, Core::ProxyType<RPC::InvokeMessage>& message) {
            RPC::Data::Input& input(message->Parameters());

            // read parameters
            RPC::Data::Frame::Reader reader(input.Reader());
            const string param0 = reader.Text();

            // call implementation
            ILgiTextToSpeech::INotification* implementation = reinterpret_cast<ILgiTextToSpeech::INotification*>(input.Implementation());
            ASSERT((implementation != nullptr) && "Null ILgiTextToSpeech::INotification implementation pointer");
            implementation->SpeechResume(param0);
        },

        // virtual void SpeechCancelled(const string&) = 0
        //
        [](Core::ProxyType<Core::IPCChannel>& channel VARIABLE_IS_NOT_USED, Core::ProxyType<RPC::InvokeMessage>& message) {
            RPC::Data::Input& input(message->Parameters());

            // read parameters
            RPC::Data::Frame::Reader reader(input.Reader());
            const string param0 = reader.Text();

            // call implementation
            ILgiTextToSpeech::INotification* implementation = reinterpret_cast<ILgiTextToSpeech::INotification*>(input.Implementation());
            ASSERT((implementation != nullptr) && "Null ILgiTextToSpeech::INotification implementation pointer");
            implementation->SpeechCancelled(param0);
        },

        // virtual void SpeechInterrupted(const string&) = 0
        //
        [](Core::ProxyType<Core::IPCChannel>& channel VARIABLE_IS_NOT_USED, Core::ProxyType<RPC::InvokeMessage>& message) {
            RPC::Data::Input& input(message->Parameters());

            // read parameters
            RPC::Data::Frame::Reader reader(input.Reader());
            const string param0 = reader.Text();

            // call implementation
            ILgiTextToSpeech::INotification* implementation = reinterpret_cast<ILgiTextToSpeech::INotification*>(input.Implementation());
            ASSERT((implementation != nullptr) && "Null ILgiTextToSpeech::INotification implementation pointer");
            implementation->SpeechInterrupted(param0);
        },

        // virtual void NetworkError(const string&) = 0
        //
        [](Core::ProxyType<Core::IPCChannel>& channel VARIABLE_IS_NOT_USED, Core::ProxyType<RPC::InvokeMessage>& message) {
            RPC::Data::Input& input(message->Parameters());

            // read parameters
            RPC::Data::Frame::Reader reader(input.Reader());
            const string param0 = reader.Text();

            // call implementation
            ILgiTextToSpeech::INotification* implementation = reinterpret_cast<ILgiTextToSpeech::INotification*>(input.Implementation());
            ASSERT((implementation != nullptr) && "Null ILgiTextToSpeech::INotification implementation pointer");
            implementation->NetworkError(param0);
        },

        // virtual void PlaybackError(const string&) = 0
        //
        [](Core::ProxyType<Core::IPCChannel>& channel VARIABLE_IS_NOT_USED, Core::ProxyType<RPC::InvokeMessage>& message) {
            RPC::Data::Input& input(message->Parameters());

            // read parameters
            RPC::Data::Frame::Reader reader(input.Reader());
            const string param0 = reader.Text();

            // call implementation
            ILgiTextToSpeech::INotification* implementation = reinterpret_cast<ILgiTextToSpeech::INotification*>(input.Implementation());
            ASSERT((implementation != nullptr) && "Null ILgiTextToSpeech::INotification implementation pointer");
            implementation->PlaybackError(param0);
        },

        // virtual void SpeechComplete(const string&) = 0
        //
        [](Core::ProxyType<Core::IPCChannel>& channel VARIABLE_IS_NOT_USED, Core::ProxyType<RPC::InvokeMessage>& message) {
            RPC::Data::Input& input(message->Parameters());

            // read parameters
            RPC::Data::Frame::Reader reader(input.Reader());
            const string param0 = reader.Text();

            // call implementation
            ILgiTextToSpeech::INotification* implementation = reinterpret_cast<ILgiTextToSpeech::INotification*>(input.Implementation());
            ASSERT((implementation != nullptr) && "Null ILgiTextToSpeech::INotification implementation pointer");
            implementation->SpeechComplete(param0);
        },

        nullptr
    }; // LgiTextToSpeechNotificationStubMethods[]

    // -----------------------------------------------------------------
    // PROXY
    // -----------------------------------------------------------------

    //
    // ILgiTextToSpeech interface proxy definitions
    //
    // Methods:
    //  (0) virtual uint32_t Configure(PluginHost::IShell*) = 0
    //  (1) virtual void Register(ILgiTextToSpeech::INotification*) = 0
    //  (2) virtual void Unregister(ILgiTextToSpeech::INotification*) = 0
    //  (3) virtual uint32_t Enable(const string&, string&) = 0
    //  (4) virtual uint32_t ListVoices(const string&, string&) = 0
    //  (5) virtual uint32_t SetConfiguration(const string&, string&) = 0
    //  (6) virtual uint32_t GetConfiguration(const string&, string&) = 0
    //  (7) virtual uint32_t IsEnabled(const string&, string&) = 0
    //  (8) virtual uint32_t Speak(const string&, string&) = 0
    //  (9) virtual uint32_t Cancel(const string&, string&) = 0
    //  (10) virtual uint32_t Pause(const string&, string&) = 0
    //  (11) virtual uint32_t Resume(const string&, string&) = 0
    //  (12) virtual uint32_t IsSpeaking(const string&, string&) = 0
    //  (13) virtual uint32_t GetSpeechState(const string&, string&) = 0
    //

    class LgiTextToSpeechProxy final : public ProxyStub::UnknownProxyType<ILgiTextToSpeech> {
    public:
        LgiTextToSpeechProxy(const Core::ProxyType<Core::IPCChannel>& channel, RPC::instance_id implementation, const bool otherSideInformed)
            : BaseClass(channel, implementation, otherSideInformed)
        {
        }

        uint32_t Configure(PluginHost::IShell* param0) override
        {
            IPCMessage newMessage(BaseClass::Message(0));

            // write parameters
            RPC::Data::Frame::Writer writer(newMessage->Parameters().Writer());
            writer.Number<RPC::instance_id>(RPC::instance_cast<PluginHost::IShell*>(param0));

            // invoke the method handler
            uint32_t output{};
            if ((output = Invoke(newMessage)) == Core::ERROR_NONE) {
                // read return value
                RPC::Data::Frame::Reader reader(newMessage->Response().Reader());
                output = reader.Number<uint32_t>();

                Complete(reader);
            }

            return output;
        }

        void Register(ILgiTextToSpeech::INotification* param0) override
        {
            IPCMessage newMessage(BaseClass::Message(1));

            // write parameters
            RPC::Data::Frame::Writer writer(newMessage->Parameters().Writer());
            writer.Number<RPC::instance_id>(RPC::instance_cast<ILgiTextToSpeech::INotification*>(param0));

            // invoke the method handler
            if (Invoke(newMessage) == Core::ERROR_NONE) {
                // read return value
                RPC::Data::Frame::Reader reader(newMessage->Response().Reader());
                Complete(reader);
            }
        }

        void Unregister(ILgiTextToSpeech::INotification* param0) override
        {
            IPCMessage newMessage(BaseClass::Message(2));

            // write parameters
            RPC::Data::Frame::Writer writer(newMessage->Parameters().Writer());
            writer.Number<RPC::instance_id>(RPC::instance_cast<ILgiTextToSpeech::INotification*>(param0));

            // invoke the method handler
            if (Invoke(newMessage) == Core::ERROR_NONE) {
                // read return value
                RPC::Data::Frame::Reader reader(newMessage->Response().Reader());
                Complete(reader);
            }
        }

        uint32_t Enable(const string& param0, string& /* out */ param1) override
        {
            IPCMessage newMessage(BaseClass::Message(3));

            // write parameters
            RPC::Data::Frame::Writer writer(newMessage->Parameters().Writer());
            writer.Text(param0);

            // invoke the method handler
            uint32_t output{};
            if ((output = Invoke(newMessage)) == Core::ERROR_NONE) {
                // read return values
                RPC::Data::Frame::Reader reader(newMessage->Response().Reader());
                output = reader.Number<uint32_t>();
                param1 = reader.Text();
            }

            return output;
        }

        uint32_t ListVoices(const string& param0, string& /* out */ param1) override
        {
            IPCMessage newMessage(BaseClass::Message(4));

            // write parameters
            RPC::Data::Frame::Writer writer(newMessage->Parameters().Writer());
            writer.Text(param0);

            // invoke the method handler
            uint32_t output{};
            if ((output = Invoke(newMessage)) == Core::ERROR_NONE) {
                // read return values
                RPC::Data::Frame::Reader reader(newMessage->Response().Reader());
                output = reader.Number<uint32_t>();
                param1 = reader.Text();
            }

            return output;
        }

        uint32_t SetConfiguration(const string& param0, string& /* out */ param1) override
        {
            IPCMessage newMessage(BaseClass::Message(5));

            // write parameters
            RPC::Data::Frame::Writer writer(newMessage->Parameters().Writer());
            writer.Text(param0);

            // invoke the method handler
            uint32_t output{};
            if ((output = Invoke(newMessage)) == Core::ERROR_NONE) {
                // read return values
                RPC::Data::Frame::Reader reader(newMessage->Response().Reader());
                output = reader.Number<uint32_t>();
                param1 = reader.Text();
            }

            return output;
        }

        uint32_t GetConfiguration(const string& param0, string& /* out */ param1) override
        {
            IPCMessage newMessage(BaseClass::Message(6));

            // write parameters
            RPC::Data::Frame::Writer writer(newMessage->Parameters().Writer());
            writer.Text(param0);

            // invoke the method handler
            uint32_t output{};
            if ((output = Invoke(newMessage)) == Core::ERROR_NONE) {
                // read return values
                RPC::Data::Frame::Reader reader(newMessage->Response().Reader());
                output = reader.Number<uint32_t>();
                param1 = reader.Text();
            }

            return output;
        }

        uint32_t IsEnabled(const string& param0, string& /* out */ param1) override
        {
            IPCMessage newMessage(BaseClass::Message(7));

            // write parameters
            RPC::Data::Frame::Writer writer(newMessage->Parameters().Writer());
            writer.Text(param0);

            // invoke the method handler
            uint32_t output{};
            if ((output = Invoke(newMessage)) == Core::ERROR_NONE) {
                // read return values
                RPC::Data::Frame::Reader reader(newMessage->Response().Reader());
                output = reader.Number<uint32_t>();
                param1 = reader.Text();
            }

            return output;
        }

        uint32_t Speak(const string& param0, string& /* out */ param1) override
        {
            IPCMessage newMessage(BaseClass::Message(8));

            // write parameters
            RPC::Data::Frame::Writer writer(newMessage->Parameters().Writer());
            writer.Text(param0);

            // invoke the method handler
            uint32_t output{};
            if ((output = Invoke(newMessage)) == Core::ERROR_NONE) {
                // read return values
                RPC::Data::Frame::Reader reader(newMessage->Response().Reader());
                output = reader.Number<uint32_t>();
                param1 = reader.Text();
            }

            return output;
        }

        uint32_t Cancel(const string& param0, string& /* out */ param1) override
        {
            IPCMessage newMessage(BaseClass::Message(9));

            // write parameters
            RPC::Data::Frame::Writer writer(newMessage->Parameters().Writer());
            writer.Text(param0);

            // invoke the method handler
            uint32_t output{};
            if ((output = Invoke(newMessage)) == Core::ERROR_NONE) {
                // read return values
                RPC::Data::Frame::Reader reader(newMessage->Response().Reader());
                output = reader.Number<uint32_t>();
                param1 = reader.Text();
            }

            return output;
        }

        uint32_t Pause(const string& param0, string& /* out */ param1) override
        {
            IPCMessage newMessage(BaseClass::Message(10));

            // write parameters
            RPC::Data::Frame::Writer writer(newMessage->Parameters().Writer());
            writer.Text(param0);

            // invoke the method handler
            uint32_t output{};
            if ((output = Invoke(newMessage)) == Core::ERROR_NONE) {
                // read return values
                RPC::Data::Frame::Reader reader(newMessage->Response().Reader());
                output = reader.Number<uint32_t>();
                param1 = reader.Text();
            }

            return output;
        }

        uint32_t Resume(const string& param0, string& /* out */ param1) override
        {
            IPCMessage newMessage(BaseClass::Message(11));

            // write parameters
            RPC::Data::Frame::Writer writer(newMessage->Parameters().Writer());
            writer.Text(param0);

            // invoke the method handler
            uint32_t output{};
            if ((output = Invoke(newMessage)) == Core::ERROR_NONE) {
                // read return values
                RPC::Data::Frame::Reader reader(newMessage->Response().Reader());
                output = reader.Number<uint32_t>();
                param1 = reader.Text();
            }

            return output;
        }

        uint32_t IsSpeaking(const string& param0, string& /* out */ param1) override
        {
            IPCMessage newMessage(BaseClass::Message(12));

            // write parameters
            RPC::Data::Frame::Writer writer(newMessage->Parameters().Writer());
            writer.Text(param0);

            // invoke the method handler
            uint32_t output{};
            if ((output = Invoke(newMessage)) == Core::ERROR_NONE) {
                // read return values
                RPC::Data::Frame::Reader reader(newMessage->Response().Reader());
                output = reader.Number<uint32_t>();
                param1 = reader.Text();
            }

            return output;
        }

        uint32_t GetSpeechState(const string& param0, string& /* out */ param1) override
        {
            IPCMessage newMessage(BaseClass::Message(13));

            // write parameters
            RPC::Data::Frame::Writer writer(newMessage->Parameters().Writer());
            writer.Text(param0);

            // invoke the method handler
            uint32_t output{};
            if ((output = Invoke(newMessage)) == Core::ERROR_NONE) {
                // read return values
                RPC::Data::Frame::Reader reader(newMessage->Response().Reader());
                output = reader.Number<uint32_t>();
                param1 = reader.Text();
            }

            return output;
        }
    }; // class LgiTextToSpeechProxy

    //
    // ILgiTextToSpeech::INotification interface proxy definitions
    //
    // Methods:
    //  (0) virtual void StateChanged(const string&) = 0
    //  (1) virtual void VoiceChanged(const string&) = 0
    //  (2) virtual void WillSpeak(const string&) = 0
    //  (3) virtual void SpeechStart(const string&) = 0
    //  (4) virtual void SpeechPause(const string&) = 0
    //  (5) virtual void SpeechResume(const string&) = 0
    //  (6) virtual void SpeechCancelled(const string&) = 0
    //  (7) virtual void SpeechInterrupted(const string&) = 0
    //  (8) virtual void NetworkError(const string&) = 0
    //  (9) virtual void PlaybackError(const string&) = 0
    //  (10) virtual void SpeechComplete(const string&) = 0
    //

    class LgiTextToSpeechNotificationProxy final : public ProxyStub::UnknownProxyType<ILgiTextToSpeech::INotification> {
    public:
        LgiTextToSpeechNotificationProxy(const Core::ProxyType<Core::IPCChannel>& channel, RPC::instance_id implementation, const bool otherSideInformed)
            : BaseClass(channel, implementation, otherSideInformed)
        {
        }

        void StateChanged(const string& param0) override
        {
            IPCMessage newMessage(BaseClass::Message(0));

            // write parameters
            RPC::Data::Frame::Writer writer(newMessage->Parameters().Writer());
            writer.Text(param0);

            // invoke the method handler
            Invoke(newMessage);
        }

        void VoiceChanged(const string& param0) override
        {
            IPCMessage newMessage(BaseClass::Message(1));

            // write parameters
            RPC::Data::Frame::Writer writer(newMessage->Parameters().Writer());
            writer.Text(param0);

            // invoke the method handler
            Invoke(newMessage);
        }

        void WillSpeak(const string& param0) override
        {
            IPCMessage newMessage(BaseClass::Message(2));

            // write parameters
            RPC::Data::Frame::Writer writer(newMessage->Parameters().Writer());
            writer.Text(param0);

            // invoke the method handler
            Invoke(newMessage);
        }

        void SpeechStart(const string& param0) override
        {
            IPCMessage newMessage(BaseClass::Message(3));

            // write parameters
            RPC::Data::Frame::Writer writer(newMessage->Parameters().Writer());
            writer.Text(param0);

            // invoke the method handler
            Invoke(newMessage);
        }

        void SpeechPause(const string& param0) override
        {
            IPCMessage newMessage(BaseClass::Message(4));

            // write parameters
            RPC::Data::Frame::Writer writer(newMessage->Parameters().Writer());
            writer.Text(param0);

            // invoke the method handler
            Invoke(newMessage);
        }

        void SpeechResume(const string& param0) override
        {
            IPCMessage newMessage(BaseClass::Message(5));

            // write parameters
            RPC::Data::Frame::Writer writer(newMessage->Parameters().Writer());
            writer.Text(param0);

            // invoke the method handler
            Invoke(newMessage);
        }

        void SpeechCancelled(const string& param0) override
        {
            IPCMessage newMessage(BaseClass::Message(6));

            // write parameters
            RPC::Data::Frame::Writer writer(newMessage->Parameters().Writer());
            writer.Text(param0);

            // invoke the method handler
            Invoke(newMessage);
        }

        void SpeechInterrupted(const string& param0) override
        {
            IPCMessage newMessage(BaseClass::Message(7));

            // write parameters
            RPC::Data::Frame::Writer writer(newMessage->Parameters().Writer());
            writer.Text(param0);

            // invoke the method handler
            Invoke(newMessage);
        }

        void NetworkError(const string& param0) override
        {
            IPCMessage newMessage(BaseClass::Message(8));

            // write parameters
            RPC::Data::Frame::Writer writer(newMessage->Parameters().Writer());
            writer.Text(param0);

            // invoke the method handler
            Invoke(newMessage);
        }

        void PlaybackError(const string& param0) override
        {
            IPCMessage newMessage(BaseClass::Message(9));

            // write parameters
            RPC::Data::Frame::Writer writer(newMessage->Parameters().Writer());
            writer.Text(param0);

            // invoke the method handler
            Invoke(newMessage);
        }

        void SpeechComplete(const string& param0) override
        {
            IPCMessage newMessage(BaseClass::Message(10));

            // write parameters
            RPC::Data::Frame::Writer writer(newMessage->Parameters().Writer());
            writer.Text(param0);

            // invoke the method handler
            Invoke(newMessage);
        }
    }; // class LgiTextToSpeechNotificationProxy

    // -----------------------------------------------------------------
    // REGISTRATION
    // -----------------------------------------------------------------

    namespace {

        typedef ProxyStub::UnknownStubType<ILgiTextToSpeech, LgiTextToSpeechStubMethods> LgiTextToSpeechStub;
        typedef ProxyStub::UnknownStubType<ILgiTextToSpeech::INotification, LgiTextToSpeechNotificationStubMethods> LgiTextToSpeechNotificationStub;

        static class Instantiation {
        public:
            Instantiation()
            {
                RPC::Administrator::Instance().Announce<ILgiTextToSpeech, LgiTextToSpeechProxy, LgiTextToSpeechStub>();
                RPC::Administrator::Instance().Announce<ILgiTextToSpeech::INotification, LgiTextToSpeechNotificationProxy, LgiTextToSpeechNotificationStub>();
            }
            ~Instantiation()
            {
                RPC::Administrator::Instance().Recall<ILgiTextToSpeech>();
                RPC::Administrator::Instance().Recall<ILgiTextToSpeech::INotification>();
            }
        } ProxyStubRegistration;

    } // namespace

} // namespace ProxyStubs

}
