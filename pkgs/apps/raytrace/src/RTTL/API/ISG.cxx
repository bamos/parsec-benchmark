#include "RTTL/API/ISG.hxx"
#include "LRT/include/lrt.h"

namespace ISG {
  int current_timestamp = 0;

  /*! the default world to use if no world was specified explicitly */
  World *World::m_default;
};


void rtShow(node_t node) 
{
  World *world = World::getDefaultWorld();
  world->rootNode.push_back((RootNode*)node);
};

