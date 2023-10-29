/*
 * Noxim - the NoC Simulator
 *
 * (C) 2005-2018 by the University of Catania
 * For the complete list of authors refer to file ../doc/AUTHORS.txt
 * For the license applied to these sources refer to file ../doc/LICENSE.txt
 *
 * This file contains the declaration of the tile
 */

#ifndef __NOXIMTILE_H__
#define __NOXIMTILE_H__

#include <systemc.h>
#include "Router.h"
#include "ProcessingElement.h"
using namespace std;

SC_MODULE(Tile)
{
    SC_HAS_PROCESS(Tile);

    // I/O Ports
    sc_in_clk clock;		                // The input clock for the tile
    sc_in <bool> reset;	                        // The reset signal for the tile

    int local_id; // Unique ID

    sc_in <Flit> flit_rx[DIRECTIONS];	// The input channels
    sc_in <bool> req_rx[DIRECTIONS];	        // The requests associated with the input channels
    sc_out <bool> ack_rx[DIRECTIONS];	        // The outgoing ack signals associated with the input channels
    sc_out <TBufferFullStatus> buffer_full_status_rx[DIRECTIONS];

    sc_out <Flit> flit_tx[DIRECTIONS];	// The output channels
    sc_out <bool> req_tx[DIRECTIONS];	        // The requests associated with the output channels
    sc_in <bool> ack_tx[DIRECTIONS];	        // The outgoing ack signals associated with the output channels
    sc_in <TBufferFullStatus> buffer_full_status_tx[DIRECTIONS];

    // hub specific ports
    sc_in <Flit> hub_flit_rx;	// The input channels
    sc_in <bool> hub_req_rx;	        // The requests associated with the input channels
    sc_out <bool> hub_ack_rx;	        // The outgoing ack signals associated with the input channels
    sc_out <TBufferFullStatus> hub_buffer_full_status_rx;

    sc_out <Flit> hub_flit_tx;	// The output channels
    sc_out <bool> hub_req_tx;	        // The requests associated with the output channels
    sc_in <bool> hub_ack_tx;	        // The outgoing ack signals associated with the output channels
    sc_in <TBufferFullStatus> hub_buffer_full_status_tx;	


    // NoP related I/O and signals
    sc_out <int> free_slots[DIRECTIONS];
    sc_in <int> free_slots_neighbor[DIRECTIONS];
    sc_out < NoP_data > NoP_data_out[DIRECTIONS];
    sc_in < NoP_data > NoP_data_in[DIRECTIONS];

    sc_signal <int> free_slots_local[DIRECTIONS];
    sc_signal <int> free_slots_neighbor_local[DIRECTIONS];

    // Signals required for Router-PE connection
    sc_signal <Flit> flit_rx_local[DIRECTIONS];	
    sc_signal <bool> req_rx_local[DIRECTIONS];     
    sc_signal <bool> ack_rx_local[DIRECTIONS];
    sc_signal <TBufferFullStatus> buffer_full_status_rx_local[DIRECTIONS];

    sc_signal <Flit> flit_tx_local[DIRECTIONS];
    sc_signal <bool> req_tx_local[DIRECTIONS];
    sc_signal <bool> ack_tx_local[DIRECTIONS];
    sc_signal <TBufferFullStatus> buffer_full_status_tx_local[DIRECTIONS];


    // Instances
    Router *r;		                // Router instance
    ProcessingElement *pe[DIRECTIONS];	                // Processing Element instance

    // Constructor

    Tile(sc_module_name nm, int id): sc_module(nm) {
		local_id = id;
		
		// Router pin assignments
		r = new Router("Router");

		r->clock(clock);
		r->reset(reset);

		for (int i = 0; i < DIRECTIONS; i++) {
			r->flit_rx[i] (flit_rx[i]);
			r->req_rx[i] (req_rx[i]);
			r->ack_rx[i] (ack_rx[i]);
			r->buffer_full_status_rx[i](buffer_full_status_rx[i]);

			r->flit_tx[i] (flit_tx[i]);
			r->req_tx[i] (req_tx[i]);
			r->ack_tx[i] (ack_tx[i]);
			r->buffer_full_status_tx[i](buffer_full_status_tx[i]);

			r->free_slots[i] (free_slots[i]);
			r->free_slots_neighbor[i] (free_slots_neighbor[i]);

			// NoP 
			r->NoP_data_out[i] (NoP_data_out[i]);
			r->NoP_data_in[i] (NoP_data_in[i]);
		}
		
		// local
		for (int i = 0; i < DIRECTIONS; i++)
		{
			r->flit_rx[DIRECTIONS+i] (flit_tx_local[i]);
			r->req_rx[DIRECTIONS+i] (req_tx_local[i]);
			r->ack_rx[DIRECTIONS+i] (ack_tx_local[i]);
			r->buffer_full_status_rx[DIRECTIONS+i] (buffer_full_status_tx_local[i]);

			r->flit_tx[DIRECTIONS+i] (flit_rx_local[i]);
			r->req_tx[DIRECTIONS+i] (req_rx_local[i]);
			r->ack_tx[DIRECTIONS+i] (ack_rx_local[i]);
			r->buffer_full_status_tx[DIRECTIONS+i] (buffer_full_status_rx_local[i]);

			r->free_slots[DIRECTIONS+i] (free_slots_local[i]);
			r->free_slots_neighbor[DIRECTIONS+i] (free_slots_neighbor_local[i]);
		}

		// hub related
		r->flit_rx[DIRECTION_HUB] (hub_flit_rx);
		r->req_rx[DIRECTION_HUB] (hub_req_rx);
		r->ack_rx[DIRECTION_HUB] (hub_ack_rx);
		r->buffer_full_status_rx[DIRECTION_HUB] (hub_buffer_full_status_rx);

		r->flit_tx[DIRECTION_HUB] (hub_flit_tx);
		r->req_tx[DIRECTION_HUB] (hub_req_tx);
		r->ack_tx[DIRECTION_HUB] (hub_ack_tx);
		r->buffer_full_status_tx[DIRECTION_HUB] (hub_buffer_full_status_tx);

		// Processing Element pin assignments
		pe[DIRECTION_NORTH] = new ProcessingElement("ProcessingElementNorth");
		pe[DIRECTION_EAST]  = new ProcessingElement("ProcessingElementEast");
		pe[DIRECTION_SOUTH] = new ProcessingElement("ProcessingElementSouth");
		pe[DIRECTION_WEST]  = new ProcessingElement("ProcessingElementWest");
		for (int i = 0; i < DIRECTIONS; i++)
		{
			// pe[i] = new ProcessingElement(pe_name[i]);
			pe[i]->clock(clock);
			pe[i]->reset(reset);

			pe[i]->flit_rx(flit_rx_local[i]);
			pe[i]->req_rx(req_rx_local[i]);
			pe[i]->ack_rx(ack_rx_local[i]);
			pe[i]->buffer_full_status_rx(buffer_full_status_rx_local[i]);
			

			pe[i]->flit_tx(flit_tx_local[i]);
			pe[i]->req_tx(req_tx_local[i]);
			pe[i]->ack_tx(ack_tx_local[i]);
			pe[i]->buffer_full_status_tx(buffer_full_status_tx_local[i]);

			// NoP
			//
			pe[i]->free_slots_neighbor(free_slots_neighbor_local[i]);
		}
    }
};

#endif
