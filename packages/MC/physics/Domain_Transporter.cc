//----------------------------------*-C++-*----------------------------------//
/*!
 * \file   mc/Domain_Transporter.cc
 * \author Thomas M. Evans
 * \date   Mon May 12 12:02:13 2014
 * \brief  Domain_Transporter member definitions.
 * \note   Copyright (C) 2014 Oak Ridge National Laboratory, UT-Battelle, LLC.
 */
//---------------------------------------------------------------------------//

#include <hpx/parallel/execution_policy.hpp>
#include <hpx/include/parallel_algorithm.hpp>

#include <boost/range/irange.hpp>

#include <cmath>
#include <algorithm>
#include <atomic>

#include "harness/DBC.hh"
#include "harness/Diagnostics.hh"
#include "geometry/Definitions.hh"
#include "Definitions.hh"
#include "Domain_Transporter.hh"

namespace profugus
{

//---------------------------------------------------------------------------//
// CONSTRUCTOR
//---------------------------------------------------------------------------//
/*!
 * \brief Constructor.
 */
Domain_Transporter::Domain_Transporter()
{
}

//---------------------------------------------------------------------------//
// PUBLIC FUNCTIONS
//---------------------------------------------------------------------------//
/*!
 * \brief Set the geometry and physics classes.
 *
 * \param geometry
 * \param physics
 */
void Domain_Transporter::set(SP_Geometry geometry,
                             SP_Physics  physics)
{
    REQUIRE(geometry);
    REQUIRE(physics);

    d_geometry = geometry;
    d_physics  = physics;

    if (d_var_reduction)
    {
        d_var_reduction->set(d_geometry);
        d_var_reduction->set(d_physics);
    }
}

//---------------------------------------------------------------------------//
/*!
 * \brief Set the variance reduction.
 *
 * \param reduction
 */
void Domain_Transporter::set(SP_Variance_Reduction reduction)
{
    REQUIRE(reduction);
    d_var_reduction = reduction;

    if (d_geometry)
        d_var_reduction->set(d_geometry);
    if (d_physics)
        d_var_reduction->set(d_physics);
}

//---------------------------------------------------------------------------//
/*!
 * \brief Set regular tallies.
 *
 * \param tallies
 */
void Domain_Transporter::set(SP_Tallier tallies)
{
    REQUIRE(tallies);
    d_tallier = tallies;
    ENSURE(d_tallier);
}

//---------------------------------------------------------------------------//
/*!
 * \brief Set regular source.
 *
 * \param source
 */
void Domain_Transporter::set(SP_Source source)
{
    REQUIRE(source);
    d_source = source;
    ENSURE(d_source);
}

//---------------------------------------------------------------------------//
/*!
 * \brief Transport a particle through the domain.
 *
 * \param particle
 * \param bank
 */
void Domain_Transporter::transport( 
    std::vector<Particle_t>& particles,
    std::vector<std::pair<std::size_t,events::Event> >& events, 
    std::vector<Bank_t>& banks ) const
{
    REQUIRE(d_geometry);
    REQUIRE(d_physics);
    REQUIRE(d_var_reduction);
    REQUIRE(d_tallier);

    // Get the number of particles
    int batch_size = particles.size();
    CHECK( batch_size > 0 );
    auto range = boost::irange( 0, batch_size );

    // initialize the source run count.
    std::atomic<def::size_type> num_run_check( batch_size );
    std::atomic<def::size_type> num_run( batch_size );
    def::size_type num_to_run = d_source->num_to_transport();

    // initialize the particle distances.
    auto init_func = [&]( const int lid )
		     { particles[lid].set_dist_mfp( 
			     -std::log(particles[lid].rng().ran()) ); };
    auto init_task = hpx::parallel::for_each( 
	hpx::parallel::parallel_task_execution_policy(),
	std::begin(range),
	std::end(range),
	init_func );
    init_task.wait();

    // Create an event sorting and searching predicate.
    auto event_pred = []( const std::pair<std::size_t,events::Event>& e1,
			  const std::pair<std::size_t,events::Event>& e2 )
		      { return e1.second < e2.second; };

    // Create a function for getting the next event.
    auto get_next_event_func = 
	[&]( std::pair<std::size_t,events::Event>& e )
	{ 
	    get_next_event( particles[e.first], e.second );
	};

    // Create a function for processing a boundary.
    auto process_boundary_func =
	[&]( std::pair<std::size_t,events::Event>& e )
	{ 
	    process_boundary( particles[e.first], e.second, banks[e.first] );
	};

    // Create a function for processing a collision.
    auto process_collision_func = 
	[&]( std::pair<std::size_t,events::Event>& e )
	{ 
	    process_collision( particles[e.first], e.second, banks[e.first] );
	    particles[e.first].set_dist_mfp( -std::log(particles[e.first].rng().ran()) );
	};

    // Create a function for tallying particles after they have died and
    // spawning a new particle if necesseary.
    auto end_history_func = [&,this]( std::pair<std::size_t,events::Event>& e )
			    { 
				this->d_tallier->end_history( particles[e.first] );
				int count = num_run_check.fetch_add(1);
				if ( count < num_to_run )
				{
				    num_run.fetch_add(1);
				    particles[e.first] = 
					this->d_source->get_particle(count);
				    e.second = events::BORN;
				}
				else
				{
				    e.second = events::END_EVENT;
				}
			    };

    // Create a functions for find the range of collision events.
    auto find_collision_begin = []( const std::pair<std::size_t,events::Event>& e )
				{ return e.second == events::COLLISION; };
    auto find_collision_end = []( const std::pair<std::size_t,events::Event>& e )
				{ return e.second > events::COLLISION; };

    // Create a functions for find the range of boundary events.
    auto find_boundary_begin = []( const std::pair<std::size_t,events::Event>& e )
				{ return e.second == events::BOUNDARY; };
    auto find_boundary_end = []( const std::pair<std::size_t,events::Event>& e )
				{ return e.second > events::BOUNDARY; };

    // Create a functions for find the range of end history events.
    auto find_end_history_begin = []( const std::pair<std::size_t,events::Event>& e )
				{ return e.second > events::STILL_ALIVE; };
    auto find_end_history_end = []( const std::pair<std::size_t,events::Event>& e )
				{ return e.second == events::END_EVENT; };

    // Create event pairs that we will be looking for.
    std::pair<std::size_t,events::Event> alive_pair( 0, events::STILL_ALIVE );
    std::pair<std::size_t,events::Event> collision_pair( 0, events::COLLISION );
    std::pair<std::size_t,events::Event> boundary_pair( 0, events::BOUNDARY );
    std::pair<std::size_t,events::Event> end_pair( 0, events::END_EVENT );

    // Create an iterator to the end of the active particles. The whole array
    // is alive to start.
    std::vector<std::pair<std::size_t,events::Event> >::iterator alive_end
	= events.end();
    std::vector<std::pair<std::size_t,events::Event> >::iterator dead_end
	= events.end();

    // create a range for particles that have just been killed.
    std::pair<std::vector<std::pair<std::size_t,events::Event> >::iterator,
	      std::vector<std::pair<std::size_t,events::Event> >::iterator>
	end_history_range;

    // Run the kernels until the first particle in the list is no longer
    // alive. The sorting will make sure that when this is the case, all
    // particles have been finished.
    int counter = 0;
    while ( events[0].second < events::STILL_ALIVE )
    {
        // process particles through internal boundaries until it hits a
        // collision site or leaves the domain
        //
        // >>>>>>> IMPORTANT <<<<<<<<
        // it is absolutely essential to check the geometry distance first,
        // before the distance to boundary mesh; this naturally takes care of
        // coincident boundary mesh and problem geometry surfaces (you may end
        // up with a small, negative distance to boundary mesh on the
        // subsequent step, but who cares)
        // >>>>>>>>>>>>-<<<<<<<<<<<<<
	auto get_next_event_task = hpx::parallel::for_each( 
	    hpx::parallel::parallel_task_execution_policy(),
	    events.begin(),
	    alive_end,
	    get_next_event_func );
	get_next_event_task.wait();
	
	// Sort the events so we can get equal ranges of particles.
	std::sort( events.begin(), dead_end, event_pred );
	
	// // Get the range of particles that have had a collision.
	// auto collision_range_begin =
	//     hpx::parallel::find_if(
	// 	hpx::parallel_task_execution_policy(),
	// 	events.begin(),
	// 	alive_end,
	// 	find_collision_begin );
	// auto collision_range_end =
	//     hpx::parallel::find_if(
	// 	hpx::parallel_task_execution_policy(),
	// 	events.end(),
	// 	alive_end,
	// 	find_collision_end );

	auto collision_range = std::equal_range( events.begin(),
						 dead_end,
						 collision_pair,
						 event_pred );

	// Get the range of particles that have hit a boundary.
	auto boundary_range = std::equal_range( events.begin(),
						dead_end,
						boundary_pair,
						event_pred );

	// Get the range of particles that have just been killed.
	end_history_range.first = std::upper_bound( events.begin(),
						    dead_end,
						    alive_pair,
						    event_pred );
	end_history_range.second = std::lower_bound( events.begin(),
						     dead_end,
						     end_pair,
						     event_pred );

	// Process collision events.
	auto process_collision_task = hpx::parallel::for_each( 
	    hpx::parallel::parallel_task_execution_policy(),
	    collision_range.first,
	    collision_range.second,
	    process_collision_func );

	// Process boundary events.
	auto process_boundary_task = hpx::parallel::for_each( 
	    hpx::parallel::parallel_task_execution_policy(),
	    boundary_range.first,
	    boundary_range.second,
	    process_boundary_func );

	// Tally particles that died on the last event cycle and spawn a new
	// one if needed.
	auto process_end_history_task = hpx::parallel::for_each( 
	    hpx::parallel::parallel_task_execution_policy(),
	    end_history_range.first,
	    end_history_range.second,
	    end_history_func );

	// Wait on events to process.
	process_collision_task.wait();
	process_boundary_task.wait();
	process_end_history_task.wait();

	// Sort the events again to get the active particles and dead particles.
	std::sort( events.begin(), events.end(), event_pred );

	// Find the first particle that is still alive.
	alive_end = std::lower_bound( events.begin(), events.end(),
				      alive_pair, event_pred );

	// Find the first particle that is completely dead and will not
	// respawn with a new source particle.
	dead_end = std::lower_bound( events.begin(), events.end(),
				     end_pair, event_pred );

	++counter;
    } 

    // Process any remaining particles.
    auto process_end_history_task = hpx::parallel::for_each( 
	hpx::parallel::parallel_task_execution_policy(),
	events.begin(),
	dead_end,
	end_history_func );
    process_end_history_task.wait();
    
    std::cout << "NUM RUN " << num_run.load() << std::endl;
    std::cout << "NUM CYCLE " << counter << std::endl;
    ENSURE( num_run.load() == num_to_run );
}

//---------------------------------------------------------------------------//
// PRIVATE FUNCTIONS
//---------------------------------------------------------------------------//
void Domain_Transporter::get_next_event( 
    Particle_t& particle, events::Event& event ) const
{
    Step_Selector step_selector;

    CHECK(particle.alive());
    CHECK(particle.dist_mfp() > 0.0);

    // total interaction cross section
    double xs_total = d_physics->total(physics::TOTAL, particle);
    CHECK(xs_total >= 0.0);

    // sample distance to next collision
    double dist_col = 
	(xs_total > 0.0) ? (particle.dist_mfp() / xs_total) : constants::huge;

    // initialize the distance to collision in the step selector
    step_selector.initialize(dist_col, events::COLLISION);

    // calculate distance to next geometry boundary
    double dist_bnd = d_geometry->distance_to_boundary( particle.geo_state() );
    step_selector.submit(dist_bnd, events::BOUNDARY);

    // set the next event in the particle
    CHECK(step_selector.tag() < events::END_EVENT);
    event = static_cast<events::Event>( step_selector.tag() );

    // path-length tallies (the actual movement of the particle will
    // take place when we process the various events)
    d_tallier->path_length(step_selector.step(), particle);

    // update the mfp distance travelled
    if (events::COLLISION == event)
    {
	particle.set_dist_mfp( step_selector.step() );
    }
    else
    {
	particle.add_dist_mfp( -step_selector.step()*xs_total );
    }
}

//---------------------------------------------------------------------------//
void Domain_Transporter::process_boundary(Particle_t &particle,
					  events::Event& event,
                                          Bank_t     &bank) const
{
    REQUIRE(particle.alive());
    REQUIRE(event == events::BOUNDARY);

    // return if not a boundary event

    // move a particle to the surface and process the event through the surface
    d_geometry->move_to_surface(particle.geo_state());

    // get the in/out state of the particle
    int state = d_geometry->boundary_state(particle.geo_state());

    // reflected flag
    bool reflected = false;

    // process the boundary crossing based on the geometric state of the
    // particle
    switch (state)
    {
        case geometry::OUTSIDE:
            // the particle has left the problem geometry
            event = events::ESCAPE;
            particle.kill();

            // add a escape diagnostic
            DIAGNOSTICS_TWO(integers["geo_escape"]++);

            ENSURE(event == events::ESCAPE);
            break;

        case geometry::REFLECT:
            // the particle has hit a reflecting surface
            reflected = d_geometry->reflect(particle.geo_state());
            CHECK(reflected);

            // add a reflecting face diagnostic
            DIAGNOSTICS_TWO(integers["geo_reflect"]++);

            ENSURE(event == events::BOUNDARY);
            break;

        case geometry::INSIDE:
            // otherwise the particle is at an internal geometry boundary;
            // update the material id of the region the particle has entered
            particle.set_matid(d_geometry->matid(particle.geo_state()));

            // add variance reduction at surface crossings
            d_var_reduction->post_surface(particle, event, bank);

            // add a boundary crossing diagnostic
            DIAGNOSTICS_TWO(integers["geo_surface"]++);

            ENSURE(event == events::BOUNDARY);
            break;

        default:
            CHECK(0);
    }
}

//---------------------------------------------------------------------------//

void Domain_Transporter::process_collision(Particle_t &particle,
					   events::Event& event,
                                           Bank_t     &bank) const
{
    REQUIRE(d_var_reduction);
    REQUIRE(event == events::COLLISION);

    // move the particle to the collision site
    d_geometry->move_to_point(particle.dist_mfp(), particle.geo_state());

    // use the physics package to process the collision
    d_physics->collide(particle, event, bank);

    // apply weight windows
    d_var_reduction->post_collision(particle, event, bank);
}

} // end namespace profugus

//---------------------------------------------------------------------------//
//                 end of Domain_Transporter.cc
//---------------------------------------------------------------------------//
