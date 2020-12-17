#include "solver.h"
#ifdef BUILD_GUI
#include "solver_socket_listener.h"
#endif
#include <iostream>
#include <fstream>

using namespace ratio;

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cerr << "usage: oRatio <input-file> [<input-file> ...] <output-file>" << std::endl;
        return -1;
    }

    // the problem files..
    std::vector<std::string> prob_names;
    for (int i = 1; i < argc - 1; i++)
        prob_names.push_back(argv[i]);

    // the solution file..
    std::string sol_name = argv[argc - 1];

    std::cout << "starting oRatio";
#ifdef BUILD_GUI
    std::cout << " in debug mode";
#endif
    std::cout << ".." << std::endl;

    solver s;
#ifdef BUILD_GUI
    solver_socket_listener l(s, HOST, SOLVER_PORT);
#endif

    s.init();
    try
    {
        std::cout << "parsing input files.." << std::endl;
        s.read(prob_names);

        std::cout << "solving the problem.." << std::endl;
        s.solve();
        std::cout << "hurray!! we have found a solution.." << std::endl;

        std::ofstream sol_file;
        sol_file.open(sol_name);
        sol_file << s;
        sol_file.close();
    }
    catch (const std::exception &ex)
    {
        std::cout << ex.what() << std::endl;
        return 1;
    }
}