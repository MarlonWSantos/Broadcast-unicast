all: broadcast-unicast 
APPS=servreg-hack
CONTIKI=../../

CONTIKI_WITH_IPV6 = 1
include $(CONTIKI)/Makefile.include
