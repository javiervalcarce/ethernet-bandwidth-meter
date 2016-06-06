// Hi Emacs, this is -*- coding: utf-8; mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#ifndef TELETRAFFIC_NETIF_TABLE_H_
#define TELETRAFFIC_NETIF_TABLE_H_

#include <string>
#include <map>
#include <stdint.h>

namespace teletraffic {

      /**
       * Represents properties of a network interface such as ip address, network mask, mac address, etc.
       *
       * Network byte order. Eg: In "10.1.2.80" 
       *
       * ip4_address[0] == 10 
       * ip4_address[1] == 1 
       * ip4_address[2] == 2 
       * ip4_address[3] == 80
       */
      class NetworkInterface {
      public:

            // Ctor.
            NetworkInterface();

            // For debug purposes
            void Print();
            
            std::string   name;         // Name of the network interface
            bool          is_up;        // If interface is up
            bool          is_plugged;   // If interface network cable is plugged into the socket RJ-45/M12
            bool          is_dormant;
            bool          is_running;

            // IP v4 address and netmask
            bool          ip4_address_valid;   // Indicates if the following IPv4 address is valid or not
            uint8_t       ip4_address[4];
            bool          ip4_netmask_valid;   // Indicates if the following IPv4 netmask is valid or not
            uint8_t       ip4_netmask[4];

            // Physical address, assume IEEE MAC-48 standard (eg: Ethernet)
            bool          mac_address_valid;   // Indicates if the following mac address is valid or not
            uint8_t       mac_address[6]; 
	  
            // Packet send/receive statistics
            bool          sta_valid;           // Indicates if the following statistics are valid or not
            int           sta_tx_packets;
            int           sta_rx_packets;
            int           sta_tx_bytes;
            int           sta_rx_bytes;
      };


      /**
       * Update network interface table.
       *
       * Clears and populates the internal table with the complete list of network interfaces that are UP, that
       * is, network interfaces in DOWN state ***are not included*** in the list.
       */
      int NetworkInterfaceTableUpdate();

      /**
       * Gets a copy of the internal network interface table.
       */
      int NetworkInterfaceTable(std::map<std::string, NetworkInterface>* table);

} 

#endif  // TELETRAFFIC_NETIF_TABLE_H_
