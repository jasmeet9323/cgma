//- Class:          CAPartitionVG
//- Owner:          Tim Tautges
//- Description:    Cubit attribute for partition virtual geometry
//- Checked by:
//- Version:
//-
//- This attribute holds the information necessary to re-partition
//- an entity into several parts.  This attribute does not store the
//- data necessary to create the virtual entities that partitions this
//- entity; those data are stored on a CAVirtualVG attribute.
//-
//- This attribute stores the following data:
//- numPC, numPS: the number of partition curves, surfaces this entity
//-     will be split into
//- vgUIDs[0..3*numPC-1]: uids of start and end vertices, and of partition
//-     curve formed from those
//- vgUIDs[3*numPC..(end)]: for each partition surface, the uids of the 
//-     bounding curves (some of them may be virtual), then the uid of the
//-     partition surface itself
//- numBdyCurves[numPS]: number of bounding curves for each surface
//-
//- For partition surfaces, this attribute actuates by:
//- . actuating partition VG attributes on next-lower order entities
//- . actuating virtual VG attributes on self
//- . gathering up the bounding curve uid's & edges, removing those
//-   already bounding the (non-partitioned) entity
//- . using these to partition the entity
//-

#ifndef CA_PARTITION_VG_HPP
#define CA_PARTITION_VG_HPP

#include "CubitAttrib.hpp"
#include "DLIList.hpp"
#include "CADefines.hpp"

class Point;
class RefEntity;
class CubitSimpleAttrib;
class RefEntity;
class PartitionCurve;
class PartitionSurface;
class CubitVector;
class RefEdge;
class RefVertex;
class RefEdge;
class BasicTopologyEntity;
class ToolDataUser;

class CAPartitionVG: public CubitAttrib
{

private:
  int numPC, numPS;
    //- the number of partition curves and surfaces on this entity

  DLIList<int> vgUIDs;
    //- unique ids of various entities

  DLIList<int> numBdyCurves;
    //- for each partition surface, the number of bounding curves

#ifdef BOYD15
  void add_pcurve(PartitionCurve *pcurve);
    //- adds data for this vpoint to this CA
#endif

#ifdef BOYD15
  void add_psurface(PartitionSurface *psurface);
    //- adds data for this vcurve to this CA
#endif

#ifdef BOYD15
  RefEdge *find_edge(RefVertex *start_vert, 
                     RefVertex *end_vert, DLIList<RefEdge*> &new_edges);
    //- given a start and end vertex and a list of edges, return the edge in the list
    //- bounded by those two vertices
#endif
    
#ifdef BOYD15
  BasicTopologyEntity* find_related( DLIList<ToolDataUser*>& list );
#endif

public:
  CAPartitionVG(RefEntity* );

  CAPartitionVG(RefEntity*, CubitSimpleAttrib*);
    //- construct a CAPVG from a simple attribute

  virtual ~CAPartitionVG() {};

  //HEADER- RTTI and safe casting functions.
  virtual const type_info& entity_type_info() const { return typeid(*this);}
  //R- The geometric modeler type
  //- This function returns the type of the geometric modeler.

  CubitStatus actuate();
  
  CubitStatus update();

  CubitStatus reset();
    //- reset info; called from CAU and also from update!

  CubitSimpleAttrib* cubit_simple_attrib();
  
  CubitSimpleAttrib* cubit_simple_attrib(CubitString);
  
  int int_attrib_type() {return CA_PARTITION_VG;}
    //- returns the enumerated attribute type

};

CubitAttrib* CAPartitionVG_creator(RefEntity* entity, CubitSimpleAttrib *p_csa);

#endif
