// Hi Emacs, this is -*- mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
/**************************************************************************
 * @file        netif_table.h
 * @brief       Implementation of utility functions to query all system 
 *              network interfaces.
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
#include "netif_table.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <unistd.h>
#include <pthread.h>

#ifdef __linux__
#include <linux/if_link.h>
#include <linux/if_packet.h>
#include <linux/if.h>
#endif

#include <net/ethernet.h>
#include <sys/ioctl.h>
// for IFF_UP, IFF_LOWER_UP, etc
//#include <net/if.h>

// c++
#include <cstdio>
#include <cstdlib>
#include <cstring>


using std::string;
using std::map;
using teletraffic::NetworkInterface;
using teletraffic::NetworkInterfaceTable;

/*************************************************************************/
/* DECLARACION DE TIPOS DE DATOS, CONSTANTES Y VARIABLES INTERNOS        */
/* DECLARATION OF INTERNAL DATA TYPES, CONSTANTS AND VARS                */
/*************************************************************************/
static pthread_mutex_t       s_lock = PTHREAD_MUTEX_INITIALIZER;
static NetworkInterfaceTable s_table;

/*************************************************************************/
/* DECLARACION DE PROTOTIPOS DE FUNCIONES INTERNAS                       */
/* DECLARATION OF INTERNAL FUNCTIONS PROTOTYPES                          */
/*************************************************************************/




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int teletraffic::NetworkInterfaceTableUpdate() {

      struct ifaddrs *ifaddr;
      struct ifaddrs *ifa;

      string name;
      int family;

      if (getifaddrs(&ifaddr) == -1) {
            return 1;
      }


      pthread_mutex_lock(&s_lock); // Cerrojo para poder modificar la variable s_table, accedida por
                                   // NetworkInterfaceTable() desde otros hilos...


      // Clear table and fill it again with the info provided by getifaddress()
      s_table.clear();

      // Walk through linked list, maintaining head pointer so we can free list later
      for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {

            if (ifa->ifa_addr == NULL) {
                  continue;
            }

            name   = ifa->ifa_name;
            family = ifa->ifa_addr->sa_family;

            // If this interface is not yet in the table, add it
            if (s_table.find(name) == s_table.end()) {
                  s_table[name].name = name;                 // copy
            }
            
            NetworkInterface& nif = s_table[name]; 

            if (family == AF_INET) {
                  
                  if (ifa->ifa_addr == NULL) {
                        nif.ip4_address_valid = false;
                  } else {
                        nif.ip4_address_valid = true;
                        struct sockaddr_in* address4 = (struct sockaddr_in*) ifa->ifa_addr;
                        memcpy(nif.ip4_address, &(address4->sin_addr), 4);
                  }
                  
                  if (ifa->ifa_netmask == NULL) {
                        nif.ip4_netmask_valid = false;
                  } else {
                        nif.ip4_netmask_valid = true;
                        struct sockaddr_in* netmask4 = (struct sockaddr_in*) ifa->ifa_netmask;                        
                        memcpy(nif.ip4_netmask, &(netmask4->sin_addr), 4);
                  }

#ifdef __linux__
                  // Flags
                  nif.is_up      = (ifa->ifa_flags & IFF_UP) != 0;
                  nif.is_plugged = (ifa->ifa_flags & IFF_LOWER_UP) != 0;
                  nif.is_dormant = (ifa->ifa_flags & IFF_DORMANT) != 0;
                  nif.is_running = (ifa->ifa_flags & IFF_RUNNING) != 0;

                  
            } else if (family == AF_INET6) {
                  // TODO
            } else if (family == AF_PACKET) {

                  // Physical address and statistics
                  struct sockaddr_ll*     addressp = (struct sockaddr_ll*) ifa->ifa_addr;
                  struct rtnl_link_stats* stats    = (rtnl_link_stats*)    ifa->ifa_data;

                  if (addressp == NULL) {
                        nif.mac_address_valid = false;
                  } else {
                        nif.mac_address_valid = true;                  
                        memcpy(nif.mac_address, addressp->sll_addr, 6);
                  }

                  if (stats == NULL) {
                        nif.sta_valid = false;
                  } else {
                  
                        nif.sta_valid = true;
                        nif.sta_tx_packets = stats->tx_packets;
                        nif.sta_rx_packets = stats->rx_packets;
                        nif.sta_tx_bytes = stats->tx_bytes;
                        nif.sta_rx_bytes = stats->rx_bytes;
                  }
            }
            
#else
            }
#endif
      
      
      }

      pthread_mutex_unlock(&s_lock);

      freeifaddrs(ifaddr);
      return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int teletraffic::NetworkInterfaceTableCopy(NetworkInterfaceTable* table) {  
      if (table == NULL) {
            return 1;
      }

      pthread_mutex_lock(&s_lock);
      // Copy interfaces table inside a critical region and return that.
      *table = s_table;
      pthread_mutex_unlock(&s_lock);

      return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int teletraffic::NetworkInterfaceTablePrint(NetworkInterface& table) {

      printf("Interface %s\n", table.name.c_str());

      if (table.ip4_address_valid == false) {
	    printf("  IP4 address: -\n");
      } else {
	    printf("  IP4 address: %u.%u.%u.%u\n", 
		   table.ip4_address[0], 
		   table.ip4_address[1], 
		   table.ip4_address[2], 
		   table.ip4_address[3]);
      }

      if (table.ip4_netmask_valid == false) {
	    printf("  IP4 netmask: -\n");
      } else {
	    printf("  IP4 netmask: %u.%u.%u.%u\n", 
		   table.ip4_netmask[0], 
		   table.ip4_netmask[1], 
		   table.ip4_netmask[2], 
		   table.ip4_netmask[3]);
      }

      if (table.mac_address_valid == false) {
	    printf("  MAC address: -\n");
      } else {
	    printf("  MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n", 
		   table.mac_address[0], 
		   table.mac_address[1], 
		   table.mac_address[2], 
		   table.mac_address[3], 
		   table.mac_address[4], 
		   table.mac_address[5]);
      }

      if (table.sta_valid == false) {
	    printf("  Statistics : -\n");
      } else {
	    printf("  Statistics : txp=%u rxp=%u txb=%u rxb=%u\n",
		   table.sta_tx_packets,
		   table.sta_rx_packets,
		   table.sta_tx_bytes,
		   table.sta_rx_bytes);
      }

      printf("   Up: %s\n", table.is_up      ? "true" : "false");
      printf("   PL: %s\n", table.is_plugged ? "true" : "false");
      printf("   DR: %s\n", table.is_dormant ? "true" : "false");
      printf("   RU: %s\n", table.is_running ? "true" : "false");

      return 0;      
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
