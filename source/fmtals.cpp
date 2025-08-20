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

void gz_decompress(std::istream& gz_stream, std::string& data)
{
    // if (std::ifstream* _fstream = dynamic_cast<std::ifstream*>(&gz_stream)) {
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
    char _buffer[4096];
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

void gz_compress(std::ostream& gz_stream, const std::string& data)
{
    if (!gz_stream) {
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
        gz_stream.write(_buffer, _compressed_size);
        if (!gz_stream) {
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

bool xml_stob(const std::string& s)
{
    if (s == "true")
        return true;
    if (s == "false")
        return false;
    throw std::invalid_argument("Expected 'true' or 'false', got: " + s);
}

std::string xml_to_string(xml_document& xml_doc)
{
    std::string xml_string;
    cereal::rapidxml::print(std::back_inserter(xml_string), xml_doc);
    return xml_string;
}

// xml_node* xml_get_node(xml_node* daw_node, const std::vector<std::string>& daw_path)
// {
//     xml_node* _current_node = daw_node;
//     for (const std::string& _entry : daw_path) {
//         _current_node = _current_node->first_node(_entry.c_str(), _entry.length());
//     }
//     return _current_node;
// }

xml_node* xml_get_node(const xml_node* parent_node, const std::string& child_name)
{
    return parent_node->first_node(child_name.c_str(), child_name.length());
}

// std::vector<xml_node*> xml_get_nodes(xml_node* daw_node, const std::vector<std::string>& daw_path)
// {
//     std::vector<xml_node*> _nodes;
//     xml_node* _current_node = xml_get_node(daw_node, daw_path)->first_node(0);
//     while (_current_node) {
//         _nodes.emplace_back(_current_node);
//         _current_node = _current_node->next_sibling(0);
//     }
//     return _nodes;
// }

std::vector<xml_node*> xml_get_nodes(xml_node* parent_node)
{
    std::vector<xml_node*> _nodes;
    xml_node* _current_node = parent_node->first_node(0);
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

xml_node* xml_create_node(xml_document& document, xml_node* parent, const std::string& name, const std::string& attribute)
{
    char* _allocated_name = document.allocate_string(name.c_str());
    xml_node* _child = document.allocate_node(cereal::rapidxml::node_element, _allocated_name);
    char* _allocated_attribute_name = document.allocate_string("Value");
    char* _allocated_attribute_value = document.allocate_string(attribute.c_str());
    _child->append_attribute(document.allocate_attribute(_allocated_attribute_name, _allocated_attribute_value));
    parent->append_node(_child);
    return _child;
}

template <typename T>
void xml_create_attribute(xml_document& document, xml_node* node, const std::string& attribute, const T& value)
{
    char* _allocated_attribute_name = document.allocate_string(attribute.c_str());
    char* _allocated_attribute_value;
    if constexpr (std::is_same_v<T, std::string>) {
        _allocated_attribute_value = document.allocate_string(value.c_str());
    } else if constexpr (std::is_same_v<T, bool>) {
        _allocated_attribute_value = document.allocate_string(value ? "true" : "false");
    } else {
        _allocated_attribute_value = document.allocate_string(std::to_string(value).c_str());
    }
    node->append_attribute(document.allocate_attribute(_allocated_attribute_name, _allocated_attribute_value));
}

template <typename T>
xml_node* xml_create_node_and_value(xml_document& document, xml_node* parent, const std::string& name, const T& value)
{
    // float format for 0.0000 ?
    if constexpr (std::is_same_v<T, std::string>) {
        return xml_create_node(document, parent, name, value.c_str());
    } else if constexpr (std::is_same_v<T, bool>) {
        return xml_create_node(document, parent, name, value ? "true" : "false");
    } else {
        return xml_create_node(document, parent, name, std::to_string(value).c_str());
    }
}

template <typename T>
T xml_get_value(const xml_node* node, const std::string& attribute = "Value")
{
    xml_attribute* _attribute = xml_get_attribute(node, attribute);
    std::string _str_value = _attribute->value();
    if constexpr (std::is_same_v<T, bool>) {
        return xml_stob(_str_value);
    } else if constexpr (std::is_same_v<T, std::string>) {
        return _str_value;
    } else if constexpr (std::is_same_v<T, float>) {
        return std::stof(_str_value);
    } else if constexpr (std::is_convertible_v<T, int>) {
        return std::stoi(_str_value);
    }
    return T {};
}

template <typename T>
T xml_get_node_and_value(const xml_node* node, const std::string& child_name)
{
    return xml_get_value<T>(xml_get_node(node, child_name));
}

template <typename T>
void xml_get_node_and_value(const xml_node* node, const std::string& child_name, T& value)
{
    value = xml_get_value<T>(xml_get_node(node, child_name));
}

namespace fmtals {

void import_project(std::istream& stream, project& proj, version& ver)
{
    // decompress
    xml_document _xml_doc;
    std::string _xml_data;
    gz_decompress(stream, _xml_data);
    _xml_doc.parse<0>(_xml_data.data());

    // ableton
    xml_node* _ableton_node = _xml_doc.first_node("Ableton");
    {
        // header attributes
        proj.major_version = xml_get_attribute(_ableton_node, "MajorVersion")->value();
        proj.minor_version = xml_get_attribute(_ableton_node, "MinorVersion")->value();
        proj.creator = xml_get_attribute(_ableton_node, "Creator")->value();
        proj.revision = xml_get_attribute(_ableton_node, "Revision")->value();
        ver = version::v_9_7_7; // HERE PROCESS VERSION FROM STRINGS

        // schema change count attribute in newer versions
        if (ver >= version::v_11_0_0) {
            proj.schema_change_count = xml_get_attribute(_ableton_node, "SchemaChangeCount")->value();
        }

        // liveset
        xml_node* _liveset_node = xml_get_node(_ableton_node, "LiveSet");
        {
            // ids
            xml_get_node_and_value(_liveset_node, "OverwriteProtectionNumber", proj.overwrite_protection_number);
            // lom id
            // lom id view

            // tracks
            std::vector<xml_node*> _track_nodes = xml_get_nodes(xml_get_node(_liveset_node, "Tracks"));
            std::size_t _tracks_count = _track_nodes.size();
            for (xml_node* _track_node : _track_nodes) {

                // track type
                project::user_track _user_track;
                std::string _track_type(_track_node->name());
                if (_track_type == "AudioTrack") {
                    _user_track = project::audio_track();
                } else if (_track_type == "MidiTrack") {
                    _user_track = project::midi_track();
                } else if (_track_type == "GroupTrack") {
                    _user_track = project::group_track();
                } else if (_track_type == "ReturnTrack") {
                    continue;
                    _user_track = project::return_track();
                } else {
                    throw std::runtime_error("Invalid track type");
                }

                // track visit
                std::visit([&](auto& _track_visit) {
                    using _track_type_t = std::decay_t<decltype(_track_visit)>;

                    _track_visit.id = xml_get_value<std::uint32_t>(_track_node, "Id");
                    // lom id TODO
                    // lom id view TODO
                    xml_get_node_and_value(_track_node, "EnvelopeModePreferred", _track_visit.envelope_mode_preferred);

                    xml_node* _delay_node = xml_get_node(_track_node, "TrackDelay");
                    xml_get_node_and_value(_delay_node, "Value", _track_visit.track_delay_value);
                    xml_get_node_and_value(_delay_node, "IsValueSampleBased", _track_visit.track_delay_is_value_sample_based);

                    xml_node* _name_node = xml_get_node(_track_node, "Name");
                    xml_get_node_and_value(_name_node, "EffectiveName", _track_visit.effective_name);
                    xml_get_node_and_value(_name_node, "UserName", _track_visit.user_name);
                    xml_get_node_and_value(_name_node, "Annotation", _track_visit.annotation);

                    if (ver >= version::v_12_0_0) {
                        xml_get_node_and_value(_track_node, "Color", _track_visit.color.emplace());
                    } else {
                        xml_get_node_and_value(_track_node, "ColorIndex", _track_visit.color_index.emplace());
                    }

                    // audio & midi specifics
                    if constexpr (std::is_same_v<_track_type_t, project::audio_track>) {
                        xml_get_node_and_value(_track_node, "TrackGroupId", _track_visit.track_group_id);
                        xml_get_node_and_value(_track_node, "TrackUnfolded", _track_visit.track_unfolded);
                        // devices list weapper TODO
                        // clip slots list weapper TODO
                        // view data TODO
                        xml_get_node_and_value(_track_node, "SavedPlayingSlot", _track_visit.saved_playing_slot);
                        xml_get_node_and_value(_track_node, "SavedPlayingOffset", _track_visit.saved_playing_slot);
                        xml_get_node_and_value(_track_node, "MidiFoldIn", _track_visit.midi_fold_in);
                        xml_get_node_and_value(_track_node, "MidiPrelisten", _track_visit.midi_prelisten);
                        xml_get_node_and_value(_track_node, "Freeze", _track_visit.freeze);
                        xml_get_node_and_value(_track_node, "VelocityDetail", _track_visit.velocity_detail);
                        xml_get_node_and_value(_track_node, "NeedArrangerRefreeze", _track_visit.need_arranger_refreeze);
                        xml_get_node_and_value(_track_node, "PostProcessFreezeClips", _track_visit.post_process_freeze_clips);
                        xml_get_node_and_value(_track_node, "MidiTargetPrefersFoldOrIsNotUniform", _track_visit.midi_target_prefers_fold_or_is_not_uniform);
                    }

                    xml_node* _device_chain_node = xml_get_node(_track_node, "DeviceChain");
                    {
                        // TODO
                    }
                },
                    _user_track);

                proj.tracks.emplace_back(_user_track);
            }

            if (ver >= version::v_12_0_0) {
                xml_node* _main_track = xml_get_node(_liveset_node, "MainTrack");
                {
                    // TODO
                }
            } else {
                xml_node* _master_track = xml_get_node(_liveset_node, "MasterTrack");
                {
                    // TODO
                }
            }

            xml_node* _pre_hear_track = xml_get_node(_liveset_node, "PreHearTrack");
            {
                // TODO
            }

            std::vector<xml_node*> _sends_pre_nodes = xml_get_nodes(xml_get_node(_liveset_node, "SendsPre"));
            proj.sends_pre.reserve(_sends_pre_nodes.size());
            for (xml_node* _sends_pre_node : _sends_pre_nodes) {
                proj.sends_pre.emplace_back(xml_get_value<bool>(_sends_pre_node));
            }

            // scenes TODO???

            xml_node* _transport_node = xml_get_node(_liveset_node, "Transport");
            xml_get_node_and_value(_transport_node, "PhaseNudgeTempo", proj.transport_phase_nudge_tempo);
            xml_get_node_and_value(_transport_node, "LoopOn", proj.transport_loop_on);
            xml_get_node_and_value(_transport_node, "LoopStart", proj.transport_loop_start);
            xml_get_node_and_value(_transport_node, "LoopLength", proj.transport_loop_length);
            xml_get_node_and_value(_transport_node, "LoopIsSongStart", proj.transport_loop_is_song_start);
            xml_get_node_and_value(_transport_node, "CurrentTime", proj.transport_current_time);
            xml_get_node_and_value(_transport_node, "PunchIn", proj.transport_punch_in);
            xml_get_node_and_value(_transport_node, "PunchOut", proj.transport_punch_out);
            // TODO metronome_tick_duration for 12
            xml_get_node_and_value(_transport_node, "DrawMode", proj.transport_draw_mode);
            if (ver < version::v_12_0_0) {
                xml_get_node_and_value(_transport_node, "ComputerKeyboardIsEnabled", proj.transport_computer_keyboard_is_enabled.emplace());
            }

            // song master values TODO

            // quantisation
            xml_get_node_and_value(_liveset_node, "GlobalQuantisation", proj.global_quantisation);
            xml_get_node_and_value(_liveset_node, "AutoQuantisation", proj.auto_quantisation);

            xml_node* _grid_node = xml_get_node(_liveset_node, "Grid");
            xml_get_node_and_value(_grid_node, "FixedNumerator", proj.grid_fixed_numerator);
            xml_get_node_and_value(_grid_node, "FixedDenominator", proj.grid_fixed_denominator);
            xml_get_node_and_value(_grid_node, "GridIntervalPixel", proj.grid_grid_interval_pixel);
            xml_get_node_and_value(_grid_node, "Ntoles", proj.grid_ntoles);
            xml_get_node_and_value(_grid_node, "SnapToGrid", proj.grid_snap_to_grid);
            xml_get_node_and_value(_grid_node, "Fixed", proj.grid_fixed);

            xml_node* _scale_info_node = xml_get_node(_liveset_node, "ScaleInformation");
            xml_get_node_and_value(_scale_info_node, "RootNote", proj.scale_information_root_note);
            xml_get_node_and_value(_scale_info_node, "Name", proj.scale_information_name);

            xml_get_node_and_value(_liveset_node, "SmpteFormat", proj.smpte_format);

            xml_node* _time_selection_node = xml_get_node(_liveset_node, "TimeSelection");
            xml_get_node_and_value(_time_selection_node, "AnchorTime", proj.time_selection_anchor_time);
            xml_get_node_and_value(_time_selection_node, "OtherTime", proj.time_selection_other_time);

            // sequencer navigator TODO

            if (ver < version::v_12_0_0) {
                xml_get_node_and_value(_liveset_node, "ViewStateLaunchPanel", proj.view_state_launch_panel.emplace());
                xml_get_node_and_value(_liveset_node, "ViewStateEnvelopePanel", proj.view_state_envelope_panel.emplace());
                xml_get_node_and_value(_liveset_node, "ViewStateSamplePanel", proj.view_state_sample_panel.emplace());
            }

            if (ver < version::v_12_0_0) {
                xml_node* _content_splitter_node = xml_get_node(_liveset_node, "ContentSplitterProperties");
                xml_get_node_and_value(_content_splitter_node, "Open", proj.content_splitter_properties_open.emplace());
                xml_get_node_and_value(_content_splitter_node, "Size", proj.content_splitter_properties_size.emplace());
            }

            // view states
            xml_get_node_and_value(_liveset_node, "ViewStateFxSlotCount", proj.view_state_fx_slot_count);
            xml_get_node_and_value(_liveset_node, "ViewStateSessionMixerHeight", proj.view_state_session_mixer_height);

            // locators TODO

            // detail clip keys midi TODO

            // tracks list wrapper TODO

            // visible tracks list wrapper TODO

            // return tracks list wrapper TODO

            // scenes list wrapper TODO

            // cue points list wrapper TODO

            xml_get_node_and_value(_liveset_node, "ChooserBar", proj.chooser_bar);
            xml_get_node_and_value(_liveset_node, "Annotation", proj.annotation);
            xml_get_node_and_value(_liveset_node, "SoloOrPflSavedValue", proj.solo_or_pfl_saved_value);
            xml_get_node_and_value(_liveset_node, "SoloInPlace", proj.solo_in_place);
            xml_get_node_and_value(_liveset_node, "CrossfadeCurve", proj.crossfade_curve);
            xml_get_node_and_value(_liveset_node, "LatencyCompensation", proj.latency_compensation);
            xml_get_node_and_value(_liveset_node, "HighlightedTrackIndex", proj.highlighted_track_index);
            xml_get_node_and_value(_liveset_node, "ArrangementOverdub", proj.arrangement_overdub);

            // color sequence index TODO
        }
    }
}

void export_project(std::ostream& stream, const project& proj, const version& ver)
{
    // xml declaration
    xml_document _xml_doc;
    xml_node* _declaration_node = _xml_doc.allocate_node(cereal::rapidxml::node_declaration);
    _declaration_node->append_attribute(_xml_doc.allocate_attribute("version", "1.0"));
    _declaration_node->append_attribute(_xml_doc.allocate_attribute("encoding", "UTF-8"));
    _xml_doc.append_node(_declaration_node);

    // ableton header
    xml_node* _ableton_node = _xml_doc.allocate_node(cereal::rapidxml::node_element, "Ableton");
    _ableton_node->append_attribute(_xml_doc.allocate_attribute("MajorVersion", proj.major_version.c_str()));
    _ableton_node->append_attribute(_xml_doc.allocate_attribute("MinorVersion", proj.minor_version.c_str()));
    if (ver >= version::v_11_0_0) {
        // _ableton_node->append_attribute(_xml_doc.allocate_attribute("SchemaChangeCount", "7"));
    }
    _ableton_node->append_attribute(_xml_doc.allocate_attribute("Creator", proj.creator.c_str()));
    _ableton_node->append_attribute(_xml_doc.allocate_attribute("Revision", proj.revision.c_str()));
    _xml_doc.append_node(_ableton_node);
    {
        // liveset
        xml_node* _liveset_node = xml_create_node(_xml_doc, _ableton_node, "LiveSet");
        {
            // ids etc
            if (ver >= version::v_11_0_0) {
                // xml_create_node(_xml_doc, _liveset_node, "NextPointeeId", { { "Value", "36073" } });
            }
            xml_create_node(_xml_doc, _liveset_node, "OverwriteProtectionNumber", "3072");
            xml_create_node(_xml_doc, _liveset_node, "LomId", "0");
            xml_create_node(_xml_doc, _liveset_node, "LomIdView", "0");

            // tracks
            xml_node* _tracks_node = xml_create_node(_xml_doc, _liveset_node, "Tracks");
            for (const project::user_track& _track : proj.tracks) {

                // auto& _audio_track = std::get<project::audio_track>(_track);

                // ids etc
                xml_node* _track_node = xml_create_node(_xml_doc, _tracks_node, "AudioTrack");
                std::visit([&](auto& _track_visit) {
                    using _track_type_t = std::decay_t<decltype(_track_visit)>;

                    // track id
                    xml_create_attribute(_xml_doc, _track_node, "Id", _track_visit.id);

                    // diverse ids
                    xml_create_node(_xml_doc, _track_node, "LomId", "0");
                    xml_create_node(_xml_doc, _track_node, "LomIdView", "0");

                    // xml_create_node(_xml_doc, _track_node, "IsContentSelectedInDocument", { { "Value",  } });
                    // xml_create_node(_xml_doc, _track_node, "PreferredContentViewMode", { { "Value", "0" } });

                    // track delay
                    xml_node* _track_delay_node = xml_create_node(_xml_doc, _track_node, "TrackDelay");
                    {
                        xml_create_node_and_value(_xml_doc, _track_delay_node, "Value", _track_visit.track_delay_value);
                        xml_create_node_and_value(_xml_doc, _track_delay_node, "IsValueSampleBased", _track_visit.track_delay_is_value_sample_based);
                    }

                    // name
                    xml_node* _track_name_node = xml_create_node(_xml_doc, _track_node, "Name");
                    {
                        xml_create_node_and_value(_xml_doc, _track_name_node, "EffectiveName", _track_visit.effective_name);
                        xml_create_node_and_value(_xml_doc, _track_name_node, "UserName", _track_visit.user_name);
                        xml_create_node_and_value(_xml_doc, _track_name_node, "Annotation", _track_visit.annotation);
                        if (ver >= version::v_12_0_0) {
                            xml_create_node_and_value(_xml_doc, _track_name_node, "MemorizedFirstClipName", _track_visit.memorized_first_clip_name.value());
                        }
                    }

                    // color
                    if (ver >= version::v_12_0_0) {
                        xml_create_node_and_value(_xml_doc, _track_node, "Color", _track_visit.color.value());
                    } else {
                        xml_create_node_and_value(_xml_doc, _track_node, "ColorIndex", _track_visit.color_index.value());
                    }

                    // xml_create_node(_xml_doc, xml_create_node(_xml_doc, _track_node, "AutomationEnvelopes"), "Envelopes");

                    xml_create_node_and_value(_xml_doc, _track_node, "TrackGroupId", _track_visit.track_group_id); // -1 vers master
                    xml_create_node_and_value(_xml_doc, _track_node, "TrackUnfolded", _track_visit.track_unfolded);
                    // xml_create_node_and_value(_xml_doc, _track_node, "DevicesListWrapper", { { "LomId", "0" } });
                    // xml_create_node_and_value(_xml_doc, _track_node, "ClipSlotsListWrapper", { { "LomId", "0" } });
                    // xml_create_node_and_value(_xml_doc, _track_node, "ViewData", { { "Value", "{}" } });

                    // take lanes for 12
                    // xml_node* _take_lanes_node = xml_create_node(_xml_doc, _track_node, "TakeLanes");
                    // {
                    //     xml_create_node(_xml_doc, _take_lanes_node, "TakeLanes");
                    //     xml_create_node(_xml_doc, _take_lanes_node, "AreTakeLanesFolded", { { "Value", "true" } });
                    // }

                    // xml_create_node(_xml_doc, _track_node, "LinkedTrackGroupId", { { "Value", "-1" } });
                    xml_create_node_and_value(_xml_doc, _track_node, "SavedPlayingSlot", _track_visit.saved_playing_slot);
                    xml_create_node_and_value(_xml_doc, _track_node, "SavedPlayingOffset", _track_visit.saved_playing_offset);
                    xml_create_node_and_value(_xml_doc, _track_node, "MidiFoldIn", _track_visit.midi_fold_in); // 9 only
                    xml_create_node_and_value(_xml_doc, _track_node, "MidiPrelisten", _track_visit.midi_prelisten); // 9 only
                    xml_create_node_and_value(_xml_doc, _track_node, "Freeze", _track_visit.freeze);
                    xml_create_node_and_value(_xml_doc, _track_node, "VelocityDetail", _track_visit.velocity_detail);
                    xml_create_node_and_value(_xml_doc, _track_node, "NeedArrangerRefreeze", _track_visit.need_arranger_refreeze);
                    xml_create_node_and_value(_xml_doc, _track_node, "PostProcessFreezeClips", _track_visit.post_process_freeze_clips);
                    xml_create_node_and_value(_xml_doc, _track_node, "MidiTargetPrefersFoldOrIsNotUniform", _track_visit.midi_target_prefers_fold_or_is_not_uniform);

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
                },
                    _track);
            }

            // master/main track TODO

            // prehear track TODO

            // sends pre
            xml_node* _sends_pre_node = xml_create_node(_xml_doc, _liveset_node, "SendsPre");
            for (const bool _send_pre : proj.sends_pre) {
                xml_create_node_and_value(_xml_doc, _sends_pre_node, "SendPreBool", _send_pre);
            }

            // scenes TODO??

            // transport
            xml_node* _transport_node = xml_create_node(_xml_doc, _liveset_node, "Transport");
            {
                xml_create_node_and_value(_xml_doc, _transport_node, "PhaseNudgeTempo", proj.transport_phase_nudge_tempo);
                xml_create_node_and_value(_xml_doc, _transport_node, "LoopOn", proj.transport_loop_on);
                xml_create_node_and_value(_xml_doc, _transport_node, "LoopStart", proj.transport_loop_start);
                xml_create_node_and_value(_xml_doc, _transport_node, "LoopLength", proj.transport_loop_length);
                xml_create_node_and_value(_xml_doc, _transport_node, "LoopIsSongStart", proj.transport_loop_is_song_start);
                xml_create_node_and_value(_xml_doc, _transport_node, "CurrentTime", proj.transport_current_time);
                xml_create_node_and_value(_xml_doc, _transport_node, "PunchIn", proj.transport_punch_in);
                xml_create_node_and_value(_xml_doc, _transport_node, "PunchOut", proj.transport_punch_out);
                xml_create_node_and_value(_xml_doc, _transport_node, "DrawMode", proj.transport_draw_mode);
                if (ver < version::v_12_0_0) {
                    xml_create_node_and_value(_xml_doc, _transport_node, "ComputerKeyboardIsEnabled", proj.transport_computer_keyboard_is_enabled.value());
                }
            }

            // scroll pos TODO

            // quantisation
            xml_create_node_and_value(_xml_doc, _liveset_node, "GlobalQuantisation", proj.global_quantisation);
            xml_create_node_and_value(_xml_doc, _liveset_node, "AutoQuantisation", proj.auto_quantisation);

            // grid
            xml_node* _grid_node = xml_create_node(_xml_doc, _liveset_node, "Grid");
            {
                xml_create_node_and_value(_xml_doc, _grid_node, "FixedNumerator", proj.grid_fixed_numerator);
                xml_create_node_and_value(_xml_doc, _grid_node, "FixedDenominator", proj.grid_fixed_denominator);
                xml_create_node_and_value(_xml_doc, _grid_node, "GridIntervalPixel", proj.grid_grid_interval_pixel);
                xml_create_node_and_value(_xml_doc, _grid_node, "Ntoles", proj.grid_ntoles);
                xml_create_node_and_value(_xml_doc, _grid_node, "SnapToGrid", proj.grid_snap_to_grid);
                xml_create_node_and_value(_xml_doc, _grid_node, "Fixed", proj.grid_fixed);
            }

            xml_node* _scale_info_node = xml_create_node(_xml_doc, _liveset_node, "ScaleInformation");
            xml_create_node_and_value(_xml_doc, _scale_info_node, "RootNote", proj.scale_information_root_note);
            xml_create_node_and_value(_xml_doc, _scale_info_node, "Name", proj.scale_information_name);

            xml_create_node_and_value(_xml_doc, _liveset_node, "SmpteFormat", proj.smpte_format);

            xml_node* _time_selection_node = xml_create_node(_xml_doc, _liveset_node, "TimeSelection");
            xml_create_node_and_value(_xml_doc, _time_selection_node, "AnchorTime", proj.time_selection_anchor_time);
            xml_create_node_and_value(_xml_doc, _time_selection_node, "OtherTime", proj.time_selection_other_time);

            // sequencer navigator
            xml_node* _sequencer_nav_node = xml_create_node(_xml_doc, _liveset_node, "SequencerNavigator");
            {
                // beat time helper
                xml_node* _beat_time_helper = xml_create_node(_xml_doc, _sequencer_nav_node, "BeatTimeHelper");
                {
                    xml_create_node_and_value(_xml_doc, _beat_time_helper, "CurrentZoom", proj.project_sequencer_navigator.beat_time.current_zoom);
                }

                // scroller position
                xml_node* _scroller_pos_node = xml_create_node(_xml_doc, _sequencer_nav_node, "ScrollerPos");
                {
                    xml_create_attribute(_xml_doc, _scroller_pos_node, "X", proj.project_sequencer_navigator.scroller_pos_x);
                    xml_create_attribute(_xml_doc, _scroller_pos_node, "Y", proj.project_sequencer_navigator.scroller_pos_y);
                }

                // client size
                xml_node* _client_size_node = xml_create_node(_xml_doc, _sequencer_nav_node, "ClientSize");
                {
                    xml_create_attribute(_xml_doc, _client_size_node, "X", proj.project_sequencer_navigator.client_size_x);
                    xml_create_attribute(_xml_doc, _client_size_node, "Y", proj.project_sequencer_navigator.client_size_y);
                }
            }

            // view states
            if (ver < version::v_12_0_0) {
                xml_create_node_and_value(_xml_doc, _liveset_node, "ViewStateLaunchPanel", proj.view_state_launch_panel.value());
                xml_create_node_and_value(_xml_doc, _liveset_node, "ViewStateEnvelopePanel", proj.view_state_envelope_panel.value());
                xml_create_node_and_value(_xml_doc, _liveset_node, "ViewStateSamplePanel", proj.view_state_sample_panel.value());
            }

            // content splitter properties
            if (ver < version::v_12_0_0) {
                xml_node* _content_splitter_node = xml_create_node(_xml_doc, _liveset_node, "ContentSplitterProperties");
                {
                    xml_create_node_and_value(_xml_doc, _content_splitter_node, "Open", proj.content_splitter_properties_open.value());
                    xml_create_node_and_value(_xml_doc, _content_splitter_node, "Size", proj.content_splitter_properties_size.value());
                }
            }

            // view states
            xml_create_node_and_value(_xml_doc, _liveset_node, "ViewStateFxSlotCount", proj.view_state_fx_slot_count);
            xml_create_node_and_value(_xml_doc, _liveset_node, "ViewStateSessionMixerHeight", proj.view_state_session_mixer_height);

            // locators TODO??

            // detail clip keys midi
            // tracks list wrapper
            // visible tracks list wrapper
            // return tracks list wrapper
            // scenes list wrapper
            // cue points list wrapper

            // chooser bar
            xml_create_node_and_value(_xml_doc, _liveset_node, "ChooserBar", proj.chooser_bar);

            // annotation
            xml_create_node_and_value(_xml_doc, _liveset_node, "Annotation", proj.annotation);

            // solo
            xml_create_node_and_value(_xml_doc, _liveset_node, "SoloOrPflSavedValue", proj.solo_or_pfl_saved_value);
            xml_create_node_and_value(_xml_doc, _liveset_node, "SoloInPlace", proj.solo_in_place);

            // crossfade curve
            xml_create_node_and_value(_xml_doc, _liveset_node, "CrossfadeCurve", proj.crossfade_curve);

            // latency compensation
            xml_create_node_and_value(_xml_doc, _liveset_node, "LatencyCompensation", proj.latency_compensation);

            // highlighted track index
            xml_create_node_and_value(_xml_doc, _liveset_node, "HighlightedTrackIndex", proj.highlighted_track_index);

            // grooves TODO
        }
    }

    // compress
    std::string _xml_data = xml_to_string(_xml_doc);
    std::cout << _xml_data;
    gz_compress(stream, _xml_data);
}

}