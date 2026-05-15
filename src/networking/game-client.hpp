#pragma once
#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>

#include <iostream>
#include <string>
#include <assert.h>
#include <functional>
#include <map>

#include "network-init.hpp"

#include "message-sender.hpp"

#include "cista.h"

using std::string;

enum class ClientStatus {
    Disconnected, // not connected :(
    Connecting, // we've received a connecting callback
    Connected, // we are currently connected to the server!
};

template<typename T>
struct IncomingMessageClient {
    T contents;

    IncomingMessageClient() {

    }

    IncomingMessageClient(T message) : contents(message) { }
};

class GameClient : public IMessageSender {

    static GameClient* instance;

    ISteamNetworkingSockets *m_pInterface = nullptr;
    HSteamNetConnection m_hConnection = {};

    std::map<string,std::function<void(IncomingMessageClient<string>)>> messageCallbacks;

    ClientStatus status = ClientStatus::Disconnected;

    string name;

    static void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t *pInfo) {
        if(instance != nullptr) {
            instance->connectionStatusChanged(pInfo);
        }
    }

    void connectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *pInfo) {

        switch ( pInfo->m_info.m_eState )
		{
			case k_ESteamNetworkingConnectionState_None:
				break;

			case k_ESteamNetworkingConnectionState_ClosedByPeer:
                break;

			case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
			{
                std::cout << "problem detected locally: " << pInfo->m_info.m_szEndDebug  << std::endl;
                status = ClientStatus::Disconnected;
                m_pInterface->CloseConnection( pInfo->m_hConn, 0, nullptr, false );
				m_hConnection = k_HSteamNetConnection_Invalid;
                break;
			}

			case k_ESteamNetworkingConnectionState_Connecting:
				std::cout << "connecting..." << std::endl;
                status = ClientStatus::Connecting;
                break;

			case k_ESteamNetworkingConnectionState_Connected:
				std::cout << "connected!" << std::endl;
                status = ClientStatus::Connected;
                sendRawMessage((string)"JOIN",name);
                break;

			default:
				// Silences -Wswitch
				break;
		}
    }


    public:

        GameClient() { 
            Network::InitSteamDatagramConnectionSockets();
            m_pInterface = SteamNetworkingSockets();
        }

        bool isConnected() {
            return status == ClientStatus::Connected;
        }

        ClientStatus getStatus() {
            return status;
        }

        void setName(string name) {
            this->name = name;
        }

        void connectRemote(string address,uint16 nPort) {

            std::cout << "connecting client" << std::endl;

            SteamNetworkingIPAddr clientLocalAddr;
            clientLocalAddr.ParseString(address.c_str());
            clientLocalAddr.m_port = nPort;

            SteamNetworkingConfigValue_t opt;
            opt.SetPtr( k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)SteamNetConnectionStatusChangedCallback );

            m_hConnection = m_pInterface->ConnectByIPAddress( clientLocalAddr, 1, &opt );

            if ( m_hConnection == k_HSteamNetConnection_Invalid )
			    std::runtime_error( "Failed to create connection" );
        }

        void pollMessages() {
            if ( m_hConnection == k_HSteamNetConnection_Invalid ) return;
            while(true) { // keep receiving messages until theres none left
                ISteamNetworkingMessage *pIncomingMsg = nullptr;
                int numMsgs = m_pInterface->ReceiveMessagesOnConnection( m_hConnection, &pIncomingMsg, 1 );
                if ( numMsgs == 0 )
                    //std::cout << "no messages : (" << std::endl; 
                    return;
                if ( numMsgs < 0 )
                    throw std::runtime_error( "Error checking for messages" );
                assert( numMsgs == 1 && pIncomingMsg ); // the 1 in ReceiveMessagesOnPollGroup means we only receive 1 message maximum
                
                string messageType;

                messageType.assign((const char *)pIncomingMsg->m_pData,4);
                
                
                std::string messageContents;

                messageContents.assign( (const char *)pIncomingMsg->m_pData+4, pIncomingMsg->m_cbSize-4 ); //copy the data over

                if(messageCallbacks.contains(messageType)) {
                    messageCallbacks.at(messageType)(IncomingMessageClient<string>{messageContents});
                }

                
                pIncomingMsg->Release();
            }
        }

        void addRawMessageCallback(string type,std::function<void(IncomingMessageClient<string>)> func) {
            messageCallbacks[type] = func;
        }

        template<typename T>
        void addMessageCallback(std::function<void(IncomingMessageClient<T>)> func) {
            addRawMessageCallback(T::getMessageType(),[func](IncomingMessageClient<string> message) {
                T* ptr = cista::deserialize<T>(message.contents);

                if(ptr != nullptr) {
                    func(IncomingMessageClient<T>(*ptr));
                }
            });
        }

        void disconnect() {
            m_pInterface->CloseConnection( m_hConnection, 0, "Goodbye", true );
            m_hConnection = k_HSteamNetConnection_Invalid;
            status = ClientStatus::Disconnected;
        }

        void handleConnections() {
            instance = this;
		    m_pInterface->RunCallbacks();
        }

        void sendRawMessage(string type,cista::byte_buf contents) override {
            sendRawMessage<cista::byte_buf>(type,contents);
        }

        template <typename T>
        void sendRawMessage(string type,T contents) {
            assert(type.size() >= 4);
            string message;
            message.insert(message.end(),type.c_str(),type.c_str()+4);
            string contentsStr;
            if constexpr(std::is_same_v<T,string>) {
                contentsStr = contents;
            } else {
                contentsStr.assign((const char *)contents.data(),contents.size());
            }
            message += contentsStr;

            
            m_pInterface->SendMessageToConnection(m_hConnection, message.c_str(), (uint32)message.length(), k_nSteamNetworkingSend_Reliable, nullptr );
        }

        template <typename T>
        void sendMessage(T contents) {
            auto byte_buf = cista::serialize(contents);

            sendRawMessage(T::getMessageType().c_str(),byte_buf);
        }
};

GameClient* GameClient::instance = nullptr;
