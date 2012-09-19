namespace sirius
{

class Density
{
    private:

        PeriodicFunction<double> charge_density_;
    
        
    public:
    
        void initialize()
        {
            charge_density_.allocate(global.lmax_rho(), global.max_num_mt_points(), global.num_atoms(), 
                                     global.fft().size(), global.num_gvec());
        }
        
        void get_density(double* _rhomt, double* _rhoir)
        {
            memcpy(_rhomt, &charge_density_.f_rlm(0, 0, 0), global.lmmax_rho() * global.max_num_mt_points() * global.num_atoms() * sizeof(double)); 
            memcpy(_rhoir, &charge_density_.f_it(0), global.fft().size() * sizeof(double)); 
        }
    
        void initial_density()
        {
            std::vector<double> enu;
            for (int i = 0; i < global.num_atom_types(); i++)
                global.atom_type(i)->solve_free_atom(1e-8, 1e-5, 1e-4, enu);

            charge_density_.zero();

            double mt_charge = 0.0;
            for (int ia = 0; ia < global.num_atoms(); ia++)
            {
                int nmtp = global.atom(ia)->type()->num_mt_points();
                Spline<double> rho(nmtp, global.atom(ia)->type()->radial_grid());
                for (int ir = 0; ir < nmtp; ir++)
                {
                    rho[ir] = global.atom(ia)->type()->free_atom_density(ir);
                    charge_density_.f_rlm(0, ir, ia) = rho[ir] / y00;
                }
                rho.interpolate();

                // add charge of the MT sphere
                mt_charge += fourpi * rho.integrate(nmtp - 1, 2);
            }
            
            // distribute remaining charge
            for (int i = 0; i < global.fft().size(); i++)
                charge_density_.f_it(i) = (global.num_electrons() - mt_charge) / global.volume_it();
            
            global.fft().input(charge_density_.f_it());
            global.fft().forward();
            global.fft().output(global.num_gvec(), global.fft_index(), charge_density_.f_pw());
        }

        inline PeriodicFunction<double>& charge_density()
        {
            return charge_density_;
        }



};

Density density;

};