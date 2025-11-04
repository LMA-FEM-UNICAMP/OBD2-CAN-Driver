
#include "obd2_can_driver.hpp"

#include <chrono>
#include <thread>

Obd2CanDriver::Obd2CanDriver(std::string can_in, std::string can_out)
{
    // TODO Setup CAN communication

    socket_in_ = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    strcpy(ifr_in_.ifr_name, can_in.c_str());
    ioctl(socket_in_, SIOCGIFINDEX, &ifr_in_);

    addr_in_.can_family = AF_CAN;
    addr_in_.can_ifindex = ifr_in_.ifr_ifindex;
    bind(socket_in_, (struct sockaddr *)&addr_in_, sizeof(addr_in_));

    // TODO verify is desired PIDs are available

    socket_out_ = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    strcpy(ifr_out_.ifr_name, can_out.c_str());
    ioctl(socket_out_, SIOCGIFINDEX, &ifr_out_);

    addr_out_.can_family = AF_CAN;
    addr_out_.can_ifindex = ifr_out_.ifr_ifindex;
    bind(socket_out_, (struct sockaddr *)&addr_out_, sizeof(addr_out_));

    // TODO Call for OBD2 PIDs

    can_frame_t response_frame;

    // response_frame = obd2_request(0x00);
    pids_.push_back(0x0C);
    pids_.push_back(0x0D);
    pids_.push_back(0x11);

    // TODO verify is desired PIDs are available

    is_new_data_ = false;
}

Obd2CanDriver::~Obd2CanDriver()
{
    std::cout << "Closing obd2_can_driver" << std::endl;

    close(socket_in_);
    close(socket_out_);
}

can_frame_t Obd2CanDriver::obd2_request(uint8_t pid)
{
    std::cout << std::endl << "Requesting pid " << static_cast<int>(pid) << std::endl;

    can_frame_t request_frame;
    can_frame_t response_frame;

    int attempts = 0;

    request_frame.can_id = 0x7DF; // Broadcast ID
    request_frame.can_dlc = 8;    // Frame data size

    memset(request_frame.data, 0, sizeof(request_frame.data)); // Setting frame data to 0

    request_frame.data[0] = 0x02; // Number of bytes
    request_frame.data[1] = 0x01; // Service 01 (Show current data)
    request_frame.data[2] = pid;  // PID

    // TODO send to CAN

        std::cout << "Sending request" << std::endl << std::endl;

    int sendbytes = write(socket_in_, &request_frame, sizeof(can_frame_t));

    // TODO get response

    int nbytes = read(socket_in_, &response_frame, sizeof(can_frame_t));

    std::cout << "Response received" << std::endl;

    std::cout << "PID received: " << static_cast<int>(response_frame.data[2]) << std::endl;

    if (nbytes > 0 && (response_frame.can_id & 0xFF0) == 0x7E0 && response_frame.data[2] == pid)
    {
        return response_frame;
    }

    // ? Try again?

    else
    {
        response_frame.can_id = 0x00;

        std::cout << "Wrong PID received, skipping..." << std::endl;

        return response_frame;
    }
}

bool Obd2CanDriver::read_obd2()
{
    for (uint8_t pid : pids_)
    {
        can_frame_t response_frame;

        response_frame = obd2_request(pid);

        if (response_frame.can_id == 0x00)
        {
            continue;
        }

        switch (pid)
        {
        case 0x0C: // Engine RPM
            engine_rpm_ = static_cast<double>(((response_frame.data[3] * 256.0) + response_frame.data[4]) / 4.0);
            is_new_data_ = true;
            std::cout << "engine_rpm_: " << engine_rpm_ << std::endl;
            break;

        case 0x0D: // Longitudinal Speed
            longitudinal_speed_ = static_cast<double>(response_frame.data[3]);
            is_new_data_ = true;
            std::cout << "longitudinal_speed_: " << longitudinal_speed_ << std::endl;
            break;

        case 0x11: // Throttle Position
            throttle_position_ = static_cast<double>(response_frame.data[3] * 100.0 / 255.0);
            is_new_data_ = true;
            std::cout << "throttle_position_: " << throttle_position_ << std::endl;
            break;

        default:
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 1;
}

bool Obd2CanDriver::send_data()
{

    if (is_new_data_)
    {
        is_new_data_ = false;

        can_frame_t send_frame;

        send_frame.can_id = 0x7DF; // ID
        send_frame.can_dlc = 8;    // Frame data size

        memset(send_frame.data, 0, sizeof(send_frame.data)); // Setting frame data to 0

        send_frame.data[0] = 0x00;
        // TODO ...

        int sendbytes = write(socket_out_, &send_frame, sizeof(can_frame_t));

        return 1;
    }

    return 0;
}