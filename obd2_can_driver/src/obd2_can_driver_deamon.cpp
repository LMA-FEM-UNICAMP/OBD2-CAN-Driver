
#include "obd2_can_driver.hpp"

#include <chrono>
#include <thread>

int main()
{

    std::cout << "Starting obd2_can_driver" << std::endl;

    Obd2CanDriver obd2_can_driver("vcan0", "vcan1");

    while (1)
    {
        obd2_can_driver.read_obd2();
        obd2_can_driver.send_data_to_can_out();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}