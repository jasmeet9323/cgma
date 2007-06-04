//-------------------------------------------------------------------------
// Filename      : SimplifyTool.cpp
//
// Purpose       :
//
// Simplify {Volume|Surface} <Range> [Angle <Value>]
//     [Respect {Surface <Range> | Curve <Range> | imprint}]
//     [Preview]
//
// Special Notes :
//
// Creator       : Sam Showman
//
// Creation Date : 11/07/2005
//-------------------------------------------------------------------------

#include "SimplifyTool.hpp"
#include "RefVolume.hpp"
#include "DLIList.hpp"
#include "ModelEntity.hpp"
#include "RefFace.hpp"
#include "RefEdge.hpp"
#include "RefVertex.hpp"
#include "CompositeTool.hpp"
#include "GfxDebug.hpp"
#include "ProgressTool.hpp"
#include "AppUtil.hpp"
#include "GeometryFeatureTool.hpp"
#include "GMem.hpp"
#include "GeometryQueryEngine.hpp"

SimplifyTool::SimplifyTool()
{
}

SimplifyTool::~SimplifyTool()
{
}

CubitStatus SimplifyTool::simplify_volumes(DLIList<RefVolume*> ref_volume_list,
                                           double surf_angle_in,
                                           DLIList<RefFace*> respect_face_list,
                                           DLIList<RefEdge*> respect_edge_list,
                                           CubitBoolean respect_rounds,
                                           CubitBoolean respect_imprints,
                                           CubitBoolean preview)
{
    ref_volume_list.uniquify_unordered();
    for(int i = ref_volume_list.size();i--;)
        simplify_volume(
        ref_volume_list.get_and_step(),
        surf_angle_in,
        respect_face_list,
        respect_edge_list,
        respect_rounds,
        respect_imprints,
        preview);

    if(preview)
        GfxDebug::flush();

    return CUBIT_SUCCESS;
}

CubitStatus SimplifyTool::simplify_surfaces(DLIList<RefFace*> ref_face_list, 
                                            double angle_in,
                                            DLIList<RefFace*> respect_face_list,
                                            DLIList<RefEdge*> respect_edge_list,
                                            CubitBoolean respect_rounds,
                                            CubitBoolean respect_imprints,
                                            CubitBoolean preview)
{
    CubitStatus status = CUBIT_FAILURE;
    ref_face_list.uniquify_unordered();
    while(ref_face_list.size())
    {
        DLIList<RefFace*> ref_faces_in_volume;
        ref_face_list.reset();
        RefFace* cur_face = ref_face_list.get_and_step();
        RefVolume* cur_vol = cur_face->ref_volume();
        ref_faces_in_volume.append(cur_face);
        for(int i =1;i<ref_face_list.size();i++)
        {
            RefFace* face = ref_face_list.get_and_step();
            if(face->ref_volume() == cur_vol)
                ref_faces_in_volume.append(face);
        }

        if(ref_faces_in_volume.size()>1)
        {
            status = simplify_surfaces_in_volume(
                ref_faces_in_volume,
                angle_in,
                respect_face_list,
                respect_edge_list,
                respect_rounds,
                respect_imprints,
                preview);
        }
        ref_face_list -= ref_faces_in_volume;
    }

    if(preview)
        GfxDebug::flush();

    return CUBIT_SUCCESS;
}

CubitStatus SimplifyTool::simplify_volume(RefVolume* ref_volume, 
                                          double angle_in,
                                          DLIList<RefFace*> respect_face_list,
                                          DLIList<RefEdge*> respect_edge_list,
                                          CubitBoolean respect_rounds,
                                          CubitBoolean respect_imprints,
                                          CubitBoolean preview)
{
    DLIList<RefFace*> ref_face_list;
    ref_volume->ref_faces(ref_face_list);
    return simplify_surfaces_in_volume(
        ref_face_list,
        angle_in,
        respect_face_list,
        respect_edge_list,
        respect_rounds,
        respect_imprints,
        preview);
}

// main funciton
// all of the input faces must be from the same volume
CubitStatus SimplifyTool::simplify_surfaces_in_volume(
    DLIList<RefFace*> ref_face_list, 
    double angle_in,
    DLIList<RefFace*> respect_face_list,
    DLIList<RefEdge*> respect_edge_list,
    CubitBoolean respect_rounds,
    CubitBoolean respect_imprints,
    CubitBoolean preview)
{
    if(ref_face_list.size()==0)
    {
        PRINT_ERROR("No surfaces specified for simplification\n");
        return CUBIT_FAILURE;
    }
    else if(ref_face_list.size() == 1)
    {
        PRINT_ERROR("Only one surface specified for simplification\n");
        return CUBIT_FAILURE;
    }

    DLIList<RefFace*> seed_faces;
    DLIList<RefEdge*> preview_edges;
    RefVolume* ref_volume = ref_face_list.get()->ref_volume();
    DLIList<RefEdge*> preview_removed;

    if(preview)
        ref_volume->ref_edges(preview_edges);

    int j,k;

    int new_face_count = 0;
    int combined_face_count = 0;
    ProgressTool *prog_ptr = 0;
    if(ref_face_list.size() > 100 )
    {
        char title[200];
        if(preview)
            sprintf(title, "Previewing Volume %d",ref_volume->id());
        else
            sprintf(title, "Simplifying Surfaces in Volume %d",ref_volume->id());

        prog_ptr = AppUtil::instance()->progress_tool();
        assert(prog_ptr != NULL);
        prog_ptr->start(0,100, title);
    }
    int start_face_count = ref_face_list.size();
    while(ref_face_list.size())
    {
        DLIList<RefFace*> composite_faces;
        seed_faces.append_unique(ref_face_list.pop());

        for ( j = ref_face_list.size(); j--; )
            ref_face_list.get_and_step()->marked( CUBIT_FALSE );

        while(seed_faces.size())
        {
            RefFace *seed_ref_face = seed_faces.pop();
            seed_ref_face->marked(CUBIT_TRUE);

            composite_faces.append(seed_ref_face);

            // Get the edges
            DLIList<RefEdge*> ref_edge_list;
            seed_ref_face->ref_edges( ref_edge_list );
            RefEdge *ref_edge_ptr;
            RefFace *ref_face_ptr;
            for( k = ref_edge_list.size(); k--; )
            {
                ref_edge_ptr = ref_edge_list.get_and_step();

                // Don't propagate across merged entities (you can't create composites
                // across merged volumes)
                if( ref_edge_ptr->bridge_manager()->number_of_bridges() > 1)
                    continue;

                // Don't go propagate across surface splits if the user asks for it
                GeometryFeatureTool* gft = GeometryFeatureTool::instance();
                if( respect_imprints &&
                    gft->feature_type(ref_edge_ptr) == GeometryFeatureEngine::FEATURE_IMPRINT)
                    continue;

                // Don't cross a curve if we want it respected
                if(respect_edge_list.is_in_list(ref_edge_ptr))
                    continue;

                DLIList<RefFace*> attached_ref_faces;
                ref_edge_ptr->ref_faces( attached_ref_faces );

                attached_ref_faces.remove(seed_ref_face);
                ref_face_ptr = attached_ref_faces.size()!=0?attached_ref_faces.get():0;

                // keep the face if we want it respected
                if(attached_ref_faces.size() == 1 &&
                    respect_face_list.is_in_list(attached_ref_faces.get()))
                    continue;

                // Don't consider ref_faces that are already in the list
                if( attached_ref_faces.size() == 1 &&
                    !ref_face_ptr->marked() &&
                    (!respect_rounds || 
                    ((gft->feature_type(ref_face_ptr) != GeometryFeatureEngine::FEATURE_ROUND && 
                    gft->feature_type(seed_ref_face) != GeometryFeatureEngine::FEATURE_ROUND) ||
                    (gft->feature_type(ref_face_ptr) == GeometryFeatureEngine::FEATURE_ROUND && 
                    gft->feature_type(seed_ref_face) == GeometryFeatureEngine::FEATURE_ROUND))))
                {
                    DLIList<RefVolume*> ref_volumes;
                    ref_face_ptr->ref_volumes( ref_volumes );
                    if( !ref_volumes.size() || ref_volumes.size()==1 )
                    {
                        // Only add the ref_face if it meets the feature angle criteria
                        if(composite_surfaces(seed_ref_face,ref_face_ptr,angle_in))
                        {
                            ref_face_ptr->marked( CUBIT_TRUE );
                            seed_faces.append(ref_face_ptr);
                            composite_faces.append(ref_face_ptr);
                        }
                    }
                }
            }
        }
        composite_faces.uniquify_unordered();
        ref_face_list -= composite_faces;

        if(!preview &&
            composite_faces.size()>1)
        {
            DLIList<RefEdge*> result_edges;
            DLIList<RefFace*> result_faces;
            CubitStatus new_face = CompositeTool::instance()->composite(
                composite_faces,
                result_faces,
                result_edges);

            combined_face_count +=composite_faces.size();
            for(int m = result_faces.size();m--;)
                result_faces.get_and_step()->marked(CUBIT_TRUE);

            new_face_count+=result_faces.size();
        }
        else if(preview)
        {
            int face_count = composite_faces.size();
            for(int i =0;i<face_count;i++)
            {
                RefFace* cur_comp_face = composite_faces[i];
                DLIList<RefEdge*> refedges; 
                for(int j =0;j<face_count;j++)
                {
                    if(i==j) continue;
                    composite_faces[j]->ref_edges(refedges);
                }

                refedges.uniquify_unordered();

                DLIList<RefEdge*> temp_refedges; 
                cur_comp_face->ref_edges(temp_refedges);
                refedges.intersect_unordered(temp_refedges);
                preview_removed+=refedges;
            }
        }

        if(prog_ptr)
        {
            double frac = 1.0-(double)ref_face_list.size()/(double)start_face_count;
            prog_ptr->percent(frac);
        }
    }

    if(prog_ptr)
    {
        prog_ptr->end();
        prog_ptr = 0;
    }

    if(preview)
    {
        preview_edges -=preview_removed;
        for(int c = preview_edges.size();c--;)
            GfxDebug::draw_ref_edge(preview_edges.get_and_step(),7);
    }
    else if(combined_face_count>new_face_count)
    {
        PRINT_INFO("Simplified %d surfaces into %d surfaces\n",
            combined_face_count,
            new_face_count);
    }

    return CUBIT_SUCCESS;
}

CubitBoolean SimplifyTool::composite_surfaces(
    RefFace* seed_ref_face,
    RefFace* ref_face_ptr,
    double angle_in)
{
    double angle;
    CubitVector pos, norm1, norm2;

    // Only add the ref_face if it meets the feature angle criteria
    double weight = 0.0;
    weighted_average_normal(seed_ref_face,norm1,weight);
    weighted_average_normal(ref_face_ptr,norm2,weight);

    angle = norm1.interior_angle( norm2 );

    if( angle < angle_in)
        return CUBIT_TRUE;

    return CUBIT_FALSE;
}

void SimplifyTool::process_rounds(RefVolume* ref_volume,
                                  double min_radius, 
                                  double max_radius)
{
    DLIList<RefEdge*> edges;
    ref_volume->ref_edges(edges);

    DLIList<RefFace*> rounds;

    // a edge must have curvature within the tolerance
    for(int j = edges.size();j--;)
    {
        RefEdge* edge = edges.get_and_step();
        CubitVector loc,tan,curv;
        edge->closest_point(edge->curve_center(),loc,&tan,&curv);

        double curv_mag = curv.length();

        if(curv_mag > GEOMETRY_RESABS &&
            1.0/curv_mag >= min_radius &&
            1.0/curv_mag <= max_radius)
        {
            DLIList<RefFace*> new_rounds;
            edge->ref_faces(new_rounds);
            rounds+=new_rounds;
        }

    }

    rounds.uniquify_unordered();
    for(int i = rounds.size();i--;)
    {
        // cull any flat surfaces
        RefFace* curr_face = rounds.get_and_step();

        double curve_0,curve_1;
        curr_face->get_principal_curvatures(curr_face->center_point(),curve_0,curve_1);
        curve_0 = fabs(curve_0);
        curve_1 = fabs(curve_1);

        if((curve_0 > GEOMETRY_RESABS &&
            1.0/curve_0 >= min_radius &&
            1.0/curve_0 <= max_radius) ||
            (curve_1 > GEOMETRY_RESABS &&
            1.0/curve_1 >= min_radius &&
            1.0/curve_1 <= max_radius))     
        {
            GfxDebug::highlight_ref_face(curr_face);
        }
    }
}

CubitStatus
SimplifyTool::weighted_average_normal(RefFace* ref_face,
                                      CubitVector &normal, 
                                      double &weight )
{
    int i;
    int num_tris, num_pnts, num_facets;
    GMem g_mem;
    unsigned short norm_tol = 30;
    double dist_tol = -1.0;

    ref_face->get_geometry_query_engine()->
        get_graphics(ref_face->get_surface_ptr(),num_tris, num_pnts, num_facets,
        &g_mem, norm_tol, dist_tol );

    if(num_tris < 1)
    {
        // Decrease tolerance and try again (we can get this for small features)
        norm_tol /= 2;
        ref_face->get_geometry_query_engine()->
            get_graphics(ref_face->get_surface_ptr(),num_tris, num_pnts, num_facets,
            &g_mem, norm_tol, dist_tol );
    }

    if(num_tris < 1)
    {
        // Lets give up 
        PRINT_ERROR( "Unable to find average normal of a surface\n" );
        return CUBIT_FAILURE;
    }

    // Initialize
    weight = 0.0;
    normal.set( 0.0, 0.0, 0.0 );

    // Loop through the triangles
    double tri_weight, A, B, C;
    GPoint p[3];
    GPoint* plist = g_mem.point_list();
    int* facet_list = g_mem.facet_list();
    int c = 0;
    for( i=0; i<num_tris; i++ )
    {
        p[0] = plist[facet_list[++c]];
        p[2] = plist[facet_list[++c]];
        p[1] = plist[facet_list[++c]]; 
        c++;

        // Get centroid
        CubitVector p1( p[0].x, p[0].y, p[0].z );
        CubitVector p2( p[2].x, p[2].y, p[2].z );
        CubitVector p3( p[1].x, p[1].y, p[1].z );

        CubitVector center = (p1 + p2 + p3)/3.0;

        CubitVector norm(ref_face->normal_at(center));

        // Get triangle area
        A = p1.y() * p2.z() + p1.z() * p3.y() + p2.y() * p3.z() -
            p2.z() * p3.y() - p1.y() * p3.z() - p1.z() * p2.y();

        B = p1.z() * p2.x() + p1.x() * p3.z() + p2.z() * p3.x() -
            p2.x() * p3.z() - p1.z() * p3.x() - p1.x() * p2.z();

        C = p1.x() * p2.y() + p1.y() * p3.x() + p2.x() * p3.y() -
            p2.y() * p3.x() - p1.x() * p3.y() - p1.y() * p2.x();

        //Note: triangle area = 0.5*(sqrt(A*A+B*B+C*C));

        tri_weight = 0.5*(A*A+B*B+C*C);

        normal += tri_weight * norm;

        weight += tri_weight;

    }

    normal.normalize();

    return CUBIT_SUCCESS;
}
