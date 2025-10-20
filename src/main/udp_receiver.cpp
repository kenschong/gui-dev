#include "udp_receiver.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <cerrno>
#include <cmath>

UDPReceiver::UDPReceiver(int port)
    : port(port), sockfd(-1), running(false), dataReceived(false) {
    memset(&latestPacket, 0, sizeof(JoystickInputPacket));
}

UDPReceiver::~UDPReceiver() {
    stop();
}

bool UDPReceiver::start() {
    if (running) {
        std::cerr << "UDP Receiver already running" << std::endl;
        return false;
    }

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "Failed to create socket: " << strerror(errno) << std::endl;
        return false;
    }

    // Set socket options to allow reuse
    int optval = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        std::cerr << "Failed to set SO_REUSEADDR: " << strerror(errno) << std::endl;
        close(sockfd);
        sockfd = -1;
        return false;
    }

    // Set non-blocking mode with timeout
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        std::cerr << "Failed to set socket timeout: " << strerror(errno) << std::endl;
        close(sockfd);
        sockfd = -1;
        return false;
    }

    // Bind socket to port
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Failed to bind socket to port " << port << ": "
                  << strerror(errno) << std::endl;
        close(sockfd);
        sockfd = -1;
        return false;
    }

    // Start receive thread
    running = true;
    receiveThread = std::thread(&UDPReceiver::receiveLoop, this);

    std::cout << "UDP Receiver started on port " << port << std::endl;
    return true;
}

void UDPReceiver::stop() {
    if (!running) {
        return;
    }

    running = false;

    // Wait for thread to finish
    if (receiveThread.joinable()) {
        receiveThread.join();
    }

    // Close socket
    if (sockfd >= 0) {
        close(sockfd);
        sockfd = -1;
    }

    std::cout << "UDP Receiver stopped" << std::endl;
}

void UDPReceiver::receiveLoop() {
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    JoystickInputPacket packet;

    while (running) {
        // Receive data
        ssize_t bytesReceived = recvfrom(sockfd, &packet, sizeof(JoystickInputPacket),
                                         0, (struct sockaddr*)&clientAddr,
                                         &clientAddrLen);

        if (bytesReceived < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // Timeout, continue loop
                continue;
            } else {
                std::cerr << "Error receiving data: " << strerror(errno) << std::endl;
                break;
            }
        }

        if (bytesReceived == sizeof(JoystickInputPacket)) {
            // Validate packet data
            bool valid = true;

            // Check for NaN or infinity in float values
            if (std::isnan(packet.rollInput) || std::isinf(packet.rollInput) ||
                std::isnan(packet.pitchInput) || std::isinf(packet.pitchInput) ||
                std::isnan(packet.yawInput) || std::isinf(packet.yawInput)) {
                std::cerr << "UDP Receiver: Invalid float values (NaN/Inf) detected" << std::endl;
                valid = false;
            }

            // Check range validation (allow slightly beyond expected range for tolerance)
            if (std::abs(packet.rollInput) > JOYSTICK_INPUT_TOLERANCE ||
                std::abs(packet.pitchInput) > JOYSTICK_INPUT_TOLERANCE ||
                std::abs(packet.yawInput) > JOYSTICK_INPUT_TOLERANCE) {
                std::cerr << "UDP Receiver: Input values out of acceptable range (>"
                          << JOYSTICK_INPUT_TOLERANCE << ")" << std::endl;
                valid = false;
            }

            // Only update if packet is valid
            if (valid) {
                // Update latest packet (thread-safe)
                {
                    std::lock_guard<std::mutex> lock(dataMutex);
                    latestPacket = packet;
                }

                if (!dataReceived) {
                    dataReceived = true;
                    std::cout << "UDP Receiver: First joystick packet received from "
                              << inet_ntoa(clientAddr.sin_addr) << ":"
                              << ntohs(clientAddr.sin_port) << std::endl;
                }
            }
        } else {
            std::cerr << "Received invalid packet size: " << bytesReceived
                      << " (expected " << sizeof(JoystickInputPacket) << ")" << std::endl;
        }
    }
}

bool UDPReceiver::getLatestInput(JoystickInputPacket& packet) {
    if (!dataReceived) {
        return false;
    }

    std::lock_guard<std::mutex> lock(dataMutex);
    packet = latestPacket;
    return true;
}

void UDPReceiver::reset() {
    std::lock_guard<std::mutex> lock(dataMutex);
    dataReceived = false;
    memset(&latestPacket, 0, sizeof(JoystickInputPacket));
    std::cout << "UDP Receiver: Reset (cleared received data)" << std::endl;
}
