/*
 * Noxim - the NoC Simulator
 *
 * (C) 2005-2018 by the University of Catania
 * For the complete list of authors refer to file ../doc/AUTHORS.txt
 * For the license applied to these sources refer to file ../doc/LICENSE.txt
 *
 * This file contains the implementation of the router
 */

#include "Router.h"

inline int toggleKthBit(int n, int k) { return (n ^ (1 << (k - 1))); }

void Router::process() {
  rxProcess();
  txProcess();
}

// AXI4-Stream Protocol
// ack_rx <-> s_axis_tready
// req_tx <-> m_axis_tvalid
void Router::rxProcess() {
  if (reset.read()) {
    TBufferFullStatus bfs;
    // Clear outputs and indexes of receiving protocol
    for (int i = 0; i < 2 * DIRECTIONS + 1; i++) {
      // Ready to recieve packets on reset
      ack_rx[i].write(1);
      buffer_full_status_rx[i].write(bfs);
    }
    routed_flits = 0;
    local_drained = 0;
  } else {
    for (int i = 0; i < 2 * DIRECTIONS + 1; i++) {
      if (req_rx[i].read() && ack_rx[i].read()) {
        Flit received_flit = flit_rx[i].read();

        if (received_flit.phys_channel_id == 1) {
          assert(is_memory_pe(received_flit.src_id));
        }

        int vc = received_flit.vc_id;

        if (!buffer[i][vc].IsFull()) {

          // Store the incoming flit in the circular buffer
          buffer[i][vc].Push(received_flit);
          LOG << " Flit " << received_flit << " collected from Input[" << i
              << "][" << vc << "]" << endl;

          power.bufferRouterPush();

          // if a new flit is injected from local PE
          if (received_flit.src_id == local_id)
            power.networkInterface();
        } else // buffer is full
        {
          // Should not happen in accordance with AXIS
          LOG << " Flit " << received_flit << " buffer full Input[" << i << "]["
              << vc << "]" << endl;
          cout << "FAILED I'm router " << local_id << " neighbour " << i << " "
               << free_slots_neighbor[i].read() << " " << received_flit << endl;
          assert(0);
        }
      }
      // Ready to recieve if buffer is not full
      ack_rx[i].write(!buffer[i][free_slots_neighbor[i].read()].IsFull());

      TBufferFullStatus bfs;
      for (int vc = 0; vc < GlobalParams::n_virtual_channels; vc++) {
        bfs.mask[vc] = buffer[i][vc].IsFull();
        bfs.fullness[vc] = buffer[i][vc].Size();
      }
      buffer_full_status_rx[i].write(bfs);
    }
  }
}

void Router::txProcess() {
  if (reset.read()) {
    // Clear outputs and indexes of transmitting protocol
    for (int i = 0; i < 2 * DIRECTIONS + 1; i++) {
      // Not valid on reset
      req_tx[i].write(0);
    }
  } else {
    // 0 phase: Checking
    // Round robin for input buffers ordering
    for (int i = 0; i < 2 * DIRECTIONS + 1; i++) {
      for (int vc = 0; vc < GlobalParams::n_virtual_channels; vc++) {
        if (!buffer[i][vc].IsEmpty()) {
          if (!reservation_status[i][vc]) {
            reservation_queue.push_back({i, vc});
            reservation_status[i][vc] = true;
          }
        } else {
          reservation_status[i][vc] = false;
          for (auto it = reservation_queue.begin();
               it != reservation_queue.end(); it++) {
            if (it->first == i && it->second == vc)
              assert(0);
          }
        }
      }
    }

    // Updating out buffers
    for (int o = 0; o < 2 * DIRECTIONS + 1; o++) {
      if (req_tx[o].read() && ack_tx[o].read()) {
        assert(!buffer_out[o][cur_out_vc[o]].IsEmpty());
        buffer_out[o][cur_out_vc[o]].Pop();
      }
    }

    // 1st phase: Reservation
    vector<pair<int, int>> reservation_queue_cpy;
    for (auto item_i_vc : reservation_queue) {
      int i = item_i_vc.first;
      int vc = item_i_vc.second;

      if (!buffer[i][vc].IsEmpty()) {
        Flit flit = buffer[i][vc].Front();
        power.bufferRouterFront();

        if (flit.flit_type(FLIT_TYPE_HEAD)) {
          // prepare data for routing
          RouteData route_data;
          route_data.current_id = local_id;
          route_data.src_id = flit.src_id;
          route_data.dst_id = flit.dst_id;
          route_data.local_direction_id = flit.local_direction_id;
          route_data.phys_channel_id = flit.phys_channel_id;
          route_data.dir_in = i;
          route_data.vc_id = flit.vc_id;

          // Routing: choosing output direction via route data
          int o = route(route_data);

          // manage special case of target hub not directly connected to
          // destination
          if (o >= DIRECTION_HUB_RELAY) {
            assert(false);
            Flit f = buffer[i][vc].Pop();
            f.hub_relay_node = o - DIRECTION_HUB_RELAY;
            buffer[i][vc].Push(f);
            o = DIRECTION_HUB;
          }

          TReservation r;
          r.input = i;
          r.vc = vc;
          assert(i != o);

          LOG << " checking availability of Output[" << o << "] for Input[" << i
              << "][" << vc << "] flit " << flit << endl;

          int rt_status = reservation_table.checkReservation(r, o, vc);

          if (rt_status == RT_AVAILABLE) {
            LOG << " reserving direction " << o << " for flit " << flit << endl;
            reservation_table.reserve(r, o, vc);
          } else if (rt_status == RT_ALREADY_SAME) {
            reservation_queue_cpy.push_back({i, vc});
            assert(0);
            LOG << " RT_ALREADY_SAME reserved direction " << o << " for flit "
                << flit << endl;
          } else if (rt_status == RT_OUTVC_BUSY) {
            reservation_queue_cpy.push_back({i, vc});
            LOG << " RT_OUTVC_BUSY reservation direction " << o << " for flit "
                << flit << endl;
          } else if (rt_status == RT_ALREADY_OTHER_OUT) {
            reservation_queue_cpy.push_back({i, vc});
            assert(0);
            LOG << "RT_ALREADY_OTHER_OUT: another output previously reserved "
                   "for the same flit "
                << endl;
          } else
            assert(false); // no meaningful status here
        }
      } else {
        assert(0);
      }
    }

    reservation_queue = reservation_queue_cpy;

    // 2nd phase: Forwarding
    // Move flits from input buffers to output buffers in accordance with
    // routing algorithm
    for (int o = 0; o < 2 * DIRECTIONS + 1; o++) {
      for (int vc_o = 0; vc_o < GlobalParams::n_virtual_channels; vc_o++) {
        vector<TReservation> reservations =
            reservation_table.getOutReservations(o, vc_o);

        if (reservations.size() != 0) {

          int rnd_idx = 0;

          int i = reservations[rnd_idx].input;
          int vc = reservations[rnd_idx].vc;

          assert(vc == vc_o);

          // can happen
          if (!buffer[i][vc].IsEmpty()) {
            // power contribution already computed in 1st phase
            Flit flit = buffer[i][vc].Front();

            if (!buffer_out[o][vc].IsFull()) {
              LOG << "Input[" << i << "][" << vc << "] forwarded to Output["
                  << o << "], flit: " << flit << endl;

              reservation_status[i][vc] = false;

              buffer[i][vc].Pop();
              buffer_out[o][vc].Push(flit);

              if (flit.flit_type(FLIT_TYPE_TAIL)) {
                TReservation r;
                r.input = i;
                r.vc = vc;
                reservation_table.release(r, o, vc);
              } else {
                assert(GlobalParams::max_packet_size > 1);
              }

              // Power & Stats -------------------------------------------------
              if (o == DIRECTION_HUB)
                power.r2hLink();
              else
                power.r2rLink();

              power.bufferRouterPop();
              power.crossBar();

              if (o >= DIRECTION_LOCAL_NORTH && o <= DIRECTION_LOCAL_WEST) {
                power.networkInterface();

                stats.receivedFlit(sc_time_stamp().to_double() /
                                       GlobalParams::clock_period_ps,
                                   flit);
                if (GlobalParams::max_volume_to_be_drained) {
                  if (drained_volume >= GlobalParams::max_volume_to_be_drained)
                    sc_stop();
                  else {
                    drained_volume++;
                    local_drained++;
                  }
                }
              } else if (!(i >= DIRECTION_LOCAL_NORTH &&
                           i <= DIRECTION_LOCAL_WEST)) // not generated locally
                routed_flits++;
              // End Power & Stats
              // -------------------------------------------------
            } else // buffer out is Full
            {
              // assert(0);
              LOG << " Cannot forward Input[" << i << "][" << vc
                  << "] to Output[" << o << "], flit: " << flit << endl;
              LOG << " **DEBUG buffer_full_status_tx "
                  << buffer_full_status_tx[o].read().mask[vc] << endl;
            }
          } else {
            assert(GlobalParams::max_packet_size > 1);
          }
        } // if not reserved
      }
    } // for loop directions

    reservation_table.updateIndex();

    // Step 3. Checking output buffers
    for (int o = 0; o < 2 * DIRECTIONS + 1; o++) {
      for (int vc = 0; vc < GlobalParams::n_virtual_channels; vc++) {
        if (!buffer_out[o][vc].IsEmpty() && !out_reservation_status[o][vc]) {
          out_reservation_queue[o].push(vc);
          out_reservation_status[o][vc] = true;
        }
      }
    }

    // Step 4. Managing output buffers
    for (int o = 0; o < 2 * DIRECTIONS + 1; o++) {
      if (!is_vc_set[o]) {
        req_tx[o].write(0);
        if (!out_reservation_queue[o].empty()) {
          free_slots[o].write(out_reservation_queue[o].front());
          is_vc_set[o] = true;
        }
      } else {
        req_tx[o].write(1);
        int vc = out_reservation_queue[o].front();
        cur_out_vc[o] = vc;
        flit_tx[o]->write(buffer_out[o][vc].Front());
        out_reservation_status[o][vc] = false;
        out_reservation_queue[o].pop();
        if (out_reservation_queue[o].empty()) {
          if (buffer_out[o][vc].Size() > 1) {
            free_slots[o].write(vc);
            out_reservation_queue[o].push(vc);
            out_reservation_status[o][vc] = true;
          } else {
            is_vc_set[o] = false;
          }
        } else {
          free_slots[o].write(out_reservation_queue[o].front());
        }
      }
    }
  }
}

bool Router::is_memory_pe(int id) {
  Coord coord = id2Coord(id);

  if (coord.x == 0)
    return false;
  if (coord.y == 0)
    return false;
  if (coord.x == GlobalParams::mesh_dim_x)
    return false;
  if (coord.y == GlobalParams::mesh_dim_y)
    return false;

  return true;
}

NoP_data Router::getCurrentNoPData() {
  NoP_data NoP_data;

  for (int j = 0; j < DIRECTIONS; j++) {
    try {
      NoP_data.channel_status_neighbor[j].free_slots =
          free_slots_neighbor[j].read();
      NoP_data.channel_status_neighbor[j].available =
          (reservation_table.isNotReserved(j, 0));
    } catch (int e) {
      if (e != NOT_VALID)
        assert(false);
      // Nothing to do if an NOT_VALID direction is caught
    };
  }

  NoP_data.sender_id = local_id;

  return NoP_data;
}

void Router::perCycleUpdate() {
  if (reset.read()) {
    for (int i = 0; i < 2 * DIRECTIONS; i++) {
      // free_slots[i].write(buffer[i][DEFAULT_VC].GetMaxBufferSize());
      // free_slots[i].write(buffer_out[i][DEFAULT_VC].GetMaxBufferSize());
    }
  } else {
    selectionStrategy->perCycleUpdate(this);

    power.leakageRouter();
    for (int i = 0; i < 2 * DIRECTIONS; i++) {
      for (int vc = 0; vc < GlobalParams::n_virtual_channels; vc++) {
        power.leakageBufferRouter();
        power.leakageLinkRouter2Router();
      }
    }

    power.leakageLinkRouter2Hub();
  }
}

vector<int> Router::nextDeltaHops(RouteData rd) {

  if (GlobalParams::topology == TOPOLOGY_MESH) {
    cout << "Mesh topologies are not supported for nextDeltaHops() ";
    assert(false);
  }
  // annotate the initial nodes
  int src = rd.src_id;
  int dst = rd.dst_id;

  int current_node = src;
  vector<int> direction; // initially is empty
  vector<int> next_hops;

  // int sw = GlobalParams::n_delta_tiles/2; //sw: switch number in each stage
  int stg = log2(GlobalParams::n_delta_tiles);
  int c = 0;
  //---From Source to stage 0 (return the sw attached to the source)---
  // Topology omega
  if (GlobalParams::topology == TOPOLOGY_OMEGA) {
    if (current_node < (GlobalParams::n_delta_tiles / 2))
      c = current_node;
    else if (current_node >= (GlobalParams::n_delta_tiles / 2))
      c = (current_node - (GlobalParams::n_delta_tiles / 2));
  }
  // Other delta topologies: Butterfly and baseline
  else if ((GlobalParams::topology == TOPOLOGY_BUTTERFLY) ||
           (GlobalParams::topology == TOPOLOGY_BASELINE)) {
    c = (current_node >> 1);
  }

  Coord temp_coord;
  temp_coord.x = 0;
  temp_coord.y = c;
  int N = coord2Id(temp_coord);

  next_hops.push_back(N);
  current_node = N;

  //---From stage 0 to Destination---
  int current_stage = 0;

  while (current_stage < stg - 1) {
    Coord new_coord;
    int y = id2Coord(current_node).y;

    rd.current_id = current_node;
    direction = routingAlgorithm->route(this, rd);

    int bit_to_check = stg - current_stage - 1;

    int bit_checked = (y & (1 << (bit_to_check - 1))) > 0 ? 1 : 0;

    // computes next node coords
    new_coord.x = current_stage + 1;
    if (bit_checked ^ direction[0])
      new_coord.y = toggleKthBit(y, bit_to_check);
    else
      new_coord.y = y;

    current_node = coord2Id(new_coord);
    next_hops.push_back(current_node);
    current_stage = id2Coord(current_node).x;
  }

  next_hops.push_back(dst);

  return next_hops;
}

vector<int> Router::routingFunction(const RouteData &route_data) {
  if (GlobalParams::use_winoc) {
    // - If the current node C and the destination D are connected to an
    // radiohub, use wireless
    // - If D is not directly connected to a radio hub, wireless
    // communication can still  be used if some intermediate node "I" in the
    // routing path is reachable from current node C.
    // - Since further wired hops will be required from I -> D, a threshold
    // "winoc_dst_hops" can be specified (via command line) to determine the max
    // distance from the intermediate node I and the destination D.
    // - NOTE: default threshold is 0, which means I=D, i.e., we explicitly ask
    // the destination D to be connected to the target radio hub
    if (hasRadioHub(local_id)) {
      // Check if destination is directly connected to an hub
      if (hasRadioHub(route_data.dst_id) &&
          !sameRadioHub(local_id, route_data.dst_id)) {
        map<int, int>::iterator it1 =
            GlobalParams::hub_for_tile.find(route_data.dst_id);
        map<int, int>::iterator it2 =
            GlobalParams::hub_for_tile.find(route_data.current_id);

        if (connectedHubs(it1->second, it2->second)) {
          LOG << "Destination node " << route_data.dst_id
              << " is directly connected to a reachable RadioHub" << endl;
          vector<int> dirv;
          dirv.push_back(DIRECTION_HUB);
          return dirv;
        }
      }
      // let's check whether some node in the route has an acceptable distance
      // to the dst
      if (GlobalParams::winoc_dst_hops > 0) {
        // TODO: for the moment, just print the set of nexts hops to check
        // everything is ok
        LOG << "NEXT_DELTA_HOPS (from node " << route_data.src_id << " to "
            << route_data.dst_id << ") >>>> :";
        vector<int> nexthops;
        nexthops = nextDeltaHops(route_data);
        for (int i = 1; i <= GlobalParams::winoc_dst_hops; i++) {
          int dest_position = nexthops.size() - 1;
          int candidate_hop = nexthops[dest_position - i];
          if (hasRadioHub(candidate_hop) &&
              !sameRadioHub(local_id, candidate_hop)) {
            // LOG << "Checking candidate hop " << candidate_hop << " ... It's
            // OK!" << endl;
            LOG << "Relaying to hub-connected node " << candidate_hop
                << " to reach destination " << route_data.dst_id << endl;
            vector<int> dirv;
            dirv.push_back(DIRECTION_HUB_RELAY + candidate_hop);
            return dirv;
          }
          // else
          //  LOG << "Checking candidate hop " << candidate_hop << " ... NOT OK"
          //  << endl;
        }
      }
    }
  }
  // TODO: fix all the deprecated verbose mode logs
  if (GlobalParams::verbose_mode > VERBOSE_OFF)
    LOG << "Wired routing for dst = " << route_data.dst_id << endl;

  // not wireless direction taken, apply normal routing
  return routingAlgorithm->route(this, route_data);
}

int Router::route(const RouteData &route_data) {

  if (route_data.dst_id == local_id)
    return route_data.local_direction_id;

  power.routing();
  vector<int> candidate_channels = routingFunction(route_data);

  power.selection();
  return selectionFunction(candidate_channels, route_data);
}

void Router::NoP_report() const {
  NoP_data NoP_tmp;
  LOG << "NoP report: " << endl;

  for (int i = 0; i < DIRECTIONS; i++) {
    NoP_tmp = NoP_data_in[i].read();
    if (NoP_tmp.sender_id != NOT_VALID)
      cout << NoP_tmp;
  }
}

//---------------------------------------------------------------------------

int Router::NoPScore(const NoP_data &nop_data,
                     const vector<int> &nop_channels) const {
  int score = 0;

  for (unsigned int i = 0; i < nop_channels.size(); i++) {
    int available;

    if (nop_data.channel_status_neighbor[nop_channels[i]].available)
      available = 1;
    else
      available = 0;

    int free_slots =
        nop_data.channel_status_neighbor[nop_channels[i]].free_slots;

    score += available * free_slots;
  }

  return score;
}

int Router::selectionFunction(const vector<int> &directions,
                              const RouteData &route_data) {
  // not so elegant but fast escape ;)
  if (directions.size() == 1)
    return directions[0];

  return selectionStrategy->apply(this, directions, route_data);
}

void Router::configure(const int _id, const double _warm_up_time,
                       const unsigned int _max_buffer_size,
                       GlobalRoutingTable &grt) {
  local_id = _id;
  stats.configure(_id, _warm_up_time);

  start_from_port = DIRECTION_LOCAL_NORTH;
  for (int i = 0; i < 2 * DIRECTIONS + 1; i++) {
    for (int vc = 0; vc < GlobalParams::n_virtual_channels; vc++) {
      reservation_status[i][vc] = false;
      out_reservation_status[i][vc] = false;
    }
    is_vc_set[i] = false;
    cur_out_vc[i] = false;
  }

  if (grt.isValid())
    routing_table.configure(grt, _id);

  reservation_table.setSize(2 * DIRECTIONS + 1,
                            GlobalParams::n_virtual_channels);

  for (int i = 0; i < 2 * DIRECTIONS + 1; i++) {
    for (int vc = 0; vc < GlobalParams::n_virtual_channels; vc++) {
      buffer[i][vc].SetMaxBufferSize(_max_buffer_size);
      buffer_out[i][vc].SetMaxBufferSize(_max_buffer_size);
      buffer[i][vc].setLabel(string(name()) + "->buffer[" + i_to_string(i) +
                             "]");
      buffer_out[i][vc].setLabel(string(name()) + "->buffer[" + i_to_string(i) +
                                 "]");
    }
  }

  if (GlobalParams::topology == TOPOLOGY_MESH) {
    int row = _id / GlobalParams::mesh_dim_x;
    int col = _id % GlobalParams::mesh_dim_x;

    for (int vc = 0; vc < GlobalParams::n_virtual_channels; vc++) {
      if (row == 0) {
        buffer[DIRECTION_NORTH][vc].Disable();
        buffer_out[DIRECTION_NORTH][vc].Disable();
      }
      if (row == GlobalParams::mesh_dim_y - 1) {
        buffer[DIRECTION_SOUTH][vc].Disable();
        buffer_out[DIRECTION_SOUTH][vc].Disable();
      }
      if (col == 0) {
        buffer[DIRECTION_WEST][vc].Disable();
        buffer_out[DIRECTION_WEST][vc].Disable();
      }
      if (col == GlobalParams::mesh_dim_x - 1) {
        buffer[DIRECTION_EAST][vc].Disable();
        buffer_out[DIRECTION_EAST][vc].Disable();
      }
    }
  }
}

unsigned long Router::getRoutedFlits() { return routed_flits; }

int Router::reflexDirection(int direction) const {
  if (direction == DIRECTION_NORTH)
    return DIRECTION_SOUTH;
  if (direction == DIRECTION_EAST)
    return DIRECTION_WEST;
  if (direction == DIRECTION_WEST)
    return DIRECTION_EAST;
  if (direction == DIRECTION_SOUTH)
    return DIRECTION_NORTH;

  // you shouldn't be here
  assert(false);
  return NOT_VALID;
}

int Router::getNeighborId(int _id, int direction) const {
  assert(GlobalParams::topology == TOPOLOGY_MESH);

  Coord my_coord = id2Coord(_id);

  switch (direction) {
  case DIRECTION_NORTH:
    if (my_coord.y == 0)
      return NOT_VALID;
    my_coord.y--;
    break;
  case DIRECTION_SOUTH:
    if (my_coord.y == GlobalParams::mesh_dim_y - 1)
      return NOT_VALID;
    my_coord.y++;
    break;
  case DIRECTION_EAST:
    if (my_coord.x == GlobalParams::mesh_dim_x - 1)
      return NOT_VALID;
    my_coord.x++;
    break;
  case DIRECTION_WEST:
    if (my_coord.x == 0)
      return NOT_VALID;
    my_coord.x--;
    break;
  default:
    LOG << "Direction not valid : " << direction;
    assert(false);
  }

  int neighbor_id = coord2Id(my_coord);

  return neighbor_id;
}

bool Router::inCongestion() {
  for (int i = 0; i < DIRECTIONS; i++) {

    if (free_slots_neighbor[i] == NOT_VALID)
      continue;

    int flits = GlobalParams::buffer_depth - free_slots_neighbor[i];
    if (flits >
        (int)(GlobalParams::buffer_depth * GlobalParams::dyad_threshold))
      return true;
  }

  return false;
}

void Router::ShowBuffersStats(std::ostream &out) {
  for (int i = 0; i < 2 * DIRECTIONS + 1; i++)
    for (int vc = 0; vc < GlobalParams::n_virtual_channels; vc++)
      buffer[i][vc].ShowStats(out);
}

bool Router::connectedHubs(int src_hub, int dst_hub) {
  vector<int> &first = GlobalParams::hub_configuration[src_hub].txChannels;
  vector<int> &second = GlobalParams::hub_configuration[dst_hub].rxChannels;

  vector<int> intersection;

  for (unsigned int i = 0; i < first.size(); i++) {
    for (unsigned int j = 0; j < second.size(); j++) {
      if (first[i] == second[j])
        intersection.push_back(first[i]);
    }
  }

  if (intersection.size() == 0)
    return false;
  else
    return true;
}
