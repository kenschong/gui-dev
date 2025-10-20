// Test program to send joystick inputs to the GUI via UDP
// Compile: g++ -o test_udp_sender test_udp_sender.cpp -I ../main
// Usage: ./test_udp_sender [host] [port]

#include "../main/udp_protocol.h"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cmath>
#include <cstdlib>
#include <ctime>

int main(int argc, char* argv[]) {
    const char* host = (argc > 1) ? argv[1] : "127.0.0.1";
    int port = (argc > 2) ? atoi(argv[2]) : UDP_DEFAULT_PORT;

    // Create UDP socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }

    // Setup server address
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);

    if (inet_pton(AF_INET, host, &serverAddr.sin_addr) <= 0) {
        std::cerr << "Invalid address: " << host << std::endl;
        close(sockfd);
        return 1;
    }

    std::cout << "UDP Joystick Test Sender" << std::endl;
    std::cout << "========================" << std::endl;
    std::cout << "Sending to " << host << ":" << port << std::endl;
    std::cout << "Press Ctrl+C to stop" << std::endl;
    std::cout << std::endl;
    std::cout << "Sending sinusoidal joystick inputs..." << std::endl;
    std::cout << "  - Roll:  Slow sine wave" << std::endl;
    std::cout << "  - Pitch: Medium sine wave" << std::endl;
    std::cout << "  - Yaw:   Fast sine wave" << std::endl;
    std::cout << std::endl;

    // Seed random number generator and randomize initial time
    srand(time(NULL));
    float time = ((float)rand() / RAND_MAX) * 10.0f;  // Random time between 0 and 10
    std::cout << "Starting with random time offset: " << time << std::endl;
    std::cout << std::endl;

    JoystickInputPacket packet;
    uint32_t frameCount = 0;

    while (true) {
        // Generate sinusoidal joystick inputs at different frequencies
        packet.rollInput = 50.0f * sin(time * 0.5f);        // Slow oscillation
        packet.pitchInput = 50.0f * sin(time * 1.0f);       // Medium oscillation
        packet.yawInput = 50.0f * sin(time * 2.0f);         // Fast oscillation
        packet.timestamp = frameCount++;

        // Send packet
        ssize_t bytesSent = sendto(sockfd, &packet, sizeof(packet), 0,
                                   (struct sockaddr*)&serverAddr, sizeof(serverAddr));

        if (bytesSent < 0) {
            std::cerr << "Failed to send packet" << std::endl;
            break;
        }

        // Print status for every packet
        std::cout << "Sent packet " << frameCount
                  << " - Roll: " << packet.rollInput
                  << ", Pitch: " << packet.pitchInput
                  << ", Yaw: " << packet.yawInput << std::endl;

        // Increment time and sleep (every 5 minutes)
        time += 0.2f;
        sleep(300); // 300 seconds = 5 minutes
    }

    close(sockfd);
    return 0;
}
