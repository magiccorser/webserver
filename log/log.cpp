#include "log.h"

log *log::get_instance()
{
    static log instance;
    return &instance;
}

void log::log_write(const std::string &msg)
{
    log_queue.push(msg);
}

int log::log_size()
{
    return log_queue.size();
}
