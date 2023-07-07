
#ifndef RTPLAYERNOTIFY_H
#define RTPLAYERNOTIFY_H

namespace WPEFramework {

    class RTPlayerNotify {
    public:
        RTPlayerNotify() {};
        virtual ~RTPlayerNotify() {};

        virtual rtError onEvent(int argc, rtValue const* argv, rtValue* result) = 0;
    };


} // namespace

#endif /* RTPLAYERNOTIFY_H */

