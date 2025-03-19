#ifndef __NOXIMROUTING_YX_H__
#define __NOXIMROUTING_YX_H__

#include "../Router.h"
#include "RoutingAlgorithm.h"
#include "RoutingAlgorithms.h"

using namespace std;

class Routing_YX : RoutingAlgorithm {
public:
  vector<int> route(Router *router, const RouteData &routeData);

  static Routing_YX *getInstance();

private:
  Routing_YX(){};
  ~Routing_YX(){};

  static Routing_YX *routing_YX;
  static RoutingAlgorithmsRegister routingAlgorithmsRegister;
};

#endif
