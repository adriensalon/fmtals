#include <fmtals/fmtals.hpp>

#include <fstream>

int main()
{
    std::ifstream _istream("C:/Users/adri/dev/fmtals/.dev/minimal9.als", std::ios::binary);
    std::ofstream _ostream("C:/Users/adri/dev/fmtals/.dev/minimal9_out.als", std::ios::binary);
    fmtals::version _version;
    fmtals::project _project;

    try {
        fmtals::import_project(_istream, _project, _version);
        fmtals::export_project(_ostream, _project, fmtals::version::v_9_7_7);
        // dfmt::export_project(_ostream, _project, _version);

    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
    }

    return 0;
}