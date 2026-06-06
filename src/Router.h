/*
 * Noxim - the NoC Simulator
 *
 * (C) 2005-2018 by the University of Catania
 * For the complete list of authors refer to file ../doc/AUTHORS.txt
 * For the license applied to these sources refer to file ../doc/LICENSE.txt
 *
 * This file contains the declaration of the router
 */

#ifndef __NOXIMROUTER_H__
#define __NOXIMROUTER_H__

#include "Buffer.h"
#include "DataStructs.h"
#include "GlobalRoutingTable.h"
#include "LocalRoutingTable.h"
#include "ReservationTable.h"
#include "Stats.h"
#include "Utils.h"
#include "routingAlgorithms/RoutingAlgorithm.h"
#include "routingAlgorithms/RoutingAlgorithms.h"
#include "selectionStrategies/SelectionStrategy.h"
#include "selectionStrategies/Selection_BUFFER_LEVEL.h"
#include "selectionStrategies/Selection_NOP.h"
#include <systemc.h>

using namespace std;

extern unsigned int drained_volume;

SC_MODULE(Router) {
  friend class Selection_NOP;
  friend class Selection_BUFFER_LEVEL;

  // I/O Ports
  sc_in_clk clock;   // The input clock for the router
  sc_in<bool> reset; // The reset signal for the router

  // number of ports: 6 mesh directions + 4xlocal + wireless
  sc_in<Flit> flit_rx[ROUTER_PORTS]; // The input channels
  sc_in<bool> req_rx[ROUTER_PORTS]; // The requests associated with the input
                                    // channels
  sc_out<bool> ack_rx[ROUTER_PORTS]; // The outgoing ack signals
                                           // associated with the input channels
  sc_out<TBufferFullStatus> buffer_full_status_rx[ROUTER_PORTS];

  sc_out<Flit> flit_tx[ROUTER_PORTS]; // The output channels
  sc_out<bool> req_tx[ROUTER_PORTS]; // The requests associated with the output
                                     // channels
  sc_in<bool> ack_tx[ROUTER_PORTS];  // The outgoing ack signals associated
                                     // with the output channels
  sc_in<TBufferFullStatus> buffer_full_status_tx[ROUTER_PORTS];

  sc_out<int> free_slots[DIRECTION_HUB];
  sc_in<int> free_slots_neighbor[DIRECTION_HUB];

  // Neighbor-on-Path related I/O
  sc_out<NoP_data> NoP_data_out[DIRECTIONS];
  sc_in<NoP_data> NoP_data_in[DIRECTIONS];

  // Registers

  int local_id; // Unique ID
  bool is_memory_pe;
  int routing_type; // Type of routing algorithm
  int selection_type;
  BufferBank buffer[ROUTER_PORTS]; // buffer[direction][virtual_channel]
  BufferBank buffer_mid[ROUTER_PORTS];
  BufferBank buffer_out[ROUTER_PORTS];
  bool current_level_rx[ROUTER_PORTS]; // Current level for Alternating Bit
                                       // Protocol (ABP)
  bool current_level_tx[ROUTER_PORTS]; // Current level for Alternating Bit
                                       // Protocol (ABP)
  Stats stats;              // Statistics
  Power power;
  LocalRoutingTable routing_table;    // Routing table
  ReservationTable reservation_table; // Switch reservation table
  unsigned long routed_flits;
  RoutingAlgorithm *routingAlgorithm;
  SelectionStrategy *selectionStrategy;

  // Functions

  void process();
  void rxProcess(); // The receiving process
  void txProcess(); // The transmitting process
  void perCycleUpdate();
  void configure(const int _id, const double _warm_up_time,
                 const unsigned int _max_input_buffer_size,
                 const unsigned int _max_output_buffer_size,
                 GlobalRoutingTable &grt);

  unsigned long getRoutedFlits(); // Returns the number of routed flits

  // Constructor

  SC_CTOR(Router) {
    SC_METHOD(process);
    sensitive << reset;
    sensitive << clock.pos();

    SC_METHOD(perCycleUpdate);
    sensitive << reset;
    sensitive << clock.pos();

    routingAlgorithm = RoutingAlgorithms::get(GlobalParams::routing_algorithm);

    if (routingAlgorithm == 0) {
      cerr << " FATAL: invalid routing -routing "
           << GlobalParams::routing_algorithm << ", check with noxim -help"
           << endl;
      exit(-1);
    }

    selectionStrategy =
        SelectionStrategies::get(GlobalParams::selection_strategy);

    if (selectionStrategy == 0) {
      cerr << " FATAL: invalid selection strategy -sel "
           << GlobalParams::selection_strategy << ", check with noxim -help"
           << endl;
      exit(-1);
    }
  }

private:
  // performs actual routing + selection
  int route(const RouteData &route_data);

  // wrappers
  int selectionFunction(const vector<int> &directions,
                        const RouteData &route_data);
  vector<int> routingFunction(const RouteData &route_data);

  NoP_data getCurrentNoPData();
  void NoP_report() const;
  int NoPScore(const NoP_data &nop_data, const vector<int> &nop_channels) const;
  int reflexDirection(int direction) const;
  int getNeighborId(int _id, int direction) const;

  vector<int> getNextHops(int src, int dst);
  int start_from_port; // Port from which to start the reservation cycle

  bool reservation_status[ROUTER_PORTS][MAX_VIRTUAL_CHANNELS];
  vector<pair<int, int>> reservation_queue;

  bool out_reservation_status[ROUTER_PORTS][MAX_VIRTUAL_CHANNELS];
  queue<int> out_reservation_queue[ROUTER_PORTS];
  int cur_out_vc[ROUTER_PORTS];

  bool is_vc_set[ROUTER_PORTS];

  vector<int> nextDeltaHops(RouteData rd);

public:
  unsigned int local_drained;

  bool inCongestion();
  void ShowBuffersStats(std::ostream & out);

  bool connectedHubs(int src_hub, int dst_hub);
};

#endif
