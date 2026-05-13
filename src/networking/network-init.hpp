#pragma once
#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>

#include <iostream>
#include <string>
#include <format>

using std::string;

namespace Network {
    static bool networkSlotsInitalized = false;
    // taken wholesale, maybe I can figure this out later
    inline static void InitSteamDatagramConnectionSockets()
    {
        if(networkSlotsInitalized) return;

        #ifdef STEAMNETWORKINGSOCKETS_OPENSOURCE
            SteamDatagramErrMsg errMsg;
            if ( !GameNetworkingSockets_Init( nullptr, errMsg ) )
                throw std::runtime_error(std::format("GameNetworkingSockets_Init failed.  %s", errMsg));
            networkSlotsInitalized = true;
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

        //g_logTimeZero = SteamNetworkingUtils()->GetLocalTimestamp(); // I think this is just for logs

        //SteamNetworkingUtils()->SetDebugOutputFunction( k_ESteamNetworkingSocketsDebugOutputType_Msg, DebugOutput );
    }
}