class RdkLoggerMilestoneImpl {
public:
    RdkLoggerMilestoneImpl()
    {
    }
    virtual ~RdkLoggerMilestoneImpl() = default;
    virtual void logMilestone(const char *msg_code) = 0;
};

class RdkLoggerMilestone {
public:

    static RdkLoggerMilestone& getInstance()
    {
        static RdkLoggerMilestone instance;
        return instance;
    }

    RdkLoggerMilestoneImpl* impl;

    static void logMilestone(const char *msg_code)
    {
	   getInstance().impl->logMilestone(msg_code);
    }

};
constexpr auto logMilestone = &RdkLoggerMilestone::logMilestone;
