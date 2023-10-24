#ifndef __NOXIMROUTING_DOR_H__
#define __NOXIMROUTING_DOR_H__

#include "RoutingAlgorithm.h"
#include "RoutingAlgorithms.h"
#include "../Router.h"

using namespace std;

class Routing_DOR : RoutingAlgorithm {
	public:
		vector<int> route(Router * router, const RouteData & routeData);

		static Routing_DOR * getInstance();

	private:
		Routing_DOR(){};
		~Routing_DOR(){};

		static Routing_DOR * routing_DOR;
		static RoutingAlgorithmsRegister routingAlgorithmsRegister;
};

#endif
