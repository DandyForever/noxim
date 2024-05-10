/*
 * Noxim - the NoC Simulator
 *
 * (C) 2005-2018 by the University of Catania
 * For the complete list of authors refer to file ../doc/AUTHORS.txt
 * For the license applied to these sources refer to file ../doc/LICENSE.txt
 *
 * This file contains the implementaton of the global statistics
 */

#include "GlobalStats.h"
using namespace std;

GlobalStats::GlobalStats(const NoC * _noc)
{
    noc = _noc;

	#ifdef TESTING
    drained_total = 0;
	#endif
}

unsigned long GlobalStats::getLocalDirectionsTotal(vector<vector<vector<unsigned long>>>& stat_mtx, Coord coord) {
	assert(GlobalParams::topology == TOPOLOGY_MESH);

	unsigned long result = 0;
	for (unsigned int loc_dir = 0; loc_dir < DIRECTIONS; loc_dir++) {
		result += stat_mtx[loc_dir][coord.y][coord.x];
	}

	return result;
}

double GlobalStats::getAverageDelay()
{
    unsigned int total_packets = 0;
    double avg_delay = 0.0;

    if (GlobalParams::topology == TOPOLOGY_MESH)
    {
	for (int y = 0; y < GlobalParams::mesh_dim_y; y++)
	    for (int x = 0; x < GlobalParams::mesh_dim_x; x++) 
	    {
		unsigned int received_packets =
		    noc->t[x][y]->r->stats.getReceivedPackets();

		if (received_packets) 
		{
		    avg_delay +=
			received_packets *
			noc->t[x][y]->r->stats.getAverageDelay();
		    total_packets += received_packets;
		}
	    }
    }
    else // other delta topologies
    { 
	for (int y = 0; y < GlobalParams::n_delta_tiles; y++)
	{
	    unsigned int received_packets =
		noc->core[y]->r->stats.getReceivedPackets();

	    if (received_packets) 
	    {
		avg_delay +=
		    received_packets *
		    noc->core[y]->r->stats.getAverageDelay();
		total_packets += received_packets;
	    }
	}

    }


    avg_delay /= (double) total_packets;

    return avg_delay;
}



double GlobalStats::getAverageDelay(const int src_id,
					 const int dst_id)
{
    Tile *tile = noc->searchNode(dst_id);

    assert(tile != NULL);

    return tile->r->stats.getAverageDelay(src_id);
}

double GlobalStats::getMaxDelay()
{
    double maxd = -1.0;

    if (GlobalParams::topology == TOPOLOGY_MESH) 
    {
	for (int y = 0; y < GlobalParams::mesh_dim_y; y++)
	    for (int x = 0; x < GlobalParams::mesh_dim_x; x++) 
	    {
		Coord coord;
		coord.x = x;
		coord.y = y;
		int node_id = coord2Id(coord);
		double d = getMaxDelay(node_id);
		if (d > maxd)
		    maxd = d;
	    }

    }
    else  // other delta topologies 
    {
	for (int y = 0; y < GlobalParams::n_delta_tiles; y++)
	{
	    double d = getMaxDelay(y);
	    if (d > maxd)
		maxd = d;
	}
    }

    return maxd;
}

double GlobalStats::getMaxDelay(const int node_id)
{
    if (GlobalParams::topology == TOPOLOGY_MESH) 
    {
	Coord coord = id2Coord(node_id);

	unsigned int received_packets =
	    noc->t[coord.x][coord.y]->r->stats.getReceivedPackets();

	if (received_packets)
	    return noc->t[coord.x][coord.y]->r->stats.getMaxDelay();
	else
	    return -1.0;
    }
    else // other delta topologies
    {
	unsigned int received_packets =
	    noc->core[node_id]->r->stats.getReceivedPackets();
	if (received_packets)
	    return noc->core[node_id]->r->stats.getMaxDelay();
	else
	    return -1.0;
    }

}

double GlobalStats::getMaxDelay(const int src_id, const int dst_id)
{
    Tile *tile = noc->searchNode(dst_id);

    assert(tile != NULL);

    return tile->r->stats.getMaxDelay(src_id);
}

vector < vector < double > > GlobalStats::getMaxDelayMtx()
{
    vector < vector < double > > mtx;

    assert(GlobalParams::topology == TOPOLOGY_MESH); 

    mtx.resize(GlobalParams::mesh_dim_y);
    for (int y = 0; y < GlobalParams::mesh_dim_y; y++)
	mtx[y].resize(GlobalParams::mesh_dim_x);

    for (int y = 0; y < GlobalParams::mesh_dim_y; y++)
	for (int x = 0; x < GlobalParams::mesh_dim_x; x++) 
	{
	    Coord coord;
	    coord.x = x;
	    coord.y = y;
	    int id = coord2Id(coord);
	    mtx[y][x] = getMaxDelay(id);
	}

    return mtx;
}

double GlobalStats::getAverageThroughput(const int src_id, const int dst_id)
{
    Tile *tile = noc->searchNode(dst_id);

    assert(tile != NULL);

    return tile->r->stats.getAverageThroughput(src_id);
}

/*
double GlobalStats::getAverageThroughput()
{
    unsigned int total_comms = 0;
    double avg_throughput = 0.0;

    for (int y = 0; y < GlobalParams::mesh_dim_y; y++)
	for (int x = 0; x < GlobalParams::mesh_dim_x; x++) {
	    unsigned int ncomms =
		noc->t[x][y]->r->stats.getTotalCommunications();

	    if (ncomms) {
		avg_throughput +=
		    ncomms * noc->t[x][y]->r->stats.getAverageThroughput();
		total_comms += ncomms;
	    }
	}

    avg_throughput /= (double) total_comms;

    return avg_throughput;
}
*/

double GlobalStats::getAggregatedThroughput()
{
    int total_cycles = GlobalParams::simulation_time - GlobalParams::stats_warm_up_time;

    return (double)getReceivedFlits()/(double)(total_cycles);
}

unsigned int GlobalStats::getReceivedPackets()
{
    unsigned int n = 0;

    if (GlobalParams::topology == TOPOLOGY_MESH) 
    {
    	for (int y = 0; y < GlobalParams::mesh_dim_y; y++)
		for (int x = 0; x < GlobalParams::mesh_dim_x; x++)
	    n += noc->t[x][y]->r->stats.getReceivedPackets();
    }
    else // other delta topologies
    {
    	for (int y = 0; y < GlobalParams::n_delta_tiles; y++)
	    n += noc->core[y]->r->stats.getReceivedPackets();
    }

    return n;
}

unsigned int GlobalStats::getReceivedFlits()
{
    unsigned int n = 0;
    if (GlobalParams::topology == TOPOLOGY_MESH) 
    {
	for (int y = 0; y < GlobalParams::mesh_dim_y; y++)
	    for (int x = 0; x < GlobalParams::mesh_dim_x; x++) {
		n += noc->t[x][y]->r->stats.getReceivedFlits();
#ifdef TESTING
		drained_total += noc->t[x][y]->r->local_drained;
#endif
	    }
    }
    else // other delta topologies
    {
	for (int y = 0; y < GlobalParams::n_delta_tiles; y++)
	{
	    n += noc->core[y]->r->stats.getReceivedFlits();
#ifdef TESTING
	    drained_total += noc->core[y]->r->local_drained;
#endif
	}
    }

    return n;
}

double GlobalStats::getThroughput()
{
    if (GlobalParams::topology == TOPOLOGY_MESH) 
    {
	int number_of_ip = GlobalParams::mesh_dim_x * GlobalParams::mesh_dim_y;
	return (double)getAggregatedThroughput()/(double)(number_of_ip);
    }
    else // other delta topologies
    {
	int number_of_ip = GlobalParams::n_delta_tiles;
	return (double)getAggregatedThroughput()/(double)(number_of_ip);
    }
}

// Only accounting IP that received at least one flit
double GlobalStats::getActiveThroughput()
{
    int total_cycles =
	GlobalParams::simulation_time -
	GlobalParams::stats_warm_up_time;
    unsigned int n = 0;
    unsigned int trf = 0;
    unsigned int rf ;
    if (GlobalParams::topology == TOPOLOGY_MESH) 
    {
	for (int y = 0; y < GlobalParams::mesh_dim_y; y++)
	    for (int x = 0; x < GlobalParams::mesh_dim_x; x++) 
	    {
		rf = noc->t[x][y]->r->stats.getReceivedFlits();

		if (rf != 0)
		    n++;

		trf += rf;
	    }
    }
    else // other delta topologies
    {
	for (int y = 0; y < GlobalParams::n_delta_tiles; y++)
	{
	    rf = noc->core[y]->r->stats.getReceivedFlits();

	    if (rf != 0)
		n++;

	    trf += rf;
	}
    }

    return (double) trf / (double) (total_cycles * n);

}

vector < vector < unsigned long > > GlobalStats::getRoutedFlitsMtx(char dir)
{

	vector < vector < unsigned long > > mtx;
	assert (GlobalParams::topology == TOPOLOGY_MESH);
	assert (dir == 'x' || dir == 'y');

	mtx.resize(GlobalParams::mesh_dim_y);
	for (int y = 0; y < GlobalParams::mesh_dim_y; y++)
		mtx[y].resize(GlobalParams::mesh_dim_x);

	for (int y = 0; y < GlobalParams::mesh_dim_y; y++)
		for (int x = 0; x < GlobalParams::mesh_dim_x; x++)
		if (dir == 'x')
			mtx[y][x] = noc->t[x][y]->r->getRoutedFlits();
		else
			mtx[y][x] = noc->t[x][y]->r_req->getRoutedFlits();


	return mtx;
}

vector < vector < unsigned long > > GlobalStats::getSentFlits(char dir)
{

	vector < vector < unsigned long > > mtx;
	assert (GlobalParams::topology == TOPOLOGY_MESH);
	assert (dir == 'x' || dir == 'y');

	mtx.resize(GlobalParams::mesh_dim_y);
	for (int y = 0; y < GlobalParams::mesh_dim_y; y++)
		mtx[y].resize(GlobalParams::mesh_dim_x);

	for (int y = 0; y < GlobalParams::mesh_dim_y; y++)
		for (int x = 0; x < GlobalParams::mesh_dim_x; x++)
			for (int y_ = 0; y_ < GlobalParams::mesh_dim_y; y_++)
				for (int x_ = 0; x_ < GlobalParams::mesh_dim_x; x_++)
					if (dir == 'x')
						mtx[y][x] += noc->t[x_][y_]->r->stats.getFlitsSrcCount(y*GlobalParams::mesh_dim_x+x);
					else
						mtx[y][x] += noc->t[x_][y_]->r_req->stats.getFlitsSrcCount(y*GlobalParams::mesh_dim_x+x);

	return mtx;
}

vector < vector < unsigned long > > GlobalStats::getRecvFlits(int pe_id, char dir = 'x')
{
	vector < vector < unsigned long > > mtx;
  assert (GlobalParams::topology == TOPOLOGY_MESH);
	assert (dir == 'x' || dir == 'y');

	mtx.resize(GlobalParams::mesh_dim_y);
	for (int y = 0; y < GlobalParams::mesh_dim_y; y++)
		mtx[y].resize(GlobalParams::mesh_dim_x);

	for (int y = 0; y < GlobalParams::mesh_dim_y; y++)
		for (int x = 0; x < GlobalParams::mesh_dim_x; x++)
			if (dir == 'x')
				mtx[y][x] += noc->t[x][y]->pe[pe_id]->flits_recv_x;
			else
				mtx[y][x] += noc->t[x][y]->pe[pe_id]->flits_recv_y;

	return mtx;
}

vector < vector < unsigned long > > GlobalStats::getSentPEFlits(int pe_id, char dir = 'x')
{
	vector < vector < unsigned long > > mtx;
  assert (GlobalParams::topology == TOPOLOGY_MESH);
	assert (dir == 'x' || dir == 'y');

  mtx.resize(GlobalParams::mesh_dim_y);
  for (int y = 0; y < GlobalParams::mesh_dim_y; y++)
		mtx[y].resize(GlobalParams::mesh_dim_x);

	for (int y = 0; y < GlobalParams::mesh_dim_y; y++)
		for (int x = 0; x < GlobalParams::mesh_dim_x; x++)
			if (dir == 'x')
				mtx[y][x] += noc->t[x][y]->pe[pe_id]->flits_sent_x;
			else
				mtx[y][x] += noc->t[x][y]->pe[pe_id]->flits_sent_y;

	return mtx;
}

unsigned int GlobalStats::getWirelessPackets()
{
    unsigned int packets = 0;

    // Wireless noc
    for (map<int, HubConfig>::iterator it = GlobalParams::hub_configuration.begin();
            it != GlobalParams::hub_configuration.end();
            ++it)
    {
	int hub_id = it->first;

	map<int,Hub*>::const_iterator i = noc->hub.find(hub_id);
	Hub * h = i->second;

	packets+= h->wireless_communications_counter;
    }
    return packets;
}

double GlobalStats::getDynamicPower()
{
    double power = 0.0;

    // Electric noc
    if (GlobalParams::topology == TOPOLOGY_MESH) 
    {
	for (int y = 0; y < GlobalParams::mesh_dim_y; y++)
	    for (int x = 0; x < GlobalParams::mesh_dim_x; x++)
		power += noc->t[x][y]->r->power.getDynamicPower();
    }
    else // other delta topologies
    {
	int stg = log2(GlobalParams::n_delta_tiles);
	int sw = GlobalParams::n_delta_tiles/2; //sw: switch number in each stage
	// Dimensions of the delta switch block network
	int dimX = stg;
	int dimY = sw;

	// power for delta topologies cores
	for (int y = 0; y < GlobalParams::n_delta_tiles; y++)
	    power += noc->core[y]->r->power.getDynamicPower();

	// power for delta topologies switches 
	for (int y = 0; y < dimY; y++)
	    for (int x = 0; x < dimX; x++)
		power += noc->t[x][y]->r->power.getDynamicPower();
    }

    // Wireless noc
    for (map<int, HubConfig>::iterator it = GlobalParams::hub_configuration.begin();
	    it != GlobalParams::hub_configuration.end();
	    ++it)
    {
	int hub_id = it->first;

	map<int,Hub*>::const_iterator i = noc->hub.find(hub_id);
	Hub * h = i->second;

	power+= h->power.getDynamicPower();
    }
    return power;
}

double GlobalStats::getStaticPower()
{
    double power = 0.0;

    if (GlobalParams::topology == TOPOLOGY_MESH) 
    {
    	for (int y = 0; y < GlobalParams::mesh_dim_y; y++)
		for (int x = 0; x < GlobalParams::mesh_dim_x; x++)
	    power += noc->t[x][y]->r->power.getStaticPower();
    }
    else // other delta topologies
    {
	int stg = log2(GlobalParams::n_delta_tiles);
	int sw = GlobalParams::n_delta_tiles/2; //sw: switch number in each stage
	// Dimensions of the delta switch block network
	int dimX = stg;
	int dimY = sw;
	// power for delta topologies switches 
	for (int y = 0; y < dimY; y++)
	    for (int x = 0; x < dimX; x++)
		power += noc->t[x][y]->r->power.getDynamicPower();

	// delta cores
    	for (int y = 0; y < GlobalParams::n_delta_tiles; y++)
	    power += noc->core[y]->r->power.getStaticPower();
    }

    // Wireless noc
    for (map<int, HubConfig>::iterator it = GlobalParams::hub_configuration.begin();
            it != GlobalParams::hub_configuration.end();
            ++it)
    {
	int hub_id = it->first;

	map<int,Hub*>::const_iterator i = noc->hub.find(hub_id);
	Hub * h = i->second;

	power+= h->power.getStaticPower();
    }
    return power;
}

void GlobalStats::showStats(std::ostream & out, bool detailed) {

	if (detailed) {

		assert (GlobalParams::topology == TOPOLOGY_MESH);

		stringstream file_name;
		file_name << GlobalParams::log_file_name <<
			"_mesh_" << GlobalParams::mesh_dim_x << "x" <<
			GlobalParams::mesh_dim_y << "_" <<
			GlobalParams::routing_algorithm <<
			"_mp_" << GlobalParams::mem_ports;
			if (GlobalParams::interliving_reps)
				file_name << "_il_" << GlobalParams::interliving_reps;
			if (GlobalParams::GlobalParams::switch_angle_masters)
				file_name << "_sv_" << GlobalParams::switch_angle_masters;
			if (GlobalParams::switch_vertical_masters)
				file_name << "_sv_all_";
			if (GlobalParams::switch_horizontal_masters)
				file_name << "_sh_" << GlobalParams::switch_horizontal_masters;
			file_name << "_pir_" << (int)(GlobalParams::packet_injection_rate * 1000);
		std::ofstream f_sent_flits (file_name.str() + ".csv", std::ofstream::out);
		std::ofstream f_latencies (file_name.str() + "_latency.log", std::ofstream::out);

		vector < char > directions;
		directions.push_back('x');
		directions.push_back('y');

		//-----------------------------------------------------------------------
		// Start Router stats
		//-----------------------------------------------------------------------
		// Router x
		out << endl << "detailed router [x] = [" << endl;

		for (int y = 0; y < GlobalParams::mesh_dim_y; y++)
			for (int x = 0; x < GlobalParams::mesh_dim_x; x++)
				noc->t[x][y]->r->stats.showStats(y * GlobalParams:: mesh_dim_x + x, out, true);
		out << "];" << endl;

		// Router y (Router req)
		out << endl << "detailed router [y] = [" << endl;

		for (int y = 0; y < GlobalParams::mesh_dim_y; y++)
			for (int x = 0; x < GlobalParams::mesh_dim_x; x++)
				noc->t[x][y]->r_req->stats.showStats(y * GlobalParams:: mesh_dim_x + x, out, true);
		out << "];" << endl;
		//-----------------------------------------------------------------------
		// End Router stats
		//-----------------------------------------------------------------------

		//-----------------------------------------------------------------------
		// Start MaxDelay
		//-----------------------------------------------------------------------
		vector < vector < double > > md_mtx = getMaxDelayMtx();

		out << endl << "max_delay = [" << endl;
		for (unsigned int y = 0; y < md_mtx.size(); y++) {
			out << "   ";
			for (unsigned int x = 0; x < md_mtx[y].size(); x++)
				out << setw(6) << md_mtx[y][x];
			out << endl;
		}
		out << "];" << endl;
		//-----------------------------------------------------------------------
		// End MaxDelay
		//-----------------------------------------------------------------------

		//-----------------------------------------------------------------------
		// Start RoutedFlits
		//-----------------------------------------------------------------------
		for (auto dir : directions) {
			vector < vector < unsigned long > > rf_mtx = getRoutedFlitsMtx(dir);

			out << endl << "routed_flits[" << dir << "] = [" << endl;
			for (unsigned int y = 0; y < rf_mtx.size(); y++) {
				out << "   ";
				for (unsigned int x = 0; x < rf_mtx[y].size(); x++)
					out << setw(10) << rf_mtx[y][x];
				out << endl;
			}
			out << "]" << endl;
		}
		//-----------------------------------------------------------------------
		// End RoutedFlits
		//-----------------------------------------------------------------------

		//-----------------------------------------------------------------------
		// Start SentFlits (Router side)
		//-----------------------------------------------------------------------
		for (auto dir : directions) {
			vector < vector < unsigned long > > sp_mtx = getSentFlits(dir);

			// Print stats to output
			out << endl << "sent_flits[" << dir << "] = [" << endl;
			for (unsigned int y = 0; y < sp_mtx.size(); y++) {
				out << "  ";
				for (unsigned int x = 0; x < sp_mtx[y].size(); x++) {
					out << setw(10) << sp_mtx[y][x] << ',';
				}
				out << endl;
			}
			out << "]" << endl;

			// Print stats to file
			f_sent_flits << "SentFlits[" << dir << "] (Router side)" << endl;
			for (int y = 0; y < GlobalParams::mesh_dim_y; y++) {
				for (int x = 0; x < GlobalParams::mesh_dim_x; x++) {
					f_sent_flits << sp_mtx[y][x] << ';';
				}
				f_sent_flits << endl;
			}
		}
		//-----------------------------------------------------------------------
		// End SentFlits (Router side)
		//-----------------------------------------------------------------------

		//-----------------------------------------------------------------------
		// Start RecvFlits (PE side)
		//-----------------------------------------------------------------------

		for (auto dir : directions) {
			vector < vector < vector < unsigned long > > > recvf_mtx_pe;

			// Aggregate all local directions
			for (unsigned int loc_dir = 0; loc_dir < DIRECTIONS; loc_dir++) {
				recvf_mtx_pe.push_back(getRecvFlits(loc_dir, dir));
			}

			// Print stats for all local directions
			if (GlobalParams::traffic_verbose) {
				for (unsigned int loc_dir = 0; loc_dir < DIRECTIONS; loc_dir++) {
					out << endl << "received_flits_pe[" << loc_dir << "][" << dir << "] = [" << endl;
					for (int y = 0; y < GlobalParams::mesh_dim_y; y++) {
						out << "  ";
						for (int x = 0; x < GlobalParams::mesh_dim_x; x++) {
							out << setw(10) << recvf_mtx_pe[loc_dir][y][x] << ',';
						}
						out << endl;
					}
					out << "]" << endl;
				}
			}

			// Print stats for sum of local directions
			out << endl << "received_flits_total[" << dir << "] = [" << endl;
			for (int y = 0; y < GlobalParams::mesh_dim_y; y++) {
				out << "  ";
				for (int x = 0; x < GlobalParams::mesh_dim_x; x++) {
					out << setw(10) << getLocalDirectionsTotal(recvf_mtx_pe, {x, y}) << ',';
				}
				out << endl;
			}
			out << "]" << endl;

			f_sent_flits << "RecvFlits (PE side) direction " << dir << endl;
			for (int y = 0; y < GlobalParams::mesh_dim_y; y++) {
				for (int x = 0; x < GlobalParams::mesh_dim_x; x++) {
					f_sent_flits << getLocalDirectionsTotal(recvf_mtx_pe, {x, y}) << ';';
				}
				f_sent_flits << endl;
			}
		}
		//-----------------------------------------------------------------------
		// End RecvFlits (PE side)
		//-----------------------------------------------------------------------

		//-----------------------------------------------------------------------
		// Start SentFlits (PE side)
		//-----------------------------------------------------------------------

		for (auto dir : directions) {
			vector < vector < vector < unsigned long > > > sentf_mtx_pe;

			// Aggregate all local directions
			for (unsigned int loc_dir = 0; loc_dir < DIRECTIONS; loc_dir++) {
				sentf_mtx_pe.push_back(getSentPEFlits(loc_dir, dir));
			}

			// Print stats for all local directions
			if (GlobalParams::traffic_verbose) {
				for (unsigned int loc_dir = 0; loc_dir < DIRECTIONS; loc_dir++) {
					out << endl << "sent_flits_pe[" << loc_dir << "][" << dir << "] = [" << endl;
					for (int y = 0; y < GlobalParams::mesh_dim_y; y++) {
						out << "  ";
						for (int x = 0; x < GlobalParams::mesh_dim_x; x++) {
							out << setw(10) << sentf_mtx_pe[loc_dir][y][x] << ',';
						}
						out << endl;
					}
					out << "]" << endl;
				}
			}

			// Print stats for sum of local directions
			out << endl << "sent_flits_total[" << dir << "] = [" << endl;
			for (int y = 0; y < GlobalParams::mesh_dim_y; y++) {
				out << "  ";
				for (int x = 0; x < GlobalParams::mesh_dim_x; x++) {
					out << setw(10) << getLocalDirectionsTotal(sentf_mtx_pe, {x, y}) << ',';
				}
				out << endl;
			}
			out << "]" << endl;

			f_sent_flits << "SentFlits (PE side) direction " << dir << endl;
			for (int y = 0; y < GlobalParams::mesh_dim_y; y++) {
				for (int x = 0; x < GlobalParams::mesh_dim_x; x++) {
					f_sent_flits << getLocalDirectionsTotal(sentf_mtx_pe, {x, y}) << ';';
				}
				f_sent_flits << endl;
			}
		}
		//-----------------------------------------------------------------------
		// End RecvFlits (PE side)
		//-----------------------------------------------------------------------
		f_sent_flits.close();

		//-----------------------------------------------------------------------
		// Start Router buffers state for deadlock analysis
		//-----------------------------------------------------------------------
		if (GlobalParams::buffer_verbose) {
			out << "Buffer out state for router " << endl;
			for (int d = 0; d < DIRECTIONS + GlobalParams::mem_ports; d++) {
				out << "\tDirection " << d << endl;
				for (int vc = 0; vc < GlobalParams::n_virtual_channels; vc++) {
					out << "\t\tVC" << vc << endl;
					for (int y = 0; y < GlobalParams::mesh_dim_y; y++) {
						for (int x = 0; x < GlobalParams::mesh_dim_x; x++) {
							if (vc == 0)
								out << "(" << noc->t[x][y]->r->buffer_out[d][0].Size() << ", " << noc->t[x][y]->r_req->buffer_out[d][1].Size() << ")" << " ";
						}
						out << endl;
					}
				}
			}

			out << "Buffer in state for router " << endl;
			for (int d = 0; d < DIRECTIONS + GlobalParams::mem_ports; d++) {
				out << "\tDirection " << d << endl;
				for (int vc = 0; vc < GlobalParams::n_virtual_channels; vc++) {
					out << "\t\tVC" << vc << endl;
					for (int y = 0; y < GlobalParams::mesh_dim_y; y++) {
						for (int x = 0; x < GlobalParams::mesh_dim_x; x++) {
							if (vc == 0)
								out << "(" << noc->t[x][y]->r->buffer[d][0].Size() << ", " << noc->t[x][y]->r_req->buffer[d][1].Size() << ")" << " ";
						}
						out << endl;
					}
				}
			}

			out << "Buffer out state for req router " << endl;
			for (int d = 0; d < DIRECTIONS + GlobalParams::mem_ports; d++) {
				out << "\tDirection " << d << endl;
				for (int vc = 0; vc < GlobalParams::n_virtual_channels; vc++) {
					out << "\t\tVC" << vc << endl;
					for (int y = 0; y < GlobalParams::mesh_dim_y; y++) {
						for (int x = 0; x < GlobalParams::mesh_dim_x; x++) {
							if (vc == 0)
								out << "(" << noc->t[x][y]->r_req->buffer_out[d][0].Size() << ", " << noc->t[x][y]->r_req->buffer_out[d][1].Size() << ")" << " ";
						}
						out << endl;
					}
				}
			}

			out << "Buffer in state for req router " << endl;
			for (int d = 0; d < DIRECTIONS + GlobalParams::mem_ports; d++) {
				out << "\tDirection " << d << endl;
				for (int vc = 0; vc < GlobalParams::n_virtual_channels; vc++) {
					out << "\t\tVC" << vc << endl;
					for (int y = 0; y < GlobalParams::mesh_dim_y; y++) {
						for (int x = 0; x < GlobalParams::mesh_dim_x; x++) {
							if (vc == 0)
								out << "(" << noc->t[x][y]->r_req->buffer[d][0].Size() << ", " << noc->t[x][y]->r_req->buffer[d][1].Size() << ")" << " ";
						}
						out << endl;
					}
				}
			}
		}
		//-----------------------------------------------------------------------
		// End Router buffers state for deadlock analysis
		//-----------------------------------------------------------------------

		//-----------------------------------------------------------------------
		// Start PE flit request latency
		//-----------------------------------------------------------------------
		for (int x = 0; x < GlobalParams::mesh_dim_x; x++) {
			for (int y = 0; y < GlobalParams::mesh_dim_y; y++) {
				for (auto it = noc->t[x][y]->pe[0]->flit_latency_x.begin(); it != noc->t[x][y]->pe[0]->flit_latency_x.end(); it++) {
					Coord dst_coord = id2Coord(it->second.dst_id);
					int latency = it->second.latency;
					if (it->second.is_back)
						f_latencies << "[x][" << x << "][" << y << "] -> " << "[" << dst_coord.x << "][" << dst_coord.y << "]: " << latency << endl;
				}
				for (auto it = noc->t[x][y]->pe[0]->flit_latency_y.begin(); it != noc->t[x][y]->pe[0]->flit_latency_y.end(); it++) {
					Coord dst_coord = id2Coord(it->second.dst_id);
					int latency = it->second.latency;
					if (it->second.is_back)
						f_latencies << "[y][" << x << "][" << y << "] -> " << "[" << dst_coord.x << "][" << dst_coord.y << "]: " << latency << endl;
				}
			}
		}

		f_latencies.close();
		//-----------------------------------------------------------------------
		// End PE flit request latency
		//-----------------------------------------------------------------------
		showPowerBreakDown(out);
		showPowerManagerStats(out);
  } // detailed stats

#ifdef DEBUG

    if (GlobalParams::topology == TOPOLOGY_MESH)
    {
	for (int y = 0; y < GlobalParams::mesh_dim_y; y++)
	    for (int x = 0; x < GlobalParams::mesh_dim_x; x++)
		out << "PE["<<x << "," << y<< "]" << noc->t[x][y]->pe->getQueueSize()<< ",";
    }
    else // other delta topologies
    {
	out << "Queue sizes: " ;
	for (int i=0;i<GlobalParams::n_delta_tiles;i++)
		out << "PE"<<i << ": " << noc->core[i]->pe->getQueueSize()<< ",";
	out << endl;
    }

    out << endl;
#endif

	//int total_cycles = GlobalParams::simulation_time - GlobalParams::stats_warm_up_time;
	out << "% Total received packets: " << getReceivedPackets() << endl;
	out << "% Total received flits: " << getReceivedFlits() << endl;
	out << "% Received/Ideal flits Ratio: " << getReceivedIdealFlitRatio() << endl;
	out << "% Average wireless utilization: " << getWirelessPackets()/(double)getReceivedPackets() << endl;
	out << "% Global average delay (cycles): " << getAverageDelay() << endl;
	out << "% Max delay (cycles): " << getMaxDelay() << endl;
	out << "% Network throughput (flits/cycle): " << getAggregatedThroughput() << endl;
	out << "% Average IP throughput (flits/cycle/IP): " << getThroughput() << endl;
	out << "% Total energy (J): " << getTotalPower() << endl;
	out << "% \tDynamic energy (J): " << getDynamicPower() << endl;
	out << "% \tStatic energy (J): " << getStaticPower() << endl;

	if (GlobalParams::show_buffer_stats)
		showBufferStats(out);

}

void GlobalStats::updatePowerBreakDown(map<string,double> &dst,PowerBreakdown* src)
{
	for (int i=0;i!=src->size;i++) {
		dst[src->breakdown[i].label]+=src->breakdown[i].value;
	}
}

void GlobalStats::showPowerManagerStats(std::ostream & out)
{
	std::streamsize p = out.precision();
	int total_cycles = sc_time_stamp().to_double() / GlobalParams::clock_period_ps - GlobalParams::reset_time;

	out.precision(4);

	out << "powermanager_stats_tx = [" << endl;
	out << "%\tFraction of: TX Transceiver off (TTXoff), AntennaBufferTX off (ABTXoff) " << endl;
	out << "%\tHUB\tTTXoff\tABTXoff\t" << endl;

	for (
		map<int, HubConfig>::iterator it = GlobalParams::hub_configuration.begin();
		it != GlobalParams::hub_configuration.end();
		++it
	) {
		int hub_id = it->first;

		map<int,Hub*>::const_iterator i = noc->hub.find(hub_id);
		Hub * h = i->second;

		out << "\t" << hub_id << "\t" << std::fixed << (double)h->total_ttxoff_cycles/total_cycles << "\t";

		int s = 0;
		for (map<int,int>::iterator i = h->abtxoff_cycles.begin(); i!=h->abtxoff_cycles.end();i++)
			s+=i->second;

		out << (double)s/h->abtxoff_cycles.size()/total_cycles << endl;
  }

	out << "];" << endl;

	out << "powermanager_stats_rx = [" << endl;
	out << "%\tFraction of: RX Transceiver off (TRXoff), AntennaBufferRX off (ABRXoff), BufferToTile off (BTToff) " << endl;
	out << "%\tHUB\tTRXoff\tABRXoff\tBTToff\t" << endl;

	for (
		map<int, HubConfig>::iterator it = GlobalParams::hub_configuration.begin();
		it != GlobalParams::hub_configuration.end();
		++it
	) {
		string bttoff_str;

		out.precision(4);

		int hub_id = it->first;

		map<int,Hub*>::const_iterator i = noc->hub.find(hub_id);
		Hub * h = i->second;

		out << "\t" << hub_id << "\t" << std::fixed << (double)h->total_sleep_cycles/total_cycles << "\t";

		int s = 0;
		for (
			map<int,int>::iterator i = h->buffer_rx_sleep_cycles.begin();
			i!=h->buffer_rx_sleep_cycles.end();
			i++
		)
			s+=i->second;

		out << (double)s/h->buffer_rx_sleep_cycles.size()/total_cycles << "\t";

		s = 0;
		for (
			map<int,int>::iterator i = h->buffer_to_tile_poweroff_cycles.begin();
			i!=h->buffer_to_tile_poweroff_cycles.end();
			i++
		) {
			double bttoff_fraction = i->second/(double)total_cycles;
			s+=i->second;
			if (bttoff_fraction<0.25)
				bttoff_str+=" ";
			else if (bttoff_fraction<0.5)
				bttoff_str+=".";
			else if (bttoff_fraction<0.75)
				bttoff_str+="o";
			else if (bttoff_fraction<0.90)
				bttoff_str+="O";
			else
				bttoff_str+="0";

		}
		out << (double)s/h->buffer_to_tile_poweroff_cycles.size()/total_cycles << "\t" << bttoff_str << endl;
  }

	out << "];" << endl;

	out.unsetf(std::ios::fixed);

	out.precision(p);

}

void GlobalStats::showPowerBreakDown(std::ostream & out)
{
    map<string,double> power_dynamic;
    map<string,double> power_static;

    if (GlobalParams::topology == TOPOLOGY_MESH) 
    {
	for (int y = 0; y < GlobalParams::mesh_dim_y; y++)
	    for (int x = 0; x < GlobalParams::mesh_dim_x; x++)
	    {
		updatePowerBreakDown(power_dynamic, noc->t[x][y]->r->power.getDynamicPowerBreakDown());
		updatePowerBreakDown(power_static, noc->t[x][y]->r->power.getStaticPowerBreakDown());
	    }
    }
    else // other delta topologies
    {
	for (int y = 0; y < GlobalParams::n_delta_tiles; y++)
	{
	    updatePowerBreakDown(power_dynamic, noc->core[y]->r->power.getDynamicPowerBreakDown());
	    updatePowerBreakDown(power_static, noc->core[y]->r->power.getStaticPowerBreakDown());
	}
    }

    for (map<int, HubConfig>::iterator it = GlobalParams::hub_configuration.begin();
	    it != GlobalParams::hub_configuration.end();
	    ++it)
    {
	int hub_id = it->first;

	map<int,Hub*>::const_iterator i = noc->hub.find(hub_id);
	Hub * h = i->second;

	updatePowerBreakDown(power_dynamic, 
		h->power.getDynamicPowerBreakDown());

	updatePowerBreakDown(power_static, 
		h->power.getStaticPowerBreakDown());
    }

    printMap("power_dynamic",power_dynamic,out);
    printMap("power_static",power_static,out);

}



void GlobalStats::showBufferStats(std::ostream & out)
{
  out << "Router id\tBuffer N\t\tBuffer E\t\tBuffer S\t\tBuffer W\t\tBuffer L" << endl;
  out << "         \tMean\tMax\tMean\tMax\tMean\tMax\tMean\tMax\tMean\tMax" << endl;
  
  if (GlobalParams::topology == TOPOLOGY_MESH) 
    {
    	for (int y = 0; y < GlobalParams::mesh_dim_y; y++)
    	for (int x = 0; x < GlobalParams::mesh_dim_x; x++)
      	{
			out << noc->t[x][y]->r->local_id;
			noc->t[x][y]->r->ShowBuffersStats(out);
			out << endl;
     	}
    }
    else // other delta topologies
    {
    	for (int y = 0; y < GlobalParams::n_delta_tiles; y++)
    	{
			out << noc->core[y]->r->local_id;
			noc->core[y]->r->ShowBuffersStats(out);
			out << endl;
     	}
    }

}

double GlobalStats::getReceivedIdealFlitRatio()
{
    int total_cycles;
    total_cycles= GlobalParams::simulation_time - GlobalParams::stats_warm_up_time;
    double ratio;
    if (GlobalParams::topology == TOPOLOGY_MESH) 
    {
	ratio = getReceivedFlits() /(GlobalParams::packet_injection_rate * (GlobalParams::min_packet_size +
		    GlobalParams::max_packet_size)/2 * total_cycles * GlobalParams::mesh_dim_y * GlobalParams::mesh_dim_x);
    }
    else // other delta topologies
    {
	ratio = getReceivedFlits() /(GlobalParams::packet_injection_rate * (GlobalParams::min_packet_size +
		    GlobalParams::max_packet_size)/2 * total_cycles * GlobalParams::n_delta_tiles);
    }
    return ratio;
}
