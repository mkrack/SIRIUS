// Copyright (c) 2013-2014 Anton Kozhevnikov, Thomas Schulthess
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that 
// the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the 
//    following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions 
//    and the following disclaimer in the documentation and/or other materials provided with the distribution.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED 
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A 
// PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR 
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

/** \file global.h
 *   
 *  \brief Contains definition and partial implementation of sirius::Global class.
 */

#ifndef __GLOBAL_H__
#define __GLOBAL_H__


#include <vector>
#include "version.h"
#include "mpi_grid.h"
#include "splindex.h"
#include "step_function.h"
#include "error_handling.h"
#include "input.h"
#include "real_space_prj.h"

/// SIRIUS namespace.
namespace sirius {

// TODO: global is a bad idea in general. How to pass tonns of variables between functions and classes?

/// Global variables and methods
class Global
{
    private:

        /// true if class was initialized
        bool initialized_;
    
        /// maximum l for APW functions
        int lmax_apw_;
        
        /// maximum l for plane waves
        int lmax_pw_;
        
        /// maximum l for density
        int lmax_rho_;
        
        /// maximum l for potential
        int lmax_pot_;

        /// Cutoff for augmented-wave functions.
        double aw_cutoff_;

        /// Cutoff for plane-waves (for density and potential expansion).
        double pw_cutoff_;

        /// Cutoff for |G+k| plane-waves.
        double gk_cutoff_;
        
        /// number of first-variational states
        int num_fv_states_;

        /// number of bands (= number of spinor states)
        int num_bands_;
       
        /// number of spin componensts (1 or 2)
        int num_spins_;

        /// number of dimensions of the magnetization and effective magnetic field (0, 1 or 3)
        int num_mag_dims_;

        /// true if spin-orbit correction is applied
        bool so_correction_;
       
        /// true if UJ correction is applied
        bool uj_correction_;

        /// MPI grid dimensions
        std::vector<int> mpi_grid_dims_;
        
        /// MPI grid
        MPI_grid mpi_grid_;

        /// Starting time of the program.
        timeval start_time_;

        ev_solver_t std_evp_solver_type_;

        ev_solver_t gen_evp_solver_type_;

        /// Type of the processing unit.
        processing_unit_t processing_unit_;

        /// Smearing function width.
        double smearing_width_;
        
        electronic_structure_method_t esm_type_;
        
        /// Step function is used in full-potential methods.
        Step_function* step_function_;

        Reciprocal_lattice* reciprocal_lattice_;

        Unit_cell* unit_cell_;

        /// Base communicator.
        Communicator comm_;

        /// FFT wrapper for dense grid.
        FFT3D<CPU>* fft_;

        /// FFT wrapper for coarse grid.
        FFT3D<CPU>* fft_coarse_;

        #ifdef _GPU_
        FFT3D<GPU>* fft_gpu_;

        FFT3D<GPU>* fft_gpu_coarse_;
        #endif

        /// Parse input data-structures.
        void parse_input();

    public:

        Real_space_prj* real_space_prj_;

        /// Initiail input parameters from the input file and command line.
        initial_input_parameters iip_;

        initial_input_parameters::iterative_solver_input_section iterative_solver_input_section_;
        initial_input_parameters::xc_functionals_input_section xc_functionals_input_section_;
        initial_input_parameters::mixer_input_section mixer_input_section_;
    
        Global(initial_input_parameters iip__, Communicator const& comm__)
            : initialized_(false), 
              lmax_apw_(lmax_apw_default), 
              lmax_pw_(-1), 
              lmax_rho_(lmax_rho_default), 
              lmax_pot_(lmax_pot_default), 
              aw_cutoff_(aw_cutoff_default), 
              pw_cutoff_(pw_cutoff_default), 
              gk_cutoff_(pw_cutoff_default / 4.0), 
              num_fv_states_(-1), 
              num_spins_(1), 
              num_mag_dims_(0), 
              so_correction_(false), 
              uj_correction_(false),
              std_evp_solver_type_(ev_lapack),
              gen_evp_solver_type_(ev_lapack),
              processing_unit_(CPU),
              smearing_width_(0.001), 
              esm_type_(full_potential_lapwlo),
              step_function_(nullptr),
              reciprocal_lattice_(nullptr),
              comm_(comm__),
              iip_(iip__)
        {
            /* get the starting time */
            gettimeofday(&start_time_, NULL);

            iterative_solver_input_section_ = iip_.iterative_solver_input_section_;
            xc_functionals_input_section_ = iip_.xc_functionals_input_section_;
            mixer_input_section_ = iip_.mixer_input_section_;
            parse_input();

            /* create new empty unit cell */
            unit_cell_ = new Unit_cell(esm_type_, comm_, processing_unit_);
        }
            
        ~Global()
        {
            clear();
            delete unit_cell_;
        }

        void set_lmax_apw(int lmax_apw__)
        {
            lmax_apw_ = lmax_apw__;
        }

        void set_lmax_rho(int lmax_rho__)
        {
            lmax_rho_ = lmax_rho__;
        }

        void set_lmax_pot(int lmax_pot__)
        {
            lmax_pot_ = lmax_pot__;
        }

        void set_num_spins(int num_spins__)
        {
            num_spins_ = num_spins__;
        }

        void set_num_mag_dims(int num_mag_dims__)
        {
            num_mag_dims_ = num_mag_dims__;
        }

        inline int lmax_apw()
        {
            return lmax_apw_;
        }

        inline int lmmax_apw()
        {
            return Utils::lmmax(lmax_apw_);
        }
        
        inline int lmax_pw()
        {
            return lmax_pw_;
        }

        inline int lmmax_pw()
        {
            return Utils::lmmax(lmax_pw_);
        }
        
        inline int lmax_rho()
        {
            return lmax_rho_;
        }

        inline int lmmax_rho()
        {
            return Utils::lmmax(lmax_rho_);
        }
        
        inline int lmax_pot()
        {
            return lmax_pot_;
        }

        inline int lmax_beta()
        {
            return unit_cell_->lmax_beta();
        }

        inline int lmmax_pot()
        {
            return Utils::lmmax(lmax_pot_);
        }

        inline double aw_cutoff()
        {
            return aw_cutoff_;
        }

        inline void set_aw_cutoff(double aw_cutoff__)
        {
            aw_cutoff_ = aw_cutoff__;
        }

        /// Return plane-wave cutoff for G-vectors.
        inline double pw_cutoff()
        {
            return pw_cutoff_;
        }

        /// Set plane-wave cutoff.
        inline void set_pw_cutoff(double pw_cutoff__)
        {
            pw_cutoff_ = pw_cutoff__;
        }

        inline double gk_cutoff()
        {
            return gk_cutoff_;
        }

        inline void set_gk_cutoff(double gk_cutoff__)
        {
            gk_cutoff_ = gk_cutoff__;
        }

        inline int num_fv_states()
        {
            return num_fv_states_;
        }

        inline void set_num_fv_states(int num_fv_states__)
        {
            num_fv_states_ = num_fv_states__;
        }

        inline int num_bands()
        {
            return num_bands_;
        }
        
        inline int num_spins()
        {
            assert(num_spins_ == 1 || num_spins_ == 2);
            
            return num_spins_;
        }

        inline int num_mag_dims()
        {
            assert(num_mag_dims_ == 0 || num_mag_dims_ == 1 || num_mag_dims_ == 3);
            
            return num_mag_dims_;
        }

        inline int max_occupancy()
        {
            return (2 / num_spins());
        }
        
        inline void set_so_correction(bool so_correction__)
        {
            so_correction_ = so_correction__; 
        }

        inline void set_uj_correction(bool uj_correction__)
        {
            uj_correction_ = uj_correction__; 
        }

        inline bool so_correction()
        {
            return so_correction_;
        }
        
        inline bool uj_correction()
        {
            return uj_correction_;
        }

        inline MPI_grid& mpi_grid()
        {
            return mpi_grid_;
        }

        inline bool initialized()
        {
            return initialized_;
        }

        inline processing_unit_t processing_unit()
        {
            return processing_unit_;
        }

        inline double smearing_width()
        {
            return smearing_width_;
        }

        bool need_sv()
        {
            if (num_spins() == 2 || uj_correction() || so_correction()) return true;
            return false;
        }
        
        inline std::vector<int>& mpi_grid_dims()
        {
            return mpi_grid_dims_;
        }

        /// Initialize the global variables
        void initialize();

        /// Clear global variables
        void clear();

        void print_info();

        void write_json_output();

        void create_storage_file();

        void update(); // TODO: better way to update unit cell after relaxation
       
        std::string start_time(const char* fmt);

        inline electronic_structure_method_t esm_type()
        {
            return esm_type_;
        }

        inline Step_function* step_function()
        {
            return step_function_;
        }

        inline double step_function(int ir)
        {
            return step_function_->theta_it(ir);
        }

        inline Reciprocal_lattice* reciprocal_lattice()
        {
            return reciprocal_lattice_;
        }

        inline Unit_cell* unit_cell()
        {
            return unit_cell_;
        }

        inline Communicator& comm()
        {
            return comm_;
        }

        void read_unit_cell_input();

        inline ev_solver_t std_evp_solver_type()
        {
            return std_evp_solver_type_;
        }

        inline ev_solver_t gen_evp_solver_type()
        {
            return gen_evp_solver_type_;
        }

        inline FFT3D<CPU>* fft()
        {
            return fft_;
        }

        inline FFT3D<CPU>* fft_coarse()
        {
            return fft_coarse_;
        }

        #ifdef _GPU_
        inline FFT3D<GPU>* fft_gpu()
        {
            return fft_gpu_;
        }

        inline FFT3D<GPU>* fft_gpu_coarse()
        {
            return fft_gpu_coarse_;
        }
        #endif
};

};

#endif // __GLOBAL_H__

/** \page stdvarname Standard variable names
    
    Below is the list of standard names for some of the loop variables:
    
    l - index of orbital quantum number \n
    m - index of azimutal quantum nuber \n
    lm - combined index of (l,m) quantum numbers \n
    ia - index of atom \n
    ic - index of atom class \n
    iat - index of atom type \n
    ir - index of r-point \n
    ig - index of G-vector \n
    idxlo - index of local orbital \n
    idxrf - index of radial function \n

    The loc suffix is added to the variable to indicate that it runs over local fraction of elements for the given
    MPI rank. Typical code looks like this:
    
    \code{.cpp}
        // zero array
        memset(&mt_val[0], 0, parameters_.num_atoms() * sizeof(T));
        
        // loop over local fraction of atoms
        for (int ialoc = 0; ialoc < parameters_.spl_num_atoms().local_size(); ialoc++)
        {
            // get global index of atom
            int ia = parameters_.spl_num_atoms(ialoc);

            int nmtp = parameters_.atom(ia)->num_mt_points();
           
            // integrate spherical part of the function
            Spline<T> s(nmtp, parameters_.atom(ia)->type()->radial_grid());
            for (int ir = 0; ir < nmtp; ir++) s[ir] = f_mt<local>(0, ir, ialoc);
            mt_val[ia] = s.interpolate().integrate(2) * fourpi * y00;
        }

        // simple array synchronization
        Platform::allreduce(&mt_val[0], parameters_.num_atoms());
    \endcode
*/
