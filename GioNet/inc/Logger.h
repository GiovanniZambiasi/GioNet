#pragma once
#include <memory>
#include <string_view>

namespace GioNet
{
    class Logger
    {
        static std::unique_ptr<Logger> instance;

        bool enabled{true};
        
    public:
        template<typename ...TArgs>
        static void Log(std::string_view fmt, TArgs... args)
        {
            Get().LogImpl(fmt, args...);
        }

    private:
        Logger();
        
        static Logger& Get();

        template<typename ...TArgs>
        void LogImpl(std::string_view fmt, TArgs... args)
        {
            if(!enabled)
                return;

            printf(fmt.data(), args...);
        }
    };
}
