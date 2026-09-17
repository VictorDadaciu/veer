#include "log.h"

#include <chrono>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <iomanip>
#include <ios>
#include <memory>
#include <mutex>
#include <ostream>
#include <queue>
#include <string>
#include <thread>

#define VEER_TO_SECONDS(x) std::chrono::duration<float, std::chrono::seconds::period>(x).count()

namespace ve::log
{
using clock = std::chrono::steady_clock;

static std::string log_level_as_string(level l)
{
    switch (l)
    {
    case level::debug:
        return "DEBUG:   ";
    case level::trace:
        return "TRACE:   ";
    case level::info:
        return "INFO:    ";
    case level::warning:
        return "WARNING: ";
    case level::error:
        return "ERROR:   ";
    default:
        return "";
    }
}

struct message
{
    clock::time_point time_point{};
    std::string msg{};
    level l{};
};

class logger
{
public:
    logger(std::ostream& ostream, const std::string& app_name, level l) :
        output(ostream), name(app_name), base_level(l)
    {
        beginning = clock::now();
        thread = std::thread(
            [this]() -> void
            {
                while (true)
                {
                    message msg;
                    {
                        std::unique_lock<std::mutex> lock(mutex);
                        cv.wait(lock,
                                [this]() -> bool
                                {
                                    return !queue.empty() || stop;
                                });

                        if (stop && queue.empty())
                        {
                            return;
                        }

                        msg = queue.front();
                        queue.pop();
                    }

                    output << "[" << std::fixed << std::setprecision(6)
                           << VEER_TO_SECONDS(msg.time_point - beginning)
                           << "] " << name << " " << log_level_as_string(msg.l) << msg.msg << "\n";
                }
            });
    }

    ~logger()
    {
        {
            std::unique_lock<std::mutex> lock(mutex);
            stop = true;
        }
        cv.notify_all();
        thread.join();
    }

    std::ostream& output;
    std::string name{};
    level base_level{};
    std::queue<message> queue{};
    std::thread thread{};
    std::mutex mutex{};
    std::condition_variable cv{};
    clock::time_point beginning{};

    bool stop{};
};

static std::unique_ptr<logger> _logger{nullptr};

void init(const std::string& app_name, level base_level, std::ostream& ostream)
{
    assert(!_logger);
    _logger = std::make_unique<logger>(ostream, app_name, base_level);
}

void set_level(level l) noexcept
{
    assert(_logger);
    _logger->base_level = l;
}
}

namespace ve
{
using namespace log;
static void _log(level l, const std::string& msg)
{
    assert(_logger);
    if (l < _logger->base_level)
        return;
    {
        std::unique_lock<std::mutex> lock(_logger->mutex);
        _logger->queue.push(message{.time_point = clock::now(), .msg = msg, .l = l});
    }
    _logger->cv.notify_one();
}

void debug(const std::string& msg)
{
    _log(level::debug, msg);
}

void trace(const std::string& msg)
{
    _log(level::trace, msg);
}

void info(const std::string& msg)
{
    _log(level::info, msg);
}

void warn(const std::string& msg)
{
    _log(level::warning, msg);
}

error_code warn(error_code err, const std::string& msg)
{
    _log(level::warning, msg);
    return err;
}

void error(const std::string& msg)
{
    _log(level::error, msg);
}

error_code error(error_code err, const std::string& msg)
{
    _log(level::error, msg);
    return err;
}
}
