#include <fmtals/fmtals.hpp>

#include <filesystem>
#include <fstream>
#include <sstream>

#include <cereal/archives/xml.hpp>

extern void gz_compress(std::ostream& gz_stream, const std::string& data);

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cerr << "Usage: drag a .xml file onto the executable\n";
        return 1;
    }
    std::filesystem::path _input_path(argv[1]);
    if (!std::filesystem::exists(_input_path)) {
        std::cerr << "Error: File does not exist\n";
        return 2;
    }
    if (_input_path.extension() != ".xml") {
        std::cerr << "Error: File is not an XML file\n";
        return 3;
    }
    std::ifstream _input_stream(_input_path);
    // cereal::rapidxml::xml_document<char> _xml_doc;
    std::ostringstream ss;
    ss << _input_stream.rdbuf();
    std::string _xml_data = ss.str();
    // _xml_doc.parse<cereal::rapidxml::parse_non_destructive>(_xml_data.data());
    std::filesystem::path _output_path = _input_path;
    _output_path.replace_extension(".als");
    std::ofstream _output_stream(_output_path, std::ios::binary);
    gz_compress(_output_stream, _xml_data);
    if (!_output_stream) {
        std::cerr << "Error: Could not write to file: " << _output_path << '\n';
        return 4;
    }
    _output_stream.close();
    std::cout << "Ableton Live set written to: " << _output_path << '\n';
    return 0;
}