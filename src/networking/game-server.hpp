#pragma once
#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>

#include <iostream>
#include <string>
#include <assert.h>
#include <format>
#include <map>
#include <functional>

#include "network-init.hpp"

#include "message-sender.hpp"

#include "cista.h"

typedef unsigned int ActorID;

using std::string;

class ConnectedClient {
    friend class GameServer;

    public:
        string name;
        HSteamNetConnection connection;
        ActorID actorID;

        ConnectedClient() = default;

        ConnectedClient(string name,HSteamNetConnection connection) : name(name), connection(connection) {

        }
        string getName() {
            return name;
        }
};

template<typename T>
struct IncomingMessageServer {
    ConnectedClient* client = nullptr;
    T contents;

    IncomingMessageServer() {

    }

    IncomingMessageServer(ConnectedClient* client,T message) : client(client), contents(message) { }
};

class GameServer : public IMessageSender {

    inline static GameServer* instance = nullptr;
    ISteamNetworkingSockets *m_pInterface = {};
    HSteamNetPollGroup m_hPollGroup = {};
    HSteamListenSocket m_hListenSock = {};

    std::map<string,std::function<void(IncomingMessageServer<string>)>> messageCallbacks;

    std::map<HSteamNetConnection,ConnectedClient> connections;

    std::function<void(ConnectedClient)> newConnectionCallback = nullptr;

    static void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t *pInfo) {
        if(instance != nullptr) {
            instance->connectionStatusChanged(pInfo);
        } else {
            throw std::runtime_error("server instance is null");
        }
    }

    void addConnectedClient(HSteamNetConnection connection) {
        connections[connection] = ConnectedClient("user"+std::to_string(connections.size()),connection);
        m_pInterface->SetConnectionPollGroup(connection,m_hPollGroup);
        if(newConnectionCallback) {
            newConnectionCallback(connections[connection]);
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
                m_pInterface->SetConnectionPollGroup(pInfo->m_hConn,k_HSteamNetPollGroup_Invalid);
                m_pInterface->CloseConnection(pInfo->m_hConn,0,"Left",false);
                break;
			case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
			{
                std::cout << "problem detected locally: " << pInfo->m_info.m_szEndDebug  << std::endl;
                break;
			}

			case k_ESteamNetworkingConnectionState_Connecting:
				std::cout << "connecting..." << std::endl;
                if ( m_pInterface->AcceptConnection( pInfo->m_hConn ) != k_EResultOK )
				{
                    m_pInterface->CloseConnection( pInfo->m_hConn, 0, nullptr, false );
                    std::cout << "can't accept connection..." << std::endl;
                }
                
                break;

			case k_ESteamNetworkingConnectionState_Connected:
				std::cout << "connected!" << std::endl;
                addConnectedClient(pInfo->m_hConn);
                break;

			default:
				// Silences -Wswitch
				break;
		}
    }
    public:

        GameServer() {
            Network::InitSteamDatagramConnectionSockets();
            m_pInterface = SteamNetworkingSockets();
        }

        void handleConnections() {
            instance = this;
            m_pInterface->RunCallbacks();
        }
        void openSocket(uint16 nPort) {

            SteamNetworkingIPAddr serverLocalAddr; // the address of this server
            serverLocalAddr.Clear();
            serverLocalAddr.m_port = nPort;

            SteamNetworkingConfigValue_t opt;
            opt.SetPtr( k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)SteamNetConnectionStatusChangedCallback );

            m_hListenSock = m_pInterface->CreateListenSocketIP( serverLocalAddr, 1, &opt );
            if ( m_hListenSock == k_HSteamListenSocket_Invalid )
                std::runtime_error(std::format( "Failed to listen on port %d", nPort ));
            m_hPollGroup = m_pInterface->CreatePollGroup();
            if ( m_hPollGroup == k_HSteamNetPollGroup_Invalid )
                std::runtime_error(std::format( "Failed to listen on port %d", nPort ));
            std::cout << "Server listening on port " << nPort << std::endl;

        }

        template <typename T>
        void sendRawMessageToClient(const ConnectedClient& client,string type,T contents) {
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

            
            m_pInterface->SendMessageToConnection( client.connection, message.c_str(), (uint32)message.length(), k_nSteamNetworkingSend_Reliable, nullptr );
        }

        void sendRawMessage(string type,cista::byte_buf contents) override {
            sendRawMessageToAllClients(type,contents);
        }

        template <typename T>
        void sendMessageToClient(const ConnectedClient& client,T contents) {
            auto byte_buf = cista::serialize(contents);

            sendRawMessageToClient(client,T::getMessageType().c_str(),byte_buf);
        }

        template <typename T>
        void sendRawMessageToAllClients(string type,T contents) {
            for(auto& pair : connections) {
                sendRawMessageToClient(pair.second,type,contents);
            }
        }

        template <typename T>
        void sendMessageToAllClientsExcept(ConnectedClient* client,T contents) {
            for(auto& pair : connections) {
                if(&pair.second != client) {
                    sendMessageToClient(pair.second,contents);
                }
            }
        }

        template <typename T>
        void sendMessageToAllClients(T contents) {
            for(auto& pair : connections) {
                sendMessageToClient(pair.second,contents);
            }
        }

        void pollMessages() {
            while(true) { // keep receiving messages until theres none left
                ISteamNetworkingMessage *pIncomingMsg = nullptr;
                int numMsgs = m_pInterface->ReceiveMessagesOnPollGroup( m_hPollGroup, &pIncomingMsg, 1 );
                if ( numMsgs == 0 )
                    //std::cout << "no messages : (" << std::endl; 
                    return;
                if ( numMsgs < 0 )
                    throw std::runtime_error( "Error checking for messages" );
                assert( numMsgs == 1 && pIncomingMsg ); // the 1 in ReceiveMessagesOnPollGroup means we only receive 1 message maximum

                
                if(!connections.contains(pIncomingMsg->m_conn)) {
                    throw std::runtime_error("message from unknown connection");
                }
                if(pIncomingMsg->m_cbSize < 4) {
                    std::cout << "message size less than 4, discarded" << std::endl;
                    break;
                }
                
                string messageType;

                messageType.assign((const char *)pIncomingMsg->m_pData,4);
                
                
                std::string messageContents;

                messageContents.assign( (const char *)pIncomingMsg->m_pData+4, pIncomingMsg->m_cbSize-4 ); //copy the data over

                auto* client = &connections.at(pIncomingMsg->m_conn);
                if(messageCallbacks.contains(messageType)) {
                    messageCallbacks.at(messageType)(IncomingMessageServer<string>{client,messageContents});
                }

                
                pIncomingMsg->Release();
            }
        }

        void setClientName(ConnectedClient* client,string name) {
            assert(client != nullptr);
            assert(connections.contains(client->connection));
            connections.at(client->connection).name = name;
        }

        // this shouldn't be here honestly. but its fine
        void setClientActorID(ConnectedClient* client,ActorID id) {
            assert(client != nullptr);
            assert(connections.contains(client->connection));
            connections.at(client->connection).actorID = id;
        }

        void addRawMessageCallback(string type,std::function<void(IncomingMessageServer<string>)> func) {
            messageCallbacks[type] = func;
        }

        template<typename T>
        void addMessageCallback(std::function<void(IncomingMessageServer<T>)> func) {
            addRawMessageCallback(T::getMessageType(),[func](IncomingMessageServer<string> message) {
                T* ptr = cista::deserialize<T>(message.contents);

                if(ptr != nullptr) {
                    func(IncomingMessageServer<T>(message.client,*ptr));
                }
            });
        }

        void setNewConnectionCallback(std::function<void(ConnectedClient)> func) {
            newConnectionCallback = func;
        }

        void close() {
            m_pInterface->CloseListenSocket( m_hListenSock );
            m_hListenSock = k_HSteamListenSocket_Invalid;

            m_pInterface->DestroyPollGroup( m_hPollGroup );
            m_hPollGroup = k_HSteamNetPollGroup_Invalid;
        }

};