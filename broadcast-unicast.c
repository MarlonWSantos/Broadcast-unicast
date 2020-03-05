/*
 * Copyright (c) 2011, Swedish Institute of Computer Science.
 * Copyright (c) 2020, Federal University of Para.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/**
 * \file
 *      broadcast-unicast
 * \author
 *      Marlon W. Santos <marlon.santos.santos@icen.ufpa.br>
 */

#include "contiki.h"
#include "lib/random.h"
#include "sys/ctimer.h"
#include "sys/etimer.h"
#include "net/ip/uip.h"
#include "net/ip/uiplib.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ip/uip-debug.h"
#include "sys/node-id.h"
#include "simple-udp.h"
#include "servreg-hack.h"
#include "net/rpl/rpl.h"
#include <stdio.h>
#include <string.h>


#define UDP_PORT 1234
#define SERVICE_ID 190

#define SEND_INTERVAL		(10 * CLOCK_SECOND)
#define SEND_TIME		(random_rand() % (SEND_INTERVAL))

static struct simple_udp_connection unicast_connection;
static struct simple_udp_connection broadcast_connection;
unsigned int is_event=1;


/*---------------------------------------------------------------------------*/
PROCESS(broadcast_unicast_process, "Broadcast-unicast example process");
AUTOSTART_PROCESSES(&broadcast_unicast_process);
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
static void
receiver(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
  printf("Data received from ");
  uip_debug_ipaddr_print(sender_addr);
  printf(" on port %d from port %d with length %d: '%s'\n",
         receiver_port, sender_port, datalen, data);

uip_ipaddr_t *addr;

uip_ipaddr_copy(addr, sender_addr);

  uip_ip6addr(&addr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&addr, &uip_lladdr);
  uip_ds6_addr_add(&addr, 0, ADDR_AUTOCONF);


    if(addr != NULL) {
      static unsigned int message_number;
      char buf[20];

      printf("Sending unicast to ");
      uip_debug_ipaddr_print(addr);
      printf("\n");
      sprintf(buf, "Message %d", message_number);
      message_number++;
      simple_udp_sendto(&unicast_connection, buf, strlen(buf) + 1, addr);
    } else {
      printf("Service %d not found\n", SERVICE_ID);
    }
}

 
/*---------------------------------------------------------------------------*/




/*---------------------------------------------------------------------------*/
PROCESS_THREAD(broadcast_unicast_process, ev, data)
{
  static struct etimer periodic_timer;
  static struct etimer send_timer;
  uip_ipaddr_t addr;

  PROCESS_BEGIN();

  simple_udp_register(&broadcast_connection, UDP_PORT,
                      NULL, UDP_PORT,
                      receiver);

 simple_udp_register(&unicast_connection, UDP_PORT,
                     NULL, UDP_PORT, receiver);


   etimer_set(&periodic_timer, SEND_INTERVAL);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    etimer_reset(&periodic_timer);
    etimer_set(&send_timer, SEND_TIME);

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&send_timer));

	if(is_event==1){
    printf("Sending broadcast\n");
    uip_create_linklocal_allnodes_mcast(&addr);
    simple_udp_sendto(&broadcast_connection, "Test", 4, &addr);
	}
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/


