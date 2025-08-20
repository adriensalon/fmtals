# fmtals

Reversal of the Ableton Live .als (liveset) and .alp (preset) formats. Ableton Live files are represented as XML data, compressed as gzip. This project provides functionnality for importing and exporting livesets as C++17 data structures. If your Ableton Live version is not yet supported please report an issue. Documentation for the `ableton::project` data structure can be found directly in the [fmtals/fmtals.hpp](include/fmtals/fmtals.hpp) header.

### Usage

Use `void fmtals::import_project(std::istream&, fmtals::project&, fmtals::version&)` to import a project and retrieve the Ableton Live version it was created with.

Use `void fmtals::export_project(std::ostream&, const fmtals::project&, const fmtals::version&)` to export a project for a specified Ableton Live version.

### Supported versions
- 9.7.7
- 12.0.0
