#include <iostream>
#include <cstring>

#if defined(__BORLANDC__) // workaround for BCB4 release build intrinsics bug
namespace std {
    using ::__strcmp__;  // avoid error: E2316 '__strcmp__' is not a member of 'std'.
}
#endif

#include "osc/OscReceivedElements.h"
#include "osc/OscPacketListener.h"
#include "ip/UdpSocket.h"
#include "osc.h"




void ExamplePacketListener::ProcessMessage(const osc::ReceivedMessage& m,
        const IpEndpointName& remoteEndpoint)
    {
        (void)remoteEndpoint; // suppress unused parameter warning

        try {
            // example of parsing single messages. osc::OsckPacketListener
            // handles the bundle traversal.

            if (std::strcmp(m.AddressPattern(), "/wek/outputs") == 0) {
                // example #1 -- argument stream interface
                osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
                float a1;
                float a2;
                float a3;
                args >> a1 >> a2 >> a3 >> osc::EndMessage;
                storedMessage[0] = a1;
                storedMessage[1] = a2;
                storedMessage[2] = a3;
                // std::cout << "received '/wek/inputs' message in osc handler with arguments: " << storedMessage[0] << " " << storedMessage[1] << " " << storedMessage[2] << "\n";

                

            }
        }
        catch (osc::Exception& e) {
            // any parsing errors such as unexpected argument types, or 
            // missing arguments get thrown as exceptions.
            std::cout << "error while parsing message: "
                << m.AddressPattern() << ": " << e.what() << "\n";
        }
    }