
#include "obd2_can_driver.hpp"

#include <chrono>
#include <thread>

Obd2CanDriver::Obd2CanDriver(std::string can_in, std::string can_out)
{

    /// OBD2 CAN communication
    socket_in_ = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    strcpy(ifr_in_.ifr_name, can_in.c_str());
    ioctl(socket_in_, SIOCGIFINDEX, &ifr_in_);

    addr_in_.can_family = AF_CAN;
    addr_in_.can_ifindex = ifr_in_.ifr_ifindex;
    bind(socket_in_, (struct sockaddr *)&addr_in_, sizeof(addr_in_));
    /// OBU CAN communication
    socket_out_ = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    strcpy(ifr_out_.ifr_name, can_out.c_str());
    ioctl(socket_out_, SIOCGIFINDEX, &ifr_out_);

    addr_out_.can_family = AF_CAN;
    addr_out_.can_ifindex = ifr_out_.ifr_ifindex;
    bind(socket_out_, (struct sockaddr *)&addr_out_, sizeof(addr_out_));

    //* Adding vehicle speed PID as default
    add_pid(VEHICLE_SPEED_PID);

    is_new_data_ = false;
    requesting_.store(true);

    /* Logging */
    // TODO is not working
    sockfd_log_ = socket(AF_UNIX, SOCK_DGRAM, 0);
    memset(&addr_log_, 0, sizeof(addr_log_));
    addr_log_.sun_family = AF_UNIX;
    strcpy(addr_log_.sun_path, LOG_SOCK_PATH);

    obd2_logging("Starting obd2_can_driver...\n\n");
}

Obd2CanDriver::~Obd2CanDriver()
{
    std::cout << "Closing obd2_can_driver" << std::endl;

    close(socket_in_);
    close(socket_out_);
    close(client_log_);
}

void Obd2CanDriver::add_pid(uint8_t pid)
{
    pids_.push_back(pid);
}

void Obd2CanDriver::obd2_logging(char *msg)
{
    sendto(sockfd_log_, msg, strlen(msg), 0, (struct sockaddr *)&addr_log_, sizeof(addr_log_));
}

bool Obd2CanDriver::obd2_request(uint8_t pid)
{
    can_frame_t request_frame;

    /// Sending OBD2 request for the desired PID

    request_frame.can_id = 0x7DF; // Broadcast ID
    request_frame.can_dlc = 8;    // Frame data size

    memset(request_frame.data, 0, sizeof(request_frame.data)); // Setting frame data to 0

    request_frame.data[0] = 0x02; // Number of bytes
    request_frame.data[1] = 0x01; // Service 01 (Show current data)
    request_frame.data[2] = pid;  // PID

    int sendbytes = write(socket_in_, &request_frame, sizeof(can_frame_t));

    return (sendbytes > 0) ? 1 : 0;
}

void Obd2CanDriver::obd2_requester()
{
    while (requesting_.load())
    {
        for (uint8_t pid : pids_)
        {
            bool request_success = obd2_request(pid);
            std::this_thread::sleep_for(std::chrono::milliseconds(REQUEST_DELAY));
        }
    }
}

void Obd2CanDriver::configure_requester(bool state)
{
    requesting_.store(state);
}

can_frame_t Obd2CanDriver::obd2_response()
{
    can_frame_t response_frame;

    /// Reading OBD2 response

    int nbytes = read(socket_in_, &response_frame, sizeof(can_frame_t));

    /// Checking if the response was successful, the ID is valid and the PID is right
    if (nbytes > 0 && (response_frame.can_id & 0xFF0) == 0x7E0) // && response_frame.data[2] == pid)
    {
        return response_frame;
    }

    /// If some is wrong, skip this request and return a invalid message
    else
    {
        response_frame.can_id = 0x00;

        return response_frame;
    }
}

bool Obd2CanDriver::read_obd2()
{

    char logging_buffer[128];

    can_frame_t response_frame;

    response_frame = obd2_response();

    if (response_frame.can_id == 0x00)
    {
        return 0;
    }

    switch (response_frame.data[2])
    {
    case ENGINE_SPEED_PID: // Engine RPM
        engine_rpm_ = static_cast<double>(((response_frame.data[3] * 256.0) + response_frame.data[4]) / 4.0);

        std::cout << "Engine Speed [RPM]: " << engine_rpm_ << std::endl;

        snprintf(logging_buffer, sizeof(logging_buffer), "Engine Speed [RPM]:  %.2f\n", engine_rpm_);
        obd2_logging(logging_buffer);
        break;

    case VEHICLE_SPEED_PID: // Longitudinal Speed
        longitudinal_speed_ = static_cast<int>(response_frame.data[3]);
        is_new_data_ = true;

        std::cout << "Vehicle Speed [km/h]: " << longitudinal_speed_ << std::endl;

        snprintf(logging_buffer, sizeof(logging_buffer), "Vehicle Speed [km/h]: %d\n", longitudinal_speed_);
        obd2_logging(logging_buffer);
        break;

    case THROTTLE_PEDAL_POSITION_PID: // Throttle Position
        throttle_position_ = static_cast<double>(response_frame.data[3] * 100.0 / 255.0);

        std::cout << "Throttle Position [%]: " << throttle_position_ << std::endl;

        snprintf(logging_buffer, sizeof(logging_buffer), "Throttle Position [%%]: %.2f\n", throttle_position_);
        obd2_logging(logging_buffer);
        break;

    default:
        break;
    }

    return 1;
}

bool Obd2CanDriver::send_data_to_can_out()
{
    //* From SDK exampleETSI can-vstate.c:
    //  #define CANVSTATE_CANID 0x123
    //  Handle the 'example' CANVSTATE_CANID CAN message
    //  Bytes 0 & 1 are MSB/LSB of Vehicle Speed (in cm per sec)
    //  Byte 2 is Vehicle Speed Confidence (in cm per sec)
    //  Byte 3 is Detected Lane Position
    //  Bytes 5 & 6 are MSB/LSB of Longitudinal Acceleration (in mm per sec per sec)
    //  Byte 7 is Longitudinal Acceleration Confidence (in mm per sec per sec)

    if (is_new_data_)
    {
        is_new_data_ = false;

        can_frame_t send_frame;

        send_frame.can_id = CANVSTATE_CANID; // Example message from Cohda
        send_frame.can_dlc = 8;              // Frame data size

        memset(send_frame.data, 0, sizeof(send_frame.data)); // Setting frame data to 0

        u_int16_t vehicle_speed_cm_per_s = static_cast<u_int16_t>(longitudinal_speed_ / 3.6 * 100.0); // longitudinal_speed_ in km/h to cm/s

        u_int16_t longitudinal_acceleration_mm_per_s2 = 0; // longitudinal_acceleration_ in m/s^2 to mm/s^2

        /// Assembling CANVSTATE_CANID packet
        send_frame.data[0] = vehicle_speed_cm_per_s >> 8;                // Vehicle Speed MSB
        send_frame.data[1] = vehicle_speed_cm_per_s & 0xFF;              // Vehicle Speed LSB
        send_frame.data[2] = 0x00;                                       // ? Vehicle Speed Confidence
        send_frame.data[3] = 0x00;                                       // ? Detected Lane Position
        send_frame.data[5] = longitudinal_acceleration_mm_per_s2 >> 8;   // ? Longitudinal Acceleration MSB
        send_frame.data[6] = longitudinal_acceleration_mm_per_s2 & 0xFF; // ? Longitudinal Acceleration LSB
        send_frame.data[7] = 0x00;                                       // ? Longitudinal Acceleration confidence

        int sendbytes = write(socket_out_, &send_frame, sizeof(can_frame_t));

        return 1;
    }

    return 0;
}

bool Obd2CanDriver::send_data_to_can_out(can_frame_t send_frame)
{
    int sendbytes = write(socket_out_, &send_frame, sizeof(can_frame_t));

    return (sendbytes > 0) ? 1 : 0;
}