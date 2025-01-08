#ifndef MAX31865_TASK_HPP
#define MAX31865_TASK_HPP

namespace MAX31865 {

    void start_task() noexcept;

    void task(void* parameter) noexcept;

}; // namespace MAX31865

#endif // MAX31865_TASK_HPP