#include <chrono>
#include <stdexcept>
#include <thread>
#define TRACY_ENABLE 1
#include "tracy/Tracy.hpp"
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
//#include <tracy/Tracy.hpp>
#include "cista.h"

#include <string>

#include <iostream>

#include <format>

#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>


SteamNetworkingMicroseconds g_logTimeZero;

using std::string;

// taken wholesale, maybe I can figure this out later
static void InitSteamDatagramConnectionSockets()
{
	#ifdef STEAMNETWORKINGSOCKETS_OPENSOURCE
		SteamDatagramErrMsg errMsg;
		if ( !GameNetworkingSockets_Init( nullptr, errMsg ) )
			throw std::runtime_error(std::format("GameNetworkingSockets_Init failed.  %s", errMsg));
	#else
		SteamDatagram_SetAppID( 570 ); // Just set something, doesn't matter what
		SteamDatagram_SetUniverse( false, k_EUniverseDev );

		SteamDatagramErrMsg errMsg;
		if ( !SteamDatagramClient_Init( errMsg ) )
			FatalError( "SteamDatagramClient_Init failed.  %s", errMsg );

		// Disable authentication when running with Steam, for this
		// example, since we're not a real app.
		//
		// Authentication is disabled automatically in the open-source
		// version since we don't have a trusted third party to issue
		// certs.
		SteamNetworkingUtils()->SetGlobalConfigValueInt32( k_ESteamNetworkingConfig_IP_AllowWithoutAuth, 1 );
	#endif

	g_logTimeZero = SteamNetworkingUtils()->GetLocalTimestamp(); // I think this is just for logs

	//SteamNetworkingUtils()->SetDebugOutputFunction( k_ESteamNetworkingSocketsDebugOutputType_Msg, DebugOutput );
}



class Server {

    static Server* instance;

    static void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t *pInfo) {
        if(instance != nullptr) {
            instance->connectionStatusChanged();
        } else {
            throw std::runtime_error("server instance is null");
        }
    }

    void connectionStatusChanged() {
        std::cout << "net connection status changed" << std::endl;
    }

    public:
        void server(uint16 nPort) {

            

            std::cout << "starting server" << std::endl;

            auto m_pInterface = SteamNetworkingSockets();

            SteamNetworkingIPAddr serverLocalAddr; // the address of this server
            serverLocalAddr.Clear();
            serverLocalAddr.m_port = nPort;

            SteamNetworkingConfigValue_t opt;
            opt.SetPtr( k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)SteamNetConnectionStatusChangedCallback );

            auto m_hListenSock = m_pInterface->CreateListenSocketIP( serverLocalAddr, 1, &opt );
            if ( m_hListenSock == k_HSteamListenSocket_Invalid )
                std::runtime_error(std::format( "Failed to listen on port %d", nPort ));
            auto m_hPollGroup = m_pInterface->CreatePollGroup();
            if ( m_hPollGroup == k_HSteamNetPollGroup_Invalid )
                std::runtime_error(std::format( "Failed to listen on port %d", nPort ));
            std::cout << "Server listening on port " << nPort << std::endl;

            while(true) {
                instance = this;
                m_pInterface->RunCallbacks();
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
};

Server* Server::instance = nullptr;

class Client {

    static Client* instance;

    static void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t *pInfo) {
        if(instance != nullptr) {
            instance->connectionStatusChanged(pInfo);
        }
    }

    void connectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *pInfo) {

        switch ( pInfo->m_info.m_eState )
		{
			case k_ESteamNetworkingConnectionState_None:
				std::cout << "network status none" << std::endl;
				break;

			case k_ESteamNetworkingConnectionState_ClosedByPeer:
                std::cout << "closed by peer" << std::endl;
                break;
			case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
			{
                std::cout << "problem detected locally: " << pInfo->m_info.m_szEndDebug  << std::endl;
                break;
			}

			case k_ESteamNetworkingConnectionState_Connecting:
				std::cout << "connecting..." << std::endl;
                break;

			case k_ESteamNetworkingConnectionState_Connected:
				std::cout << "connected!" << std::endl;
                break;

			default:
				// Silences -Wswitch
				break;
		}
    }

    public:
        void client(uint16 nPort) {
            std::cout << "starting client" << std::endl;

            auto m_pInterface = SteamNetworkingSockets();

            SteamNetworkingIPAddr clientLocalAddr;
            clientLocalAddr.SetIPv6LocalHost(nPort);
            //clientLocalAddr.m_port = nPort;

            SteamNetworkingConfigValue_t opt;
            opt.SetPtr( k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)SteamNetConnectionStatusChangedCallback );

            auto m_hConnection = m_pInterface->ConnectByIPAddress( clientLocalAddr, 1, &opt );

            if ( m_hConnection == k_HSteamNetConnection_Invalid )
			    std::runtime_error( "Failed to create connection" );

            while(true) {
                instance = this;
		        m_pInterface->RunCallbacks();
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
};

Client* Client::instance = nullptr;

int main() {
    
    auto m_pInterface = SteamNetworkingSockets();

    uint16 nPort = 27020;

    SteamNetworkingIPAddr serverLocalAddr;
    serverLocalAddr.Clear();
    serverLocalAddr.m_port = nPort;

    InitSteamDatagramConnectionSockets(); // initalizes the ports?

    std::cout << "client or server?" << std::endl;
    string input;
    std::cin >> input;
    if(input == "server") {
        Server server;
        server.server(nPort);
    } else {
        Client client;
        client.client(nPort);
    }
    


}


