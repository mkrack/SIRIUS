#include <sirius.h>
#include <thread>
#include <wave_functions.h>

using namespace sirius;

void test_fft_real(vector3d<int> const& dims__, double cutoff__)
{
    matrix3d<double> M;
    M(0, 0) = M(1, 1) = M(2, 2) = 1.0;

    FFT3D_grid fft_grid(cutoff__, M);

    FFT3D fft(fft_grid, mpi_comm_world(), CPU);

    Gvec gvec(vector3d<double>(0, 0, 0), M, cutoff__, fft.grid(), mpi_comm_world(), 1, false, false);
    Gvec gvec_r(vector3d<double>(0, 0, 0), M, cutoff__, fft.grid(), mpi_comm_world(), 1, false, true);

    if (gvec_r.num_gvec() != gvec.num_gvec() / 2 + 1)
    {
        printf("wrong number of reduced G-vectors");
        exit(1);
    }

    printf("num_gvec: %i, num_gvec_reduced: %i\n", gvec.num_gvec(), gvec_r.num_gvec());
    printf("num_gvec_loc: %i %i\n", gvec.num_gvec(mpi_comm_world().rank()), gvec_r.num_gvec(mpi_comm_world().rank()));

    mdarray<double_complex, 1> phi(gvec_r.num_gvec_fft());
    for (int i = 0; i < gvec_r.num_gvec_fft(); i++) phi(i) = type_wrapper<double_complex>::random();
    phi(0) = 1.0;
    fft.transform<1>(gvec_r, &phi(0));

    for (int i = 0; i < fft.local_size(); i++)
    {
        if (fft.buffer(i).imag() > 1e-10)
        {
            printf("function is not real at idx = %i, image value: %18.12f\n", i, fft.buffer(i).imag());
            exit(1);
        }
    }
    mdarray<double_complex, 1> phi1(gvec_r.num_gvec_fft());
    fft.transform<-1>(gvec_r, &phi1(0));

    double rms = 0;
    for (int i = 0; i < gvec_r.num_gvec_fft(); i++) rms += std::pow(std::abs(phi(i) - phi1(i)), 2);
    rms = std::sqrt(rms / gvec_r.num_gvec_fft());
    printf("rms: %18.12f\n", rms);
    if (rms > 1e-13)
    {
        printf("functions are different\n");
        exit(1);
    }
}

int main(int argn, char **argv)
{
    cmd_args args;
    args.register_key("--dims=", "{vector3d<int>} FFT dimensions");
    args.register_key("--cutoff=", "{double} cutoff radius in G-space");

    args.parse_args(argn, argv);
    if (args.exist("help"))
    {
        printf("Usage: %s [options]\n", argv[0]);
        args.print_help();
        return 0;
    }

    std::vector<int> vd = args.value< std::vector<int> >("dims", {132, 132, 132});
    vector3d<int> dims(vd[0], vd[1], vd[2]); 
    double cutoff = args.value<double>("cutoff", 50);

    sirius::initialize(1);

    test_fft_real(dims, cutoff);
    
    sirius::finalize();

    return 0;
}