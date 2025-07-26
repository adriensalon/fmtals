#include <fmtals/fmtals.hpp>

#include <filesystem>
#include <fstream>
#include <sstream>

#include <cereal/archives/xml.hpp>

extern void gz_decompress(std::istream& gz_stream, std::string& data);

std::string remove_double_newlines(const std::string& input)
{
    std::istringstream _stream(input);
    std::string _line;
    std::string _result;
    while (std::getline(_stream, _line)) {
        if (!_line.empty()) {
            _result += _line;
        }
    }
    return _result;
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cerr << "Usage: drag a .als file onto the executable\n";
        return 1;
    }
    std::filesystem::path _input_path(argv[1]);
    if (!std::filesystem::exists(_input_path)) {
        std::cerr << "Error: File does not exist\n";
        return 2;
    }
    if (_input_path.extension() != ".als" && _input_path.extension() != ".alp") {
        std::cerr << "Error: File is not an Ableton Live set or preset\n";
        return 3;
    }
    std::ifstream _input_stream(_input_path, std::ios::binary);
    cereal::rapidxml::xml_document<char> _xml_doc;
    std::string _xml_data;
    gz_decompress(_input_stream, _xml_data);
    std::filesystem::path _output_path = _input_path;
    _output_path.replace_extension(".xml");
    std::ofstream _output_stream(_output_path);
    if (!_output_stream) {
        std::cerr << "Error: Could not write to file: " << _output_path << '\n';
        return 4;
    }
    // _output_stream << remove_double_newlines(_xml_data);
    _output_stream << (_xml_data);
    _output_stream.close();
    std::cout << "XML written to: " << _output_path << '\n';
    return 0;
}