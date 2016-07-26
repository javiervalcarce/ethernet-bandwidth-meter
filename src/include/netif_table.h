// Hi Emacs, this is -*- coding: utf-8; mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#ifndef TELETRAFFIC_NETIF_TABLE_H_
#define TELETRAFFIC_NETIF_TABLE_H_
/**************************************************************************
 * @file        netif_table.h
 * @brief       Declares a pair of functions that serve to query all system 
 *              network interfaces and get its mac/ip addresses, state, etc
 * @author      JVG
 * @date        2016-07-19
 * @copyright   SEPSA, 2016
 * @copyright   COPYRIGHT NOTICE:
 * @copyright
 * This software is property of SEPSA and has been forwarded to:
 * ............................, under the terms of contract:
 * ............................
 * Unauthorized duplication and disclosure to third parties is absolutely
 * forbidden.
 * This software is property of SEPSA. Its reproduction, total or partial,
 * by any means, in any form, permanent or temporary, is forbidden unless
 * explicitly authorised by SEPSA.
 * Any adaptation, amendment, translation or transformation, as well as the
 * decompiling or disassembly of this software product, shall only be
 * performed with the explicit authorization of SEPSA.
 * The user of the present software product shall be allowed to make a
 * backup copy as long as it is necessary for the utilization of the same.
 * The terms stated above shall be understood  affecting that stated in
 * applicable Laws.
 *
 * CONTROLES COMPILACION CONDICIONAL/CONDITIONAL COMPILATION CONTROLS:
 * Control        | Valor/Value | Proposito/Purpose
 * -------------- | ----------- | ----------------------------------------
 * .............. | ........... | ........................................
 * .............. | ........... | ........................................
 * .............. | ........... | ........................................
 * 
 *************************************************************************/


/*************************************************************************/
/* LITERALES, MACROS, TIPOS DE DATOS Y FUNCIONES IMPORTADAS              */
/* IMPORTED LITERALS, MACROS, DATA TYPES AND FUNCTIONS                   */
/*************************************************************************/
#include <string>
#include <map>
#include <stdint.h>


namespace teletraffic {


      /**
       * Represents a network interface and its properties such as its name, ip address, network mask, mac address, and
       * some basic statisdics, etc.
       */
      struct NetworkInterface {

            // Default ctor.
            NetworkInterface() {
                  is_up = false;
                  is_plugged = false;
                  is_dormant = false;
                  is_running = false;
                  ip4_address_valid = false;
                  ip4_netmask_valid = false;
                  mac_address_valid = false;
                  sta_valid = false;
            }

            std::string   name;         // Name of the network interface

            // Properties of that network interface
            bool          is_up;        // If interface is up
            bool          is_plugged;   // If interface network cable is plugged into the socket RJ-45/M12
            bool          is_dormant;   // If interface is dormant
            bool          is_running;   // If interface is running


            // IP v4 address and netmask. 
            // The IP address is in 'network byte order'. eg: In "10.1.2.80" 
            // ip4_address[0] == 10 
            // ip4_address[1] == 1 
            // ip4_address[2] == 2 
            // ip4_address[3] == 80
            bool          ip4_address_valid;   // Indicates if the following IPv4 address is valid or not
            uint8_t       ip4_address[4];
            bool          ip4_netmask_valid;   // Indicates if the following IPv4 netmask is valid or not
            uint8_t       ip4_netmask[4];

            // Physical address, assume an IEEE 802.2 MAC address (48 bits)
            bool          mac_address_valid;   // Indicates if the following mac address is valid or not
            uint8_t       mac_address[6]; 
	  
            // Packet send/receive statistics
            bool          sta_valid;           // Indicates if the following statistics are valid or not
            int           sta_tx_packets;      // Number of packets transmitted
            int           sta_rx_packets;      // Number of packets received 
            int           sta_tx_bytes;        // Number of bytes transmitted
            int           sta_rx_bytes;        // Number of bytes transmitted
      };



      /**
       * Network Interface Table (STL map) 
       * 
       * The key is the name of the network interface, for example: "eth0", "lo", etc. The value is a NetworkInterface
       * object that contains information about that interface, see teletraffic::NetworkInterface struct.
       */
      typedef std::map<std::string, NetworkInterface> NetworkInterfaceTable;


      /**
       * Update the internal network interface table.
       *
       * Clears and populates the internal table with the complete list of network interfaces that are UP, that
       * is, network interfaces in DOWN state ***are not included*** in the list, take that into account.
       */
      int NetworkInterfaceTableUpdate();

      /**
       * Gets a copy of the internal network interface table.
       * 
       * @param table Pointer to a valid NetworkInterfaceTable object wich will be filled. The application owners that
       * object and is responsible for its destruction.
       */
      int NetworkInterfaceTableCopy(NetworkInterfaceTable* table);

      /**
       * For debug purposes. 
       * Print all structure's members with printf() to stdout.
       */
      int NetworkInterfaceTablePrint(NetworkInterface& table);

} 

#endif  // TELETRAFFIC_NETIF_TABLE_H_
