#pragma once
#include "osc/OscReceivedElements.h"
#include "osc/OscPacketListener.h"
#include "ip/UdpSocket.h"
#include "AudioMath.h"



class ExamplePacketListener : public osc::OscPacketListener {

protected:

    virtual void ProcessMessage(const osc::ReceivedMessage& m,
        const IpEndpointName& remoteEndpoint);

public:
    float* storedMessage = new float[3];
};