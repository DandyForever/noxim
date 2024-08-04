/*
 * Noxim - the NoC Simulator
 *
 * (C) 2005-2018 by the University of Catania
 * For the complete list of authors refer to file ../doc/AUTHORS.txt
 * For the license applied to these sources refer to file ../doc/LICENSE.txt
 *
 * This file contains the declaration of the processing element
 */

#ifndef __NOXIMPROCESSINGELEMENT_H__
#define __NOXIMPROCESSINGELEMENT_H__

#include <queue>
#include <systemc.h>

#include "DataStructs.h"
#include "GlobalTrafficTable.h"
#include "Utils.h"

using namespace std;

enum RequestType {
  WRITE,
  READ,
};

SC_MODULE(ProcessingElement) {

  // I/O Ports
  sc_in_clk clock;   // The input clock for the PE
  sc_in<bool> reset; // The reset signal for the PE

  //-------------------------------------------------------------------------
  // X channel
  sc_in<Flit> flit_rx; // The input channel
  sc_in<bool> req_rx;  // The request associated with the input channel
  sc_out<bool>
      ack_rx; // The outgoing ack signal associated with the input channel
  sc_out<TBufferFullStatus> buffer_full_status_rx;

  sc_out<Flit> flit_tx; // The output channel
  sc_out<bool> req_tx;  // The request associated with the output channel
  sc_in<bool>
      ack_tx; // The outgoing ack signal associated with the output channel
  sc_in<TBufferFullStatus> buffer_full_status_tx;

  sc_in<int> free_slots_neighbor;
  sc_out<int> free_slots;
  //-------------------------------------------------------------------------

  //-------------------------------------------------------------------------
  // Y channel
  sc_in<Flit> flit_ry; // The input channel
  sc_in<bool> req_ry;  // The request associated with the input channel
  sc_out<bool>
      ack_ry; // The outgoing ack signal associated with the input channel
  sc_out<TBufferFullStatus> buffer_full_status_ry;

  sc_out<Flit> flit_ty; // The output channel
  sc_out<bool> req_ty;  // The request associated with the output channel
  sc_in<bool>
      ack_ty; // The outgoing ack signal associated with the output channel
  sc_in<TBufferFullStatus> buffer_full_status_ty;

  sc_in<int> free_slots_neighbor_y;
  sc_out<int> free_slots_y;
  //-------------------------------------------------------------------------

  // Registers
  int local_id; // Unique identification number
  bool is_memory_pe;
  int local_direction_id;
  bool current_level_rx; // Current level for Alternating Bit Protocol (ABP)
  bool current_level_tx; // Current level for Alternating Bit Protocol (ABP)
  queue<Packet> packet_queue_x; // Local queue of packets for x channel
  queue<Flit> in_flit_queue_x[MAX_VIRTUAL_CHANNELS]; // Queue of incoming flits
                                                     // for x channel
  queue<Packet>
      in_packet_queue_x[MAX_VIRTUAL_CHANNELS]; // Queue of incoming packets
                                               // for x channel
  queue<Packet> packet_queue_y; // Local queue of packets for y channel
  queue<Flit> in_flit_queue_y[MAX_VIRTUAL_CHANNELS]; // Queue of incoming flits
                                                     // for y channel
  queue<Packet>
      in_packet_queue_y[MAX_VIRTUAL_CHANNELS]; // Queue of incoming packets
                                               // for y channel

  int cur_out_vc_x;
  queue<int> out_reservation_queue_x;
  bool out_reservation_status_x[MAX_VIRTUAL_CHANNELS];
  bool is_vc_set_x = 0;

  int cur_out_vc_y;
  queue<int> out_reservation_queue_y;
  bool out_reservation_status_y[MAX_VIRTUAL_CHANNELS];
  bool is_vc_set_y = 0;

  bool transmittedAtPreviousCycle; // Used for distributions with memory

  // Interlivin feature parameters
  int interliving_prev_dst;
  int interliving_prev_reps;
  int interliving_local_dst;

  // Traffic burst state
  unsigned long traffic_burst_curr_x = 0;
  unsigned long traffic_burst_curr_y = 0;
  int traffic_burst_curr_dst_x;
  int traffic_burst_curr_dst_y;

  // Metrics
  unsigned long flits_sent_x = 0;
  unsigned long flits_recv_x = 0;
  unsigned long flits_sent_y = 0;
  unsigned long flits_recv_y = 0;

  unsigned long packets_sent_x = 0;
  unsigned long packets_recv_x = 0;
  unsigned long packets_sent_y = 0;
  unsigned long packets_recv_y = 0;

  unsigned long traffic_burst_packets_sent_x = 0;
  unsigned long traffic_burst_packets_recv_x = 0;
  unsigned long traffic_burst_packets_sent_y = 0;
  unsigned long traffic_burst_packets_recv_y = 0;

  map<int, FlitLatencyInfo> flit_latency_x;
  map<int, FlitLatencyInfo> flit_latency_y;
  map<int, FlitLatencyInfo> traffic_burst_flit_latency_x;
  map<int, FlitLatencyInfo> traffic_burst_flit_latency_y;

  // Functions
  void rxProcess(); // The receiving process
  void txProcess(); // The transmitting process
  void ryProcess();
  void tyProcess();
  bool is_same_quadrant(int self_id, int id);
  bool is_angle_pe(int id);
  bool is_vertical_pe(int id);
  bool is_angle_special_pe(int id, int num);
  bool is_horizontal_special_pe(int id, int num);
  bool canShot(Packet & packet,
               RequestType); // True when the packet must be shot
  Flit nextFlit(queue<Packet> & packet_queue,
                bool);               // Take the next flit of the current packet
  Packet trafficTest();              // used for testing traffic
  Packet trafficRandom(RequestType); // Random destination distribution
  Packet trafficTranspose1();        // Transpose 1 destination distribution
  Packet trafficTranspose2();        // Transpose 2 destination distribution
  Packet trafficBitReversal();       // Bit-reversal destination distribution
  Packet trafficShuffle();           // Shuffle destination distribution
  Packet trafficButterfly();         // Butterfly destination distribution
  Packet trafficLocal();             // Random with locality
  Packet trafficULocal();            // Random with locality

  Packet generateResponse(Flit, RequestType);

  GlobalTrafficTable *traffic_table; // Reference to the Global traffic Table
  bool never_transmit; // true if the PE does not transmit any packet
  //  (valid only for the table based traffic)

  void fixRanges(const Coord, Coord &); // Fix the ranges of the destination
  int randInt(int min,
              int max); // Extracts a random integer number between min and max
  int getRandomSize();  // Returns a random size in flits for the packet
  void setBit(int &x, int w, int v);
  int getBit(int x, int w);
  double log2ceil(double x);

  int roulett();
  int findRandomDestination(int local_id, int hops);
  unsigned int getQueueSize() const;

  // Constructor
  SC_CTOR(ProcessingElement) {

    SC_METHOD(txProcess);
    sensitive << reset;
    sensitive << clock.pos();

    SC_METHOD(tyProcess);
    sensitive << reset;
    sensitive << clock.pos();

    SC_METHOD(rxProcess);
    sensitive << reset;
    sensitive << clock.pos();

    SC_METHOD(ryProcess);
    sensitive << reset;
    sensitive << clock.pos();
  }
};

#endif
