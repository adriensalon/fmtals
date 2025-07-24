#include <fmtals/fmtals.hpp>

#include <fstream>

int main()
{
    std::ifstream _istream("C:/Users/adri/dev/libfmtals/.dev/minimal9.als", std::ios::binary);
    std::ofstream _ostream("C:/Users/adri/dev/libfmtals/.dev/minimal9_out.als");
    dfmt::ableton::version _version;
    dfmt::ableton::project _project;

    dfmt::import_project(_istream, _project, _version);
    
    dfmt::ableton::project::user_track _track;
    _project.tracks.emplace_back(_track);
    // dfmt::export_project(_ostream, _project, _version);
    dfmt::export_project(_ostream, _project, dfmt::ableton::version::v_9_7_7);

    return 0;
}