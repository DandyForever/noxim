/*
 * Noxim - the NoC Simulator
 *
 * (C) 2005-2018 by the University of Catania
 * For the complete list of authors refer to file ../doc/AUTHORS.txt
 * For the license applied to these sources refer to file ../doc/LICENSE.txt
 *
 * This file contains the implementation of the processing element
 */

#include "ProcessingElement.h"

#define DEBUG if (0)

int ProcessingElement::randInt(int min, int max) {
  return min + (int)((double)(max - min + 1) * rand() / (RAND_MAX + 1.0));
}

// AXI4-Stream protocol
// ack_rx <-> s_axis_tready
// req_tx <-> m_axis_tvalid
void ProcessingElement::rxProcess() {
  if (reset.read()) {
    ack_rx.write(1);
    return;
  }

  //---------------------------------------------------------------------
  // Start checks
  //---------------------------------------------------------------------
  DEBUG if (!GlobalParams::both_phys_req_mode) {
    if (!is_memory_pe) {
      assert(!req_rx.read());
    }
  }
  //!---------------------------------------------------------------------
  //! End checks
  //!---------------------------------------------------------------------

  //---------------------------------------------------------------------
  // Start collect stats
  //---------------------------------------------------------------------
  if (req_rx.read() && ack_rx.read()) {
    Flit flit = flit_rx.read();
    flits_recv_x++;
    if (flit.flit_type(FLIT_TYPE_TAIL)) {
      packets_recv_x++;
      if (flit.traffic_burst_is_tail) {
        traffic_burst_packets_recv_x++;
      }
    }

    if (!is_memory_pe && flit.flit_type(FLIT_TYPE_TAIL)) {
      int send_timestamp = flit_latency_y[flit.id].latency;
      int recv_timestamp = timestamp();
      int latency = recv_timestamp - send_timestamp;
      flit_latency_y[flit.id] = {1, flit.src_id, latency};
      if (flit.traffic_burst_is_tail) {
        int traffic_burst_send_timestamp =
            traffic_burst_flit_latency_y[flit.traffic_burst_id].latency;
        int traffic_burst_latency =
            recv_timestamp - traffic_burst_send_timestamp;
        traffic_burst_flit_latency_y[flit.traffic_burst_id] = {
            1, flit.src_id, traffic_burst_latency};
      }
    }
  }
  //!---------------------------------------------------------------------
  //! End collect stats
  //!---------------------------------------------------------------------

  //---------------------------------------------------------------------
  // Start dump info
  //---------------------------------------------------------------------
  if (req_rx.read() && ack_rx.read()) {
    if (GlobalParams::flit_dump) {
      cout << "X Recv Info: Cycle["
           << timestamp()
           << "] flit " << flit_rx.read().src_id << " -> "
           << flit_rx.read().dst_id << endl;
    }
  }
  //!---------------------------------------------------------------------
  //! End dump info
  //!---------------------------------------------------------------------

  //---------------------------------------------------------------------
  // Start managing flit recieved
  //---------------------------------------------------------------------
  // Only memory PEs are required to send response back
  if (GlobalParams::req_ack_mode && is_memory_pe) {
    // Slave AXIS valid & ready handshake
    if (req_rx.read() && ack_rx.read()) {
      if (flit_rx.read().flit_type(FLIT_TYPE_TAIL)) {
        Flit flit_tmp = flit_rx.read();
        int vc_id = GlobalParams::n_virtual_channels - 1 - flit_tmp.vc_id;
        Packet p = generateResponse(flit_tmp, RequestType::WRITE);
        in_packet_queue_x[vc_id].push(p);
      }
    }
  }
  //!---------------------------------------------------------------------
  //! End managing flit recieved
  //!---------------------------------------------------------------------

  //---------------------------------------------------------------------
  // Start managing ack_rx
  //---------------------------------------------------------------------
  if (is_memory_pe) {
    ack_rx.write(in_packet_queue_x[free_slots_neighbor].size() <
                 GlobalParams::buffer_depth);
  } else {
    // PE is always ready to recieve packets
    ack_rx.write(1);
  }
  //!---------------------------------------------------------------------
  //! End managing ack_rx
  //!---------------------------------------------------------------------
}

void ProcessingElement::txProcess() {
  if (reset.read()) {
    // Not valid on reset
    req_tx.write(0);
    transmittedAtPreviousCycle = false;
    return;
  }

  //---------------------------------------------------------------------
  // Start collect stats
  //---------------------------------------------------------------------
  if (req_tx.read() && ack_tx.read()) {
    Flit flit = flit_tx.read();

    if (!is_memory_pe && flit.flit_type(FLIT_TYPE_HEAD)) {
      int dst_id = flit.dst_id;
      int now = timestamp();
      flit_latency_x[flit.id] = {0, dst_id, now};
      if (flit.traffic_burst_is_head) {
        traffic_burst_flit_latency_x[flit.traffic_burst_id] = {0, dst_id,
                                                               now};
      }
    }

    flits_sent_x++;
    if (flit.flit_type(FLIT_TYPE_TAIL)) {
      packets_sent_x++;
      if (flit.traffic_burst_is_tail) {
        traffic_burst_packets_sent_x++;
      }
    }
  }
  //!---------------------------------------------------------------------
  //! End collect stats
  //!---------------------------------------------------------------------

  //---------------------------------------------------------------------
  // Start dump info
  //---------------------------------------------------------------------
  if (req_tx.read() && ack_tx.read()) {
    if (GlobalParams::flit_dump) {
      cout << "X Send Info: Cycle["
           << timestamp()
           << "] flit " << flit_tx.read().src_id << " -> "
           << flit_tx.read().dst_id << endl;
    }
  }
  //!---------------------------------------------------------------------
  //! End dump info
  //!---------------------------------------------------------------------

  //---------------------------------------------------------------------
  // Start managing memory PE
  //---------------------------------------------------------------------
  if (is_memory_pe) {
    if (req_tx.read() && ack_tx.read()) {
      DEBUG assert(!in_packet_queue_y[cur_out_vc_x].empty());
      nextFlit(in_packet_queue_y[cur_out_vc_x], true);
    }

    for (int vc = 0; vc < GlobalParams::n_virtual_channels; vc++) {
      if (!in_packet_queue_y[vc].empty() && !out_reservation_status_x[vc]) {
        out_reservation_queue_x.push(vc);
        out_reservation_status_x[vc] = true;
      }
    }

    if (!is_vc_set_x) {
      req_tx.write(0);
      if (!out_reservation_queue_x.empty()) {
        free_slots.write(out_reservation_queue_x.front());
        is_vc_set_x = true;
      }
    } else {
      req_tx.write(1);
      int vc = out_reservation_queue_x.front();
      cur_out_vc_x = vc;
      Flit flit = nextFlit(in_packet_queue_y[vc], false);
      flit_tx->write(flit);
      out_reservation_status_x[vc] = false;
      out_reservation_queue_x.pop();
      if (out_reservation_queue_x.empty()) {
        if (in_packet_queue_y[vc].size() > 1) {
          free_slots.write(vc);
          out_reservation_queue_x.push(vc);
          out_reservation_status_x[vc] = true;
        } else {
          is_vc_set_x = false;
        }
      } else {
        free_slots.write(out_reservation_queue_x.front());
      }
    }
  }
  //!---------------------------------------------------------------------
  //! End managing memory PE
  //!---------------------------------------------------------------------

  //---------------------------------------------------------------------
  // Start managing EU PE
  //---------------------------------------------------------------------
  if (!is_memory_pe) {
    // Generate packet and add to packet queue
    Packet packet;
    if ((packet_queue_x.size() < 2) && canShot(packet, RequestType::WRITE)) {
      packet_queue_x.push(packet);
      transmittedAtPreviousCycle = true;
    } else {
      transmittedAtPreviousCycle = false;
    }
    // Tell the router VC to recieve flits
    if (GlobalParams::routing_algorithm == "MOD_DOR") {
      free_slots.write(is_vertical_pe(local_id));
    } else {
      free_slots.write(0);
    }
    // Handle AXIS
    if ((req_tx.read() && ack_tx.read()) || // Handshake happened
        !req_tx.read()                      // Not started sending
    ) {
      int on_the_fly_x = packets_sent_x - packets_recv_y;
      if (!packet_queue_x.empty() && (!GlobalParams::req_ack_mode ||
          on_the_fly_x < GlobalParams::pe_request_buffer_size /
                             GlobalParams::max_packet_size)) {
        Flit flit = nextFlit(packet_queue_x, true);
        flit.id = packets_sent_x;
        flit.traffic_burst_id = traffic_burst_packets_sent_x;
        flit_tx->write(flit);
        req_tx.write(1);
      } else {
        req_tx.write(0);
      }
    } else if (req_tx.read() && !ack_tx.read()) {
      req_tx.write(1);
    }
  }
  //!---------------------------------------------------------------------
  //! End managing EU PE
  //!---------------------------------------------------------------------

  //---------------------------------------------------------------------
  // Start checks
  //---------------------------------------------------------------------
  DEBUG if (!GlobalParams::both_phys_req_mode) {
    if (is_memory_pe) {
      assert(!req_tx.read());
    }
  }
  DEBUG if (!GlobalParams::req_ack_mode) {
    if (is_memory_pe) {
      assert(!req_tx.read());
    }
  }
  DEBUG if (!GlobalParams::req_ack_mode) {
    for (int vc = 0; vc < GlobalParams::n_virtual_channels; vc++) {
      assert(in_packet_queue_y[vc].empty());
    }
  }
  //!---------------------------------------------------------------------
  //! End checks
  //!---------------------------------------------------------------------
}

void ProcessingElement::ryProcess() {
  if (reset.read()) {
    ack_ry.write(1);
    return;
  }

  //---------------------------------------------------------------------
  // Start checks
  //---------------------------------------------------------------------
  DEBUG if (!GlobalParams::both_phys_req_mode) {
    if (is_memory_pe) {
      assert(!req_ry.read());
    }
  }
  //!---------------------------------------------------------------------
  //! End checks
  //!---------------------------------------------------------------------

  //---------------------------------------------------------------------
  // Start collect stats
  //---------------------------------------------------------------------
  if (req_ry.read() && ack_ry.read()) {
    Flit flit = flit_ry.read();
    flits_recv_y++;
    if (flit.flit_type(FLIT_TYPE_TAIL)) {
      packets_recv_y++;
      if (flit.traffic_burst_is_head) {
        traffic_burst_packets_recv_y++;
      }
    }

    if (!is_memory_pe && flit.flit_type(FLIT_TYPE_TAIL)) {
      int send_timestamp = flit_latency_x[flit.id].latency;
      int recv_timestamp = timestamp();
      int latency = recv_timestamp - send_timestamp;
      flit_latency_x[flit.id] = {1, flit.src_id, latency};
      if (flit.traffic_burst_is_tail) {
        int traffic_burst_send_timestamp =
            traffic_burst_flit_latency_x[flit.traffic_burst_id].latency;
        int traffic_burst_latency =
            recv_timestamp - traffic_burst_send_timestamp;
        traffic_burst_flit_latency_x[flit.traffic_burst_id] = {
            1, flit.src_id, traffic_burst_latency};
      }
    }
  }
  //!---------------------------------------------------------------------
  //! End collect stats
  //!---------------------------------------------------------------------

  //---------------------------------------------------------------------
  // Start dump info
  //---------------------------------------------------------------------
  if (req_ry.read() && ack_ry.read()) {
    if (GlobalParams::flit_dump) {
      cout << "Y Recv Info: Cycle["
           << timestamp()
           << "] flit " << flit_ry.read().src_id << " -> "
           << flit_ry.read().dst_id << endl;
    }
  }
  //!---------------------------------------------------------------------
  //! End dump info
  //!---------------------------------------------------------------------

  //---------------------------------------------------------------------
  // Start managing flit recieved
  //---------------------------------------------------------------------
  // Only memory PEs are required to send response back
  if (GlobalParams::both_phys_req_mode && GlobalParams::req_ack_mode &&
      is_memory_pe) {
    // Slave AXIS valid & ready handshake
    if (req_ry.read() && ack_ry.read()) {
      if (flit_ry.read().flit_type(FLIT_TYPE_TAIL)) {
        Flit flit_tmp = flit_ry.read();
        int vc_id = GlobalParams::n_virtual_channels - 1 - flit_tmp.vc_id;
        Packet p = generateResponse(flit_ry.read(), RequestType::READ);
        in_packet_queue_y[vc_id].push(p);
      }
    }
  }
  //!---------------------------------------------------------------------
  //! End managing flit recieved
  //!---------------------------------------------------------------------

  //---------------------------------------------------------------------
  // Start managing ack_ry
  //---------------------------------------------------------------------
  if (is_memory_pe) {
    ack_ry.write(in_packet_queue_y[free_slots_neighbor_y].size() <
                 GlobalParams::buffer_depth);
  } else {
    // PE is always ready to recieve packets
    ack_ry.write(1);
  }
  //---------------------------------------------------------------------
  // End managing ack_ry
  //---------------------------------------------------------------------
}

void ProcessingElement::tyProcess() {
  if (reset.read()) {
    req_ty.write(0);
    return;
  }

  //---------------------------------------------------------------------
  // Start collect stats
  //---------------------------------------------------------------------
  if (req_ty.read() && ack_ty.read()) {
    Flit flit = flit_ty.read();

    if (!is_memory_pe && flit.flit_type(FLIT_TYPE_HEAD)) {
      int now = timestamp();
      int dst_id = flit.dst_id;
      flit_latency_y[flit.id] = {0, dst_id, now};
      if (flit.traffic_burst_is_head) {
        traffic_burst_flit_latency_y[flit.traffic_burst_id] = {0, dst_id,
                                                               now};
      }
    }

    flits_sent_y++;
    if (flit.flit_type(FLIT_TYPE_TAIL)) {
      packets_sent_y++;
      if (flit.traffic_burst_is_tail) {
        traffic_burst_packets_sent_y++;
      }
    }
  }
  //!---------------------------------------------------------------------
  //! End collect stats
  //!---------------------------------------------------------------------

  //---------------------------------------------------------------------
  // Start dump info
  //---------------------------------------------------------------------
  if (req_ty.read() && ack_ty.read()) {
    if (GlobalParams::flit_dump) {
      cout << "Y Send Info: Cycle["
           << timestamp()
           << "] flit " << flit_ty.read().src_id << " -> "
           << flit_ty.read().dst_id << endl;
    }
  }
  //!---------------------------------------------------------------------
  //! End dump info
  //!---------------------------------------------------------------------

  //---------------------------------------------------------------------
  // Start managing memory PE
  //---------------------------------------------------------------------
  if (is_memory_pe) {
    if (req_ty.read() && ack_ty.read()) {
      DEBUG assert(!in_packet_queue_x[cur_out_vc_y].empty());
      nextFlit(in_packet_queue_x[cur_out_vc_y], true);
    }

    for (int vc = 0; vc < GlobalParams::n_virtual_channels; vc++) {
      if (!in_packet_queue_x[vc].empty() && !out_reservation_status_y[vc]) {
        out_reservation_queue_y.push(vc);
        out_reservation_status_y[vc] = true;
      }
    }

    if (!is_vc_set_y) {
      req_ty.write(0);
      if (!out_reservation_queue_y.empty()) {
        free_slots_y.write(out_reservation_queue_y.front());
        is_vc_set_y = true;
      }
    } else {
      req_ty.write(1);
      int vc = out_reservation_queue_y.front();
      cur_out_vc_y = vc;
      Flit flit = nextFlit(in_packet_queue_x[vc], false);
      flit_ty->write(flit);
      out_reservation_status_y[vc] = false;
      out_reservation_queue_y.pop();
      if (out_reservation_queue_y.empty()) {
        if (in_packet_queue_x[vc].size() > 1) {
          free_slots_y.write(vc);
          out_reservation_queue_y.push(vc);
          out_reservation_status_y[vc] = true;
        } else {
          is_vc_set_y = false;
        }
      } else {
        free_slots_y.write(out_reservation_queue_y.front());
      }
    }
  }
  //!---------------------------------------------------------------------
  //! End managing memory PE
  //!---------------------------------------------------------------------

  //---------------------------------------------------------------------
  // Start managing EU PE
  //---------------------------------------------------------------------
  if (GlobalParams::both_phys_req_mode && !is_memory_pe) {
    // Generate packet and add to packet queue
    Packet packet;
    if ((packet_queue_y.size() < 2) && canShot(packet, RequestType::READ)) {
      packet_queue_y.push(packet);
      transmittedAtPreviousCycle = true;
    } else {
      transmittedAtPreviousCycle = false;
    }
    // Tell the router VC to recieve flits
    if (GlobalParams::routing_algorithm == "MOD_DOR") {
      free_slots_y.write(is_vertical_pe(local_id));
    } else {
      free_slots_y.write(0);
    }
    // Handle AXIS
    if ((req_ty.read() && ack_ty.read()) || // Handshake happened
        !req_ty.read()                      // Not started sending
    ) {
      int on_the_fly_y = packets_sent_y - packets_recv_x;
      if (!packet_queue_y.empty() && (!GlobalParams::req_ack_mode ||
          on_the_fly_y < GlobalParams::pe_request_buffer_size /
                             GlobalParams::max_packet_size)) {
        Flit flit = nextFlit(packet_queue_y, true);
        flit.id = packets_sent_y;
        flit.traffic_burst_id = traffic_burst_packets_sent_y;
        flit_ty->write(flit);
        req_ty.write(1);
      } else { // Packet queue is empty
        req_ty.write(0);
      }
    } else if (req_ty.read() && !ack_ty.read()) // Router not ready to recieve
    {
      req_ty.write(1);
    }
  }
  //!---------------------------------------------------------------------
  //! End managing EU PE
  //!---------------------------------------------------------------------

  //---------------------------------------------------------------------
  // Start checks
  //---------------------------------------------------------------------
  DEBUG if (!GlobalParams::both_phys_req_mode) {
    if (!is_memory_pe) {
      assert(!req_ty.read());
    }
  }
  DEBUG if (!GlobalParams::req_ack_mode) {
    if (is_memory_pe) {
      assert(!req_ty.read());
    }
  }
  DEBUG if (!GlobalParams::req_ack_mode) {
    for (int vc = 0; vc < GlobalParams::n_virtual_channels; vc++) {
      assert(in_packet_queue_x[vc].empty());
    }
  }
  //!---------------------------------------------------------------------
  //! End checks
  //!---------------------------------------------------------------------
}

Flit ProcessingElement::nextFlit(queue<Packet> &packet_queue, bool is_update) {
  Flit flit;
  Packet packet = packet_queue.front();

  flit.src_id = packet.src_id;
  flit.dst_id = packet.dst_id;
  flit.id = packet.id;
  flit.local_direction_id = packet.local_direction_id;
  flit.phys_channel_id = packet.phys_channel_id;
  flit.vc_id = packet.vc_id;
  flit.timestamp = packet.timestamp;
  flit.sequence_no = packet.size - packet.flit_left;
  flit.sequence_length = packet.size;
  flit.hop_no = 0;

  flit.hub_relay_node = NOT_VALID;

  if (packet.size == packet.flit_left)
    flit.is_head = true;
  if (packet.flit_left == 1)
    flit.is_tail = true;

  flit.traffic_burst_is_head = packet.is_head;
  flit.traffic_burst_is_tail = packet.is_tail;
  flit.traffic_burst_id = packet.traffic_burst_id;

  if (!is_update)
    return flit;

  packet_queue.front().flit_left--;
  if (packet_queue.front().flit_left == 0)
    packet_queue.pop();

  return flit;
}

bool ProcessingElement::is_same_quadrant(int self_id, int id) {
  Coord self_coord = id2Coord(self_id);
  Coord coord = id2Coord(id);

  bool is_self_left = 2 * self_coord.x < GlobalParams::mesh_dim_x;
  bool is_self_top = 2 * self_coord.y < GlobalParams::mesh_dim_y;

  bool is_left = 2 * coord.x < GlobalParams::mesh_dim_x;
  bool is_top = 2 * coord.y < GlobalParams::mesh_dim_y;

  if (is_self_left && is_self_top && is_left && is_top)
    return true;

  if (!is_self_left && is_self_top && !is_left && is_top)
    return true;

  if (is_self_left && !is_self_top && is_left && !is_top)
    return true;

  if (!is_self_left && !is_self_top && !is_left && !is_top)
    return true;

  return false;
}

bool ProcessingElement::is_angle_pe(int id) {
  Coord coord = id2Coord(id);

  if (coord.x == 0 && coord.y == 0)
    return true;
  if (coord.x == 0 && coord.y == GlobalParams::mesh_dim_y - 1)
    return true;
  if (coord.x == GlobalParams::mesh_dim_x - 1 && coord.y == 0)
    return true;
  if (coord.x == GlobalParams::mesh_dim_x - 1 &&
      coord.y == GlobalParams::mesh_dim_y - 1)
    return true;

  return false;
}

bool ProcessingElement::is_angle_special_pe(int id, int num) {
  DEBUG assert(GlobalParams::mesh_dim_y >= 2 * num);

  Coord coord = id2Coord(id);

  if (coord.x == 0 || coord.x == GlobalParams::mesh_dim_x - 1)
    return (coord.y < num) || (coord.y + num > GlobalParams::mesh_dim_y - 1);

  return false;
}

bool ProcessingElement::is_horizontal_special_pe(int id, int num) {
  DEBUG assert(GlobalParams::mesh_dim_x >= 2 * num);

  Coord coord = id2Coord(id);

  if (coord.y == 0 || coord.y == GlobalParams::mesh_dim_y - 1)
    return (coord.x < num) || (coord.x + num > GlobalParams::mesh_dim_x - 1);

  return false;
}

bool ProcessingElement::is_vertical_pe(int id) {
  Coord coord = id2Coord(id);

  return (coord.x == 0) || (coord.x == GlobalParams::mesh_dim_x - 1);
}

bool ProcessingElement::canShot(Packet &packet, RequestType request_type) {
  if (never_transmit)
    return false;

  // Central tiles should not send packets
  if (is_memory_pe)
    return false;

  // Switching off some PEs
  //-----------------------------------------------------
  if (GlobalParams::switch_angle_masters &&
      is_angle_special_pe(local_id, GlobalParams::switch_angle_masters))
    return false;
  if (GlobalParams::switch_horizontal_masters &&
      is_horizontal_special_pe(local_id,
                               GlobalParams::switch_horizontal_masters))
    return false;
  if (GlobalParams::switch_vertical_masters && is_vertical_pe(local_id))
    return false;
  //-----------------------------------------------------

  // Switching off some local directions
  //-----------------------------------------------------
  if (GlobalParams::eu_ports < 2 && local_direction_id == DIRECTION_LOCAL_WEST)
    return false;
  if (GlobalParams::eu_ports < 3 && local_direction_id == DIRECTION_LOCAL_SOUTH)
    return false;
  if (GlobalParams::eu_ports < 4 && local_direction_id == DIRECTION_LOCAL_EAST)
    return false;
  //-----------------------------------------------------

  //-----------------------------------------------------
  // For debug only
  //-----------------------------------------------------
  if (GlobalParams::switch_debug &&
      local_id != 0
      // && local_id != 1
      && local_id != GlobalParams::mesh_dim_x &&
      local_id != 2 * GlobalParams::mesh_dim_x &&
      local_id + 1 != 2 * GlobalParams::mesh_dim_x)
    return false;
  //-----------------------------------------------------

  //-----------------------------------------------------
  // For validation only
  //-----------------------------------------------------
  if (GlobalParams::switch_interliving_validation &&
      (local_id < GlobalParams::mesh_dim_x ||
       local_id >= GlobalParams::mesh_dim_x * (GlobalParams::mesh_dim_y - 1)))
    return false;
    //-----------------------------------------------------

    //-----------------------------------------------------
    // Temporary for thinning
    //-----------------------------------------------------
    // if (
    //     is_vertical_pe(local_id) &&
    //     (local_id != 3 * GlobalParams::mesh_dim_x) && (local_id != 5 *
    //     GlobalParams::mesh_dim_x - 1) && (local_id != 5 *
    //     GlobalParams::mesh_dim_x) && (local_id != 7 *
    //     GlobalParams::mesh_dim_x - 1) && (local_id != 7 *
    //     GlobalParams::mesh_dim_x) && (local_id != 9 *
    //     GlobalParams::mesh_dim_x - 1) && (local_id != 9 *
    //     GlobalParams::mesh_dim_x) && (local_id != 11 *
    //     GlobalParams::mesh_dim_x - 1)
    // ) return false;
    //-----------------------------------------------------

#ifdef DEADLOCK_AVOIDANCE
  if (local_id % 2 == 0)
    return false;
#endif
  bool shot;
  double threshold;

  double now = timestamp();

  if (GlobalParams::traffic_distribution != TRAFFIC_TABLE_BASED) {
    switch (request_type) {
    case READ:
      if (traffic_burst_curr_y == 0) {
        threshold = GlobalParams::packet_injection_rate /
                    GlobalParams::traffic_burst_size;
      } else {
        threshold = 1.;
      }
      break;
    case WRITE:
      if (traffic_burst_curr_x == 0) {
        threshold = GlobalParams::packet_injection_rate /
                    GlobalParams::traffic_burst_size;
      } else {
        threshold = 1.;
      }
      break;
    default:
      threshold = GlobalParams::packet_injection_rate;
      break;
    }

    shot = (((double)rand()) / RAND_MAX < threshold);
    if (shot) {
      if (GlobalParams::traffic_distribution == TRAFFIC_RANDOM)
        packet = trafficRandom(request_type);
      else if (GlobalParams::traffic_distribution == TRAFFIC_TRANSPOSE1)
        packet = trafficTranspose1();
      else if (GlobalParams::traffic_distribution == TRAFFIC_TRANSPOSE2)
        packet = trafficTranspose2();
      else if (GlobalParams::traffic_distribution == TRAFFIC_BIT_REVERSAL)
        packet = trafficBitReversal();
      else if (GlobalParams::traffic_distribution == TRAFFIC_SHUFFLE)
        packet = trafficShuffle();
      else if (GlobalParams::traffic_distribution == TRAFFIC_BUTTERFLY)
        packet = trafficButterfly();
      else if (GlobalParams::traffic_distribution == TRAFFIC_LOCAL)
        packet = trafficLocal();
      else if (GlobalParams::traffic_distribution == TRAFFIC_ULOCAL)
        packet = trafficULocal();
      else {
        cout << "Invalid traffic distribution: "
             << GlobalParams::traffic_distribution << endl;
        exit(-1);
      }
    }
  } else { // Table based communication traffic
    if (never_transmit)
      return false;

    bool use_pir = (transmittedAtPreviousCycle == false);
    vector<pair<int, double>> dst_prob;
    double threshold = traffic_table->getCumulativePirPor(local_id, (int)now,
                                                          use_pir, dst_prob);

    double prob = (double)rand() / RAND_MAX;
    shot = (prob < threshold);
    if (shot) {
      for (unsigned int i = 0; i < dst_prob.size(); i++) {
        if (prob < dst_prob[i].second) {
          int vc = randInt(0, GlobalParams::n_virtual_channels - 1);
          packet.make(local_id, dst_prob[i].first, vc, now, getRandomSize(),
                      local_direction_id, 0, 0, true, true, 0);
          break;
        }
      }
    }
  }

  return shot;
}

Packet ProcessingElement::trafficLocal() {
  Packet p;
  p.src_id = local_id;
  double rnd = rand() / (double)RAND_MAX;

  vector<int> dst_set;

  int max_id = (GlobalParams::mesh_dim_x * GlobalParams::mesh_dim_y);

  for (int i = 0; i < max_id; i++) {
    if (rnd <= GlobalParams::locality) {
      if (local_id != i && sameRadioHub(local_id, i))
        dst_set.push_back(i);
    } else {
      if (!sameRadioHub(local_id, i)) {
        dst_set.push_back(i);
      }
    }
  }

  int i_rnd = rand() % dst_set.size();

  p.dst_id = dst_set[i_rnd];
  p.timestamp = timestamp();
  p.size = p.flit_left = getRandomSize();
  p.vc_id = randInt(0, GlobalParams::n_virtual_channels - 1);

  return p;
}

int ProcessingElement::findRandomDestination(int id, int hops) {
  assert(GlobalParams::topology == TOPOLOGY_MESH);

  int inc_y = rand() % 2 ? -1 : 1;
  int inc_x = rand() % 2 ? -1 : 1;

  Coord current = id2Coord(id);

  for (int h = 0; h < hops; h++) {

    if (current.x == 0)
      if (inc_x < 0)
        inc_x = 0;

    if (current.x == GlobalParams::mesh_dim_x - 1)
      if (inc_x > 0)
        inc_x = 0;

    if (current.y == 0)
      if (inc_y < 0)
        inc_y = 0;

    if (current.y == GlobalParams::mesh_dim_y - 1)
      if (inc_y > 0)
        inc_y = 0;

    if (rand() % 2)
      current.x += inc_x;
    else
      current.y += inc_y;
  }
  return coord2Id(current);
}

int roulette() {
  int slices = GlobalParams::mesh_dim_x + GlobalParams::mesh_dim_y - 2;

  double r = rand() / (double)RAND_MAX;

  for (int i = 1; i <= slices; i++) {
    if (r < (1 - 1 / double(2 << i))) {
      return i;
    }
  }
  assert(false);
  return 1;
}

Packet ProcessingElement::trafficULocal() {
  Packet p;
  p.src_id = local_id;

  int target_hops = roulette();

  p.dst_id = findRandomDestination(local_id, target_hops);

  p.timestamp = timestamp();
  p.size = p.flit_left = getRandomSize();
  p.vc_id = randInt(0, GlobalParams::n_virtual_channels - 1);

  return p;
}

Packet ProcessingElement::generateResponse(Flit flit,
                                           RequestType request_type) {
  Packet p;
  p.src_id = flit.dst_id;
  p.dst_id = flit.src_id;
  p.id = flit.id;
  p.vc_id = GlobalParams::n_virtual_channels - 1 - flit.vc_id;
  p.is_head = flit.traffic_burst_is_head;
  p.is_tail = flit.traffic_burst_is_tail;
  p.traffic_burst_id = flit.traffic_burst_id;
  p.local_direction_id = DIRECTION_LOCAL_NORTH;
  p.phys_channel_id = 1 - flit.phys_channel_id;
  p.timestamp = timestamp();
  switch (request_type) {
  case RequestType::READ:
    p.size = p.flit_left = GlobalParams::max_packet_size;
    break;
  case RequestType::WRITE:
    p.size = p.flit_left = 1;
    break;
  default:
    p.size = p.flit_left = 1;
    break;
  }
  return p;
}

Packet ProcessingElement::trafficRandom(RequestType request_type) {
  Packet p;
  p.src_id = local_id;
  p.phys_channel_id = 0;
  p.id = 0;
  p.traffic_burst_id = 0;
  p.local_direction_id =
      randInt(DIRECTION_LOCAL_NORTH,
              DIRECTION_LOCAL_NORTH - 1 + GlobalParams::mem_ports);
  double rnd = rand() / (double)RAND_MAX;
  double range_start = 0.0;
  int max_id;

  if (GlobalParams::topology == TOPOLOGY_MESH)
    max_id = (GlobalParams::mesh_dim_x * GlobalParams::mesh_dim_y) - 1; // Mesh
  else // other delta topologies
    max_id = GlobalParams::n_delta_tiles - 1;

  // Random destination distribution
  do {
    p.dst_id = randInt(0, max_id);

    // check for hotspot destination
    for (size_t i = 0; i < GlobalParams::hotspots.size(); i++) {

      if (rnd >= range_start &&
          rnd < range_start + GlobalParams::hotspots[i].second) {
        if (local_id != GlobalParams::hotspots[i].first) {
          p.dst_id = GlobalParams::hotspots[i].first;
        }
        break;
      } else
        range_start += GlobalParams::hotspots[i].second; // try next
    }
#ifdef DEADLOCK_AVOIDANCE
    assert((GlobalParams::topology == TOPOLOGY_MESH));
    if (p.dst_id % 2 != 0) {
      p.dst_id = (p.dst_id + 1) % 256;
    }
#endif

  } while (!is_memory_node(p.dst_id)
           //------------------------------
           // Temporary for quadrant study
           //------------------------------
           // || !is_same_quadrant(local_id, p.dst_id)
           //------------------------------
           // Temporary for empty columns
           //------------------------------
           // || ((p.dst_id - 1) % GlobalParams::mesh_dim_x == 0)
           // || ((p.dst_id + 2) % GlobalParams::mesh_dim_x == 0)
           // || ((p.dst_id - 2) % GlobalParams::mesh_dim_x == 0)
           // || ((p.dst_id + 3) % GlobalParams::mesh_dim_x == 0)
           //------------------------------
  );

  //-----------------------------------
  // Interliving feature traffic
  //-----------------------------------
  if (GlobalParams::interliving_reps) {
    if (is_vertical_pe(local_id) && !is_angle_pe(local_id)) {
      if (local_id % GlobalParams::mesh_dim_x == 0) { // left side tile PE
        if (interliving_prev_reps == GlobalParams::interliving_reps) {
          interliving_prev_reps = 0;
          if (GlobalParams::interliving_direction) {
            interliving_prev_dst--;
            if (interliving_prev_dst == local_id) {
              interliving_prev_dst = local_id + GlobalParams::mesh_dim_x - 2;
              interliving_local_dst =
                  (interliving_local_dst + 1) % GlobalParams::mem_ports;
            }
          } else {
            interliving_prev_dst++;
            if (interliving_prev_dst + 1 ==
                GlobalParams::mesh_dim_x + local_id) {
              interliving_prev_dst = local_id + 1;
              interliving_local_dst =
                  (interliving_local_dst + 1) % GlobalParams::mem_ports;
            }
          }
        }
        p.dst_id = interliving_prev_dst;
        p.local_direction_id = DIRECTION_LOCAL_NORTH + interliving_local_dst;
        interliving_prev_reps++;
      } else { // right side tile PE
        if (interliving_prev_reps == GlobalParams::interliving_reps) {
          interliving_prev_reps = 0;
          if (GlobalParams::interliving_direction) {
            interliving_prev_dst++;
            if (interliving_prev_dst == local_id) {
              interliving_prev_dst = local_id - GlobalParams::mesh_dim_x + 2;
              interliving_local_dst =
                  (interliving_local_dst + 1) % GlobalParams::mem_ports;
            }
          } else {
            interliving_prev_dst--;
            if (interliving_prev_dst ==
                local_id + 1 - GlobalParams::mesh_dim_x) {
              interliving_prev_dst = local_id - 1;
              interliving_local_dst =
                  (interliving_local_dst + 1) % GlobalParams::mem_ports;
            }
          }
        }
        p.dst_id = interliving_prev_dst;
        p.local_direction_id = DIRECTION_LOCAL_NORTH + interliving_local_dst;
        interliving_prev_reps++;
      }
    }
  }
  //-----------------------------------

  p.timestamp = timestamp();
  p.is_head = false;
  p.is_tail = false;
  switch (request_type) {
  case RequestType::READ:
    p.size = p.flit_left = 1;
    if (traffic_burst_curr_y == 0) {
      p.is_head = true;
      traffic_burst_curr_dst_y = p.dst_id;
    } else {
      p.dst_id = traffic_burst_curr_dst_y;
    }
    traffic_burst_curr_y++;
    if (traffic_burst_curr_y * GlobalParams::max_packet_size ==
        GlobalParams::traffic_burst_size) {
      p.is_tail = true;
      traffic_burst_curr_y = 0;
    }
    break;
  case RequestType::WRITE:
    p.size = p.flit_left = GlobalParams::max_packet_size;
    if (traffic_burst_curr_x == 0) {
      p.is_head = true;
      traffic_burst_curr_dst_x = p.dst_id;
    } else {
      p.dst_id = traffic_burst_curr_dst_x;
    }
    traffic_burst_curr_x++;
    if (traffic_burst_curr_x * GlobalParams::max_packet_size ==
        GlobalParams::traffic_burst_size) {
      p.is_tail = true;
      traffic_burst_curr_x = 0;
    }
    break;
  default:
    p.size = p.flit_left = 1;
    break;
  }
  if (GlobalParams::routing_algorithm == "MOD_DOR") {
    p.vc_id = (int)is_vertical_pe(local_id);
  } else {
    p.vc_id = 0;
  }

  if (GlobalParams::switch_debug)
    cout << "For " << local_id << " dst " << p.dst_id << " ldid "
         << p.local_direction_id << endl;
  return p;
}

// TODO: for testing only
Packet ProcessingElement::trafficTest() {
  Packet p;
  p.src_id = local_id;
  p.dst_id = 10;

  p.timestamp = timestamp();
  p.size = p.flit_left = getRandomSize();
  p.vc_id = randInt(0, GlobalParams::n_virtual_channels - 1);

  return p;
}

Packet ProcessingElement::trafficTranspose1() {
  assert(GlobalParams::topology == TOPOLOGY_MESH);
  Packet p;
  p.src_id = local_id;
  Coord src, dst;

  // Transpose 1 destination distribution
  src.x = id2Coord(p.src_id).x;
  src.y = id2Coord(p.src_id).y;
  dst.x = GlobalParams::mesh_dim_x - 1 - src.y;
  dst.y = GlobalParams::mesh_dim_y - 1 - src.x;
  fixRanges(src, dst);
  p.dst_id = coord2Id(dst);

  p.vc_id = randInt(0, GlobalParams::n_virtual_channels - 1);
  p.timestamp = timestamp();
  p.size = p.flit_left = getRandomSize();

  return p;
}

Packet ProcessingElement::trafficTranspose2() {
  assert(GlobalParams::topology == TOPOLOGY_MESH);
  Packet p;
  p.src_id = local_id;
  Coord src, dst;

  // Transpose 2 destination distribution
  src.x = id2Coord(p.src_id).x;
  src.y = id2Coord(p.src_id).y;
  dst.x = src.y;
  dst.y = src.x;
  fixRanges(src, dst);
  p.dst_id = coord2Id(dst);

  p.vc_id = randInt(0, GlobalParams::n_virtual_channels - 1);
  p.timestamp = timestamp();
  p.size = p.flit_left = getRandomSize();

  return p;
}

void ProcessingElement::setBit(int &x, int w, int v) {
  int mask = 1 << w;

  if (v == 1)
    x = x | mask;
  else if (v == 0)
    x = x & ~mask;
  else
    assert(false);
}

int ProcessingElement::getBit(int x, int w) { return (x >> w) & 1; }

inline double ProcessingElement::log2ceil(double x) {
  return ceil(log(x) / log(2.0));
}

Packet ProcessingElement::trafficBitReversal() {

  int nbits = (int)log2ceil(
      (double)(GlobalParams::mesh_dim_x * GlobalParams::mesh_dim_y));
  int dnode = 0;
  for (int i = 0; i < nbits; i++)
    setBit(dnode, i, getBit(local_id, nbits - i - 1));

  Packet p;
  p.src_id = local_id;
  p.dst_id = dnode;

  p.vc_id = randInt(0, GlobalParams::n_virtual_channels - 1);
  p.timestamp = timestamp();
  p.size = p.flit_left = getRandomSize();

  return p;
}

Packet ProcessingElement::trafficShuffle() {

  int nbits = (int)log2ceil(
      (double)(GlobalParams::mesh_dim_x * GlobalParams::mesh_dim_y));
  int dnode = 0;
  for (int i = 0; i < nbits - 1; i++)
    setBit(dnode, i + 1, getBit(local_id, i));
  setBit(dnode, 0, getBit(local_id, nbits - 1));

  Packet p;
  p.src_id = local_id;
  p.dst_id = dnode;

  p.vc_id = randInt(0, GlobalParams::n_virtual_channels - 1);
  p.timestamp = timestamp();
  p.size = p.flit_left = getRandomSize();

  return p;
}

Packet ProcessingElement::trafficButterfly() {

  int nbits = (int)log2ceil(
      (double)(GlobalParams::mesh_dim_x * GlobalParams::mesh_dim_y));
  int dnode = 0;
  for (int i = 1; i < nbits - 1; i++)
    setBit(dnode, i, getBit(local_id, i));
  setBit(dnode, 0, getBit(local_id, nbits - 1));
  setBit(dnode, nbits - 1, getBit(local_id, 0));

  Packet p;
  p.src_id = local_id;
  p.dst_id = dnode;

  p.vc_id = randInt(0, GlobalParams::n_virtual_channels - 1);
  p.timestamp = timestamp();
  p.size = p.flit_left = getRandomSize();

  return p;
}

void ProcessingElement::fixRanges(const Coord src, Coord &dst) {
  // Fix ranges
  if (dst.x < 0)
    dst.x = 0;
  if (dst.y < 0)
    dst.y = 0;
  if (dst.x >= GlobalParams::mesh_dim_x)
    dst.x = GlobalParams::mesh_dim_x - 1;
  if (dst.y >= GlobalParams::mesh_dim_y)
    dst.y = GlobalParams::mesh_dim_y - 1;
}

int ProcessingElement::getRandomSize() {
  return randInt(GlobalParams::min_packet_size, GlobalParams::max_packet_size);
}

unsigned int ProcessingElement::getQueueSize() const {
  return packet_queue_x.size();
}
