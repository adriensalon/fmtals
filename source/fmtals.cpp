#include <fmtals/fmtals.hpp>

#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <variant>
#include <vector>

#include <cereal/archives/xml.hpp>
#include <zlib.h>

// gz

void gz_decompress(const std::string& gz_filepath, std::string& data)
{
    std::ifstream _file(gz_filepath, std::ios::binary);
    if (!_file) {
        throw std::runtime_error("Failed to open file");
    }
    _file.seekg(0, std::ios::end);
    size_t _file_size = _file.tellg();
    _file.seekg(0, std::ios::beg);
    std::vector<char> _compressed_data(_file_size);
    _file.read(_compressed_data.data(), _file_size);
    z_stream _stream {};
    _stream.next_in = reinterpret_cast<Bytef*>(_compressed_data.data());
    _stream.avail_in = static_cast<uInt>(_file_size);
    if (inflateInit2(&_stream, 16 + MAX_WBITS) != Z_OK) {
        throw std::runtime_error("Failed to initialize zlib");
    }
    data.clear();
    char _buffer[4096];
    int _ret;
    do {
        _stream.next_out = reinterpret_cast<Bytef*>(_buffer);
        _stream.avail_out = sizeof(_buffer);
        _ret = inflate(&_stream, Z_NO_FLUSH);
        if (_ret == Z_STREAM_ERROR) {
            throw std::runtime_error("Stream error during decompression");
        }
        size_t _decompressed_size = sizeof(_buffer) - _stream.avail_out;
        data.insert(data.end(), _buffer, _buffer + _decompressed_size);

    } while (_ret != Z_STREAM_END);
    inflateEnd(&_stream);
}

void gz_decompress(std::istream& gz_stream, std::string& data)
{
    // if (auto* _fstream = dynamic_cast<std::ifstream*>(&gz_stream)) {
    //     if (!(_fstream->flags() & std::ios::binary)) {
    //         throw std::runtime_error("gz_decompress: ifstream must be opened in binary mode");
    //     }
    // }
    std::vector<char> _compressed_data((std::istreambuf_iterator<char>(gz_stream)), std::istreambuf_iterator<char>());
    if (_compressed_data.empty()) {
        throw std::runtime_error("Input stream is empty or unreadable");
    }
    z_stream _zstream {};
    _zstream.next_in = reinterpret_cast<Bytef*>(_compressed_data.data());
    _zstream.avail_in = static_cast<uInt>(_compressed_data.size());
    if (inflateInit2(&_zstream, 16 + MAX_WBITS) != Z_OK) {
        throw std::runtime_error("Failed to initialize zlib (gzip mode)");
    }
    data.clear();
    char _buffer[40960];
    int _ret;
    do {
        _zstream.next_out = reinterpret_cast<Bytef*>(_buffer);
        _zstream.avail_out = sizeof(_buffer);
        _ret = inflate(&_zstream, Z_NO_FLUSH);
        if (_ret != Z_OK && _ret != Z_STREAM_END) {
            inflateEnd(&_zstream);
            throw std::runtime_error("Zlib inflate error: " + std::to_string(_ret));
        }
        std::size_t _n_written = sizeof(_buffer) - _zstream.avail_out;
        data.append(_buffer, _n_written);
    } while (_ret != Z_STREAM_END);
    inflateEnd(&_zstream);
}

void gz_compress(const std::string& gz_filepath, const std::string& data)
{
    std::ofstream _file(gz_filepath, std::ios::binary);
    if (!_file) {
        throw std::runtime_error("Failed to open file for writing");
    }
    z_stream _stream {};
    _stream.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(data.data())); // Input data
    _stream.avail_in = static_cast<uInt>(data.length());
    if (deflateInit2(&_stream, Z_BEST_COMPRESSION, Z_DEFLATED, 16 + MAX_WBITS, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
        throw std::runtime_error("Failed to initialize zlib for compression");
    }
    char _buffer[4096];
    int _ret;
    do {
        _stream.next_out = reinterpret_cast<Bytef*>(_buffer); // Output buffer
        _stream.avail_out = sizeof(_buffer); // Buffer size

        _ret = deflate(&_stream, Z_FINISH); // Compress
        if (_ret != Z_OK && _ret != Z_STREAM_END) {
            deflateEnd(&_stream);
            throw std::runtime_error("Stream error during compression");
        }
        size_t _compressed_size = sizeof(_buffer) - _stream.avail_out;
        _file.write(_buffer, _compressed_size);
        if (!_file) {
            deflateEnd(&_stream);
            throw std::runtime_error("Failed to write to file");
        }
    } while (_ret != Z_STREAM_END); // Continue until all data is compressed
    deflateEnd(&_stream);
}

// xml

using xml_document = cereal::rapidxml::xml_document<char>;
using xml_node = cereal::rapidxml::xml_node<char>;
using xml_attribute = cereal::rapidxml::xml_attribute<char>;

std::string xml_to_string(xml_document& xml_doc)
{
    std::string xml_string;
    cereal::rapidxml::print(std::back_inserter(xml_string), xml_doc);
    return xml_string;
}

xml_node* xml_get_node(xml_node* daw_node, const std::vector<std::string>& daw_path)
{
    xml_node* _current_node = daw_node;
    for (const std::string& _entry : daw_path) {
        _current_node = _current_node->first_node(_entry.c_str(), _entry.length());
    }
    return _current_node;
}

std::vector<xml_node*> xml_get_nodes(xml_node* daw_node, const std::vector<std::string>& daw_path)
{
    std::vector<xml_node*> _nodes;
    xml_node* _current_node = xml_get_node(daw_node, daw_path)->first_node(0);
    while (_current_node) {
        _nodes.emplace_back(_current_node);
        _current_node = _current_node->next_sibling(0);
    }
    return _nodes;
}

xml_attribute* xml_get_attribute(const xml_node* daw_node, const std::string& daw_attribute)
{
    return daw_node->first_attribute(daw_attribute.c_str(), daw_attribute.length());
}

xml_node* xml_create_node(xml_document& document, xml_node* parent, const std::string& name, const std::vector<std::pair<std::string, std::string>>& attributes = {})
{
    char* _allocated_name = document.allocate_string(name.c_str());
    xml_node* _child = document.allocate_node(cereal::rapidxml::node_element, _allocated_name);
    for (const std::pair<std::string, std::string>& _attribute : attributes) {
        char* _allocated_attribute_name = document.allocate_string(_attribute.first.c_str());
        char* _allocated_attribute_value = document.allocate_string(_attribute.second.c_str());
        _child->append_attribute(document.allocate_attribute(_allocated_attribute_name, _allocated_attribute_value));
    }
    parent->append_node(_child);
    return _child;
}

#define VARIANT2_ACCESSOR_IMPL(struct_name, accessor_type, accessor_name) \
    accessor_type& struct_name::accessor_name()                           \
    {                                                                     \
        if (index() == 0) {                                               \
            return std::get<0>(*this).accessor_name;                      \
        } else if (index() == 1) {                                        \
            return std::get<1>(*this).accessor_name;                      \
        } else {                                                          \
            throw std::runtime_error("Unexpected track variant");         \
        }                                                                 \
    }                                                                     \
    const accessor_type& struct_name::accessor_name() const               \
    {                                                                     \
        if (index() == 0) {                                               \
            return std::get<0>(*this).accessor_name;                      \
        } else if (index() == 1) {                                        \
            return std::get<1>(*this).accessor_name;                      \
        } else {                                                          \
            throw std::runtime_error("Unexpected track variant");         \
        }                                                                 \
    }

namespace dfmt {
namespace ableton {

    VARIANT2_ACCESSOR_IMPL(project::user_track, bool, envelope_mode_preferred)
    VARIANT2_ACCESSOR_IMPL(project::user_track, project::track_delay, delay)
    VARIANT2_ACCESSOR_IMPL(project::user_track, project::name, track_name)
    VARIANT2_ACCESSOR_IMPL(project::user_track, std::optional<std::uint32_t>, color_index)
    VARIANT2_ACCESSOR_IMPL(project::user_track, std::optional<std::uint32_t>, color)
    VARIANT2_ACCESSOR_IMPL(project::user_track, std::int32_t, track_group_id)
    VARIANT2_ACCESSOR_IMPL(project::user_track, bool, track_unfolded)

}

void import_project(std::istream& stream, ableton::project& proj, ableton::version& ver)
{
    // decompress
    xml_document _xml_doc;
    std::string _xml_data;
    gz_decompress(stream, _xml_data);
    // std::cout << _xml_data << std::endl;
    _xml_doc.parse<0>(_xml_data.data());

    // ableton header
    xml_node* _ableton_node = _xml_doc.first_node("Ableton");
    proj.project_version_info.major_version = xml_get_attribute(_ableton_node, "MajorVersion")->value();
    proj.project_version_info.minor_version = xml_get_attribute(_ableton_node, "MinorVersion")->value();

    // HERE PROCESS VERSION FROM STRINGS
    ver = ableton::version::v_9_7_7;

    if (ver >= ableton::version::v_11_0_0) {
        proj.project_version_info.minor_version = xml_get_attribute(_ableton_node, "SchemaChangeCount")->value();
    }
    proj.project_version_info.creator = xml_get_attribute(_ableton_node, "Creator")->value();
    proj.project_version_info.revision = xml_get_attribute(_ableton_node, "Revision")->value();
    {
        // liveset
        xml_node* _liveset_node = xml_get_node(_ableton_node, { "LiveSet" });
        {
            // ids
        }
    }
}

void export_project(std::ostream& stream, const ableton::project& proj, const ableton::version& ver)
{
    // xml declaration
    xml_document _xml_doc;
    xml_node* _declaration_node = _xml_doc.allocate_node(cereal::rapidxml::node_declaration);
    _declaration_node->append_attribute(_xml_doc.allocate_attribute("version", "1.0"));
    _declaration_node->append_attribute(_xml_doc.allocate_attribute("encoding", "UTF-8"));
    _xml_doc.append_node(_declaration_node);

    // ableton header
    xml_node* _ableton_node = _xml_doc.allocate_node(cereal::rapidxml::node_element, "Ableton");
    _ableton_node->append_attribute(_xml_doc.allocate_attribute("MajorVersion", proj.project_version_info.major_version.c_str()));
    _ableton_node->append_attribute(_xml_doc.allocate_attribute("MinorVersion", proj.project_version_info.minor_version.c_str()));
    if (ver >= ableton::version::v_11_0_0) {
        // _ableton_node->append_attribute(_xml_doc.allocate_attribute("SchemaChangeCount", "7"));
    }
    _ableton_node->append_attribute(_xml_doc.allocate_attribute("Creator", proj.project_version_info.creator.c_str()));
    _ableton_node->append_attribute(_xml_doc.allocate_attribute("Revision", proj.project_version_info.revision.c_str()));
    _xml_doc.append_node(_ableton_node);
    {
        // liveset
        xml_node* _liveset_node = xml_create_node(_xml_doc, _ableton_node, "LiveSet");
        {
            // ids etc
            if (ver >= ableton::version::v_11_0_0) {
                // xml_create_node(_xml_doc, _liveset_node, "NextPointeeId", { { "Value", "36073" } });
            }
            xml_create_node(_xml_doc, _liveset_node, "OverwriteProtectionNumber", { { "Value", "3072" } });
            xml_create_node(_xml_doc, _liveset_node, "LomId", { { "Value", "0" } });
            xml_create_node(_xml_doc, _liveset_node, "LomIdView", { { "Value", "0" } });

            // tracks
            xml_node* _tracks_node = xml_create_node(_xml_doc, _liveset_node, "Tracks");
            for (const ableton::project::user_track& _track : proj.tracks) {

                // auto& _audio_track = std::get<ableton::project::audio_track>(_track);

                // ids etc
                xml_node* _track_node = xml_create_node(_xml_doc, _tracks_node, "AudioTrack", { { "Id", "46" } });
                xml_create_node(_xml_doc, _track_node, "LomId", { { "Value", "0" } });
                xml_create_node(_xml_doc, _track_node, "LomIdView", { { "Value", "0" } });
                // xml_create_node(_xml_doc, _track_node, "IsContentSelectedInDocument", { { "Value",  } });
                // xml_create_node(_xml_doc, _track_node, "PreferredContentViewMode", { { "Value", "0" } });

                // track delay
                xml_node* _track_delay_node = xml_create_node(_xml_doc, _track_node, "TrackDelay");
                xml_create_node(_xml_doc, _track_delay_node, "Value", { { "Value", std::to_string(_track.delay().value).c_str() } });
                xml_create_node(_xml_doc, _track_delay_node, "IsValueSampleBased", { { "Value", std::to_string(_track.delay().is_value_sample_based).c_str() } });

                // name
                xml_node* _track_name_node = xml_create_node(_xml_doc, _track_node, "Name");
                {
                    xml_create_node(_xml_doc, _track_name_node, "EffectiveName", { { "Value", _track.track_name().effective_name.c_str() } });
                    xml_create_node(_xml_doc, _track_name_node, "UserName", { { "Value", _track.track_name().user_name.c_str() } });
                    xml_create_node(_xml_doc, _track_name_node, "Annotation", { { "Value", _track.track_name().annotation.c_str() } });
                    if (ver >= ableton::version::v_12_0_0) {
                        // xml_create_node(_xml_doc, _track_name_node, "MemorizedFirstClipName", { { "Value", "" } });
                    }
                }

                // color
                if (ver >= ableton::version::v_12_0_0) {
                    xml_create_node(_xml_doc, _track_node, "Color", { { "Value", std::to_string(_track.color().value()).c_str() } });
                } else {
                    xml_create_node(_xml_doc, _track_node, "ColorIndex", { { "Value", std::to_string(_track.color_index().value()).c_str() } });
                }
                xml_create_node(_xml_doc, xml_create_node(_xml_doc, _track_node, "AutomationEnvelopes"), "Envelopes");
                xml_create_node(_xml_doc, _track_node, "TrackGroupId", { { "Value", "-1" } }); // -1 vers master
                xml_create_node(_xml_doc, _track_node, "TrackUnfolded", { { "Value", "true" } });
                xml_create_node(_xml_doc, _track_node, "DevicesListWrapper", { { "LomId", "0" } });
                xml_create_node(_xml_doc, _track_node, "ClipSlotsListWrapper", { { "LomId", "0" } });
                xml_create_node(_xml_doc, _track_node, "ViewData", { { "Value", "{}" } });

                // take lanes
                xml_node* _take_lanes_node = xml_create_node(_xml_doc, _track_node, "TakeLanes");
                {
                    xml_create_node(_xml_doc, _take_lanes_node, "TakeLanes");
                    xml_create_node(_xml_doc, _take_lanes_node, "AreTakeLanesFolded", { { "Value", "true" } });
                }

                xml_create_node(_xml_doc, _track_node, "LinkedTrackGroupId", { { "Value", "-1" } });
                xml_create_node(_xml_doc, _track_node, "SavedPlayingSlot", { { "Value", "-1" } });
                xml_create_node(_xml_doc, _track_node, "SavedPlayingOffset", { { "Value", "0" } });
                xml_create_node(_xml_doc, _track_node, "Freeze", { { "Value", "false" } });
                xml_create_node(_xml_doc, _track_node, "VelocityDetail", { { "Value", "0" } });
                xml_create_node(_xml_doc, _track_node, "NeedArrangerRefreeze", { { "Value", "true" } });
                xml_create_node(_xml_doc, _track_node, "PostProcessFreezeClips", { { "Value", "0" } });

                // device chain
                xml_node* _device_chain_node = xml_create_node(_xml_doc, _track_node, "DeviceChain");
                {
                    xml_node* _automation_lanes_node = xml_create_node(_xml_doc, _device_chain_node, "AutomationLanes");
                    xml_node* _automation_lanes_child_node = xml_create_node(_xml_doc, _automation_lanes_node, "AutomationLanes");

                    // automation lane 0
                    xml_node* _automation_lane_0_node = xml_create_node(_xml_doc, _automation_lanes_child_node, "AutomationLane", { { "Id", "0" } });
                    {
                        xml_create_node(_xml_doc, _automation_lane_0_node, "SelectedDevice", { { "Value", "0" } }); // !!
                        xml_create_node(_xml_doc, _automation_lane_0_node, "SelectedEnvelope", { { "Value", "0" } }); // !!
                        xml_create_node(_xml_doc, _automation_lane_0_node, "IsContentSelectedInDocument", { { "Value", "false" } });
                        xml_create_node(_xml_doc, _automation_lane_0_node, "LaneHeight", { { "Value", "68" } }); // !!
                        xml_create_node(_xml_doc, _automation_lanes_node, "AreAdditionalAutomationLanesFolded", { { "Value", "false" } }); // !!
                    }

                    // clip envelope chooser
                    xml_node* _clip_envelope_chooser_node = xml_create_node(_xml_doc, _device_chain_node, "ClipEnvelopeChooserViewState");
                    {
                        xml_create_node(_xml_doc, _clip_envelope_chooser_node, "SelectedDevice", { { "Value", "0" } });
                        xml_create_node(_xml_doc, _clip_envelope_chooser_node, "SelectedEnvelope", { { "Value", "0" } });
                        xml_create_node(_xml_doc, _clip_envelope_chooser_node, "PreferModulationVisible", { { "Value", "false" } });
                    }

                    // audio input routing
                    xml_node* _audio_input_routing_node = xml_create_node(_xml_doc, _device_chain_node, "AudioInputRouting");
                    {
                        xml_create_node(_xml_doc, _audio_input_routing_node, "Target", { { "Value", "AudioIn/External/M0" } });
                        xml_create_node(_xml_doc, _audio_input_routing_node, "UpperDisplayString", { { "Value", "Ext. In" } });
                        xml_create_node(_xml_doc, _audio_input_routing_node, "LowerDisplayString", { { "Value", "1" } });

                        // mpe settings
                        xml_node* _mpe_settings_node = xml_create_node(_xml_doc, _audio_input_routing_node, "MpeSettings");
                        {
                            xml_create_node(_xml_doc, _mpe_settings_node, "ZoneType", { { "Value", "0" } });
                            xml_create_node(_xml_doc, _mpe_settings_node, "FirstNoteChannel", { { "Value", "1" } });
                            xml_create_node(_xml_doc, _mpe_settings_node, "LastNoteChannel", { { "Value", "15" } });
                        }
                    }

                    // midi input routing
                    xml_node* _midi_input_routing_node = xml_create_node(_xml_doc, _device_chain_node, "MidiInputRouting");
                    {
                        xml_create_node(_xml_doc, _midi_input_routing_node, "Target", { { "Value", "MidiIn/External.All/-1" } });
                        xml_create_node(_xml_doc, _midi_input_routing_node, "UpperDisplayString", { { "Value", "Ext: All Ins" } });
                        xml_create_node(_xml_doc, _midi_input_routing_node, "LowerDisplayString", { { "Value", "" } });

                        // mpe settings
                        xml_node* _mpe_settings_node = xml_create_node(_xml_doc, _midi_input_routing_node, "MpeSettings");
                        {
                            xml_create_node(_xml_doc, _mpe_settings_node, "ZoneType", { { "Value", "0" } });
                            xml_create_node(_xml_doc, _mpe_settings_node, "FirstNoteChannel", { { "Value", "1" } });
                            xml_create_node(_xml_doc, _mpe_settings_node, "LastNoteChannel", { { "Value", "15" } });
                        }
                    }

                    // audio output routing
                    xml_node* _audio_output_routing_node = xml_create_node(_xml_doc, _device_chain_node, "AudioOutputRouting");
                    {
                        xml_create_node(_xml_doc, _audio_output_routing_node, "Target", { { "Value", "AudioOut/Main" } });
                        xml_create_node(_xml_doc, _audio_output_routing_node, "UpperDisplayString", { { "Value", "Main" } });
                        xml_create_node(_xml_doc, _audio_output_routing_node, "LowerDisplayString", { { "Value", "" } });

                        // mpe settings
                        xml_node* _mpe_settings_node = xml_create_node(_xml_doc, _audio_output_routing_node, "MpeSettings");
                        {
                            xml_create_node(_xml_doc, _mpe_settings_node, "ZoneType", { { "Value", "0" } });
                            xml_create_node(_xml_doc, _mpe_settings_node, "FirstNoteChannel", { { "Value", "1" } });
                            xml_create_node(_xml_doc, _mpe_settings_node, "LastNoteChannel", { { "Value", "15" } });
                        }
                    }

                    // midi output routing
                    xml_node* _midi_output_routing_node = xml_create_node(_xml_doc, _device_chain_node, "MidiOutputRouting");
                    {
                        xml_create_node(_xml_doc, _midi_output_routing_node, "Target", { { "Value", "MidiOut/None" } });
                        xml_create_node(_xml_doc, _midi_output_routing_node, "UpperDisplayString", { { "Value", "None" } });
                        xml_create_node(_xml_doc, _midi_output_routing_node, "LowerDisplayString", { { "Value", "" } });

                        // mpe settings
                        xml_node* _mpe_settings_node = xml_create_node(_xml_doc, _midi_output_routing_node, "MpeSettings");
                        {
                            xml_create_node(_xml_doc, _mpe_settings_node, "ZoneType", { { "Value", "0" } });
                            xml_create_node(_xml_doc, _mpe_settings_node, "FirstNoteChannel", { { "Value", "1" } });
                            xml_create_node(_xml_doc, _mpe_settings_node, "LastNoteChannel", { { "Value", "15" } });
                        }
                    }

                    // mixer
                    xml_node* _mixer_node = xml_create_node(_xml_doc, _device_chain_node, "Mixer");
                    {
                    }
                }
            }
        }
    }

    // compress
    std::string _xml_data = xml_to_string(_xml_doc);
    std::cout << _xml_data;
    // gz_compress(output.string(), _xml_data);
}

}