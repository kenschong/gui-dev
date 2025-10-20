#ifndef UDP_RECEIVER_H
#define UDP_RECEIVER_H

#include "udp_protocol.h"
#include <string>
#include <atomic>
#include <thread>
#include <mutex>

class UDPReceiver {
public:
    UDPReceiver(int port = UDP_DEFAULT_PORT);
    ~UDPReceiver();

    // Start/stop the receiver
    bool start();
    void stop();

    // Check if receiver is running
    bool isRunning() const { return running; }

    // Get latest joystick input data (thread-safe)
    bool getLatestInput(JoystickInputPacket& packet);

    // Get connection status
    bool hasReceivedData() const { return dataReceived; }

    // Reset the receiver state (clears received data flag)
    void reset();

    // Get port number
    int getPort() const { return port; }

private:
    void receiveLoop();

    int port;
    int sockfd;
    std::atomic<bool> running;
    std::atomic<bool> dataReceived;
    std::thread receiveThread;

    // Thread-safe data storage
    std::mutex dataMutex;
    JoystickInputPacket latestPacket;
};

#endif // UDP_RECEIVER_H
