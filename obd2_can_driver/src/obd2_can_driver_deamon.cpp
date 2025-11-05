
#include "obd2_can_driver.hpp"

#include <chrono>
#include <thread>

int main(int argc, char *argv[])
{

    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " <string1> <string2>\n";
        return 1;
    }

    std::cout << "Starting obd2_can_driver" << std::endl;

    std::cout << "Using " << argv[1] << " as OBD2 CAN interface" << std::endl;
    std::cout << "Using " << argv[2 ] << " as OBU CAN interface" << std::endl;

    Obd2CanDriver obd2_can_driver(argv[1], argv[2]);

    while (1)
    {
        obd2_can_driver.read_obd2();
        obd2_can_driver.send_data_to_can_out();
    }

    return 0;
}