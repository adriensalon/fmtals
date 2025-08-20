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

xml_node* xml_get_node(const xml_node* parent_node, const std::string& child_name)
{
    return parent_node->first_node(child_name.c_str(), child_name.length());
}

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

template <typename T>
void xml_get_value(const xml_node* node, const std::string& attribute, T& value)
{
    xml_attribute* _attribute = node->first_attribute(attribute.c_str(), attribute.length());
    std::string _str = _attribute->value();
    if constexpr (std::is_same_v<T, bool>) {
        if (_str == "true" || _str == "1") {
            value = true;
        } else if (_str == "false" || _str == "0") {
            value = false;
        } else {
            throw std::invalid_argument("Expected 'true', '1', 'false', '0', got: " + _str);
        }
    } else if constexpr (std::is_same_v<T, std::string>) {
        value = _str;
    } else if constexpr (std::is_same_v<T, float>) {
        value = std::stof(_str);
    } else if constexpr (std::is_same_v<T, double>) {
        value = std::stod(_str);
    } else if constexpr (std::is_integral_v<T>) {
        if constexpr (std::is_signed_v<T>) {
            value = static_cast<T>(std::stoll(_str));
        } else {
            value = static_cast<T>(std::stoull(_str));
        }
    } else {
        static_assert(!sizeof(T*), "xml_get_value: unsupported T");
    }
}

template <typename T>
void xml_get_node_and_value(const xml_node* node, const std::string& child_name, T& value)
{
    xml_get_value(xml_get_node(node, child_name), "Value", value);
}

xml_node* xml_create_node(xml_document& document, xml_node* node, const std::string& child_name, cereal::rapidxml::node_type child_type = cereal::rapidxml::node_element)
{
    char* _allocated_name = document.allocate_string(child_name.c_str());
    xml_node* _child = document.allocate_node(child_type, _allocated_name);
    if (node) {
        node->append_node(_child);
    } else {
        document.append_node(_child);
    }
    return _child;
}

template <typename T>
void xml_create_value(xml_document& document, xml_node* node, const std::string& attribute, const T& value)
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
void xml_create_node_and_value(xml_document& document, xml_node* parent, const std::string& child_name, const T& value)
{
    xml_create_value(document, xml_create_node(document, parent, child_name), "Value", value);
}

// version

static fmtals::version detect_version(const std::string& creator)
{
    unsigned maj = 0, min = 0, pat = 0;
    if (sscanf_s(creator.c_str(), "Ableton Live %u.%u.%u", &maj, &min, &pat) == 3) {
        if (maj == 9 && min == 7 && pat == 7)
            return fmtals::version::v_9_7_7;
        if (maj == 11 && min == 0 && pat == 0)
            return fmtals::version::v_11_0_0;
        if (maj == 12 && min == 0 && pat == 0)
            return fmtals::version::v_12_0_0;
        if (maj == 9 && min == 0 && pat == 0)
            return fmtals::version::v_9_0_0;
        if (maj == 9 && min == 1 && pat == 0)
            return fmtals::version::v_9_1_0;
        if (maj == 9 && min == 2 && pat == 0)
            return fmtals::version::v_9_2_0;
    }
    throw std::runtime_error("Unimplemented Ableton Live version");
}

namespace fmtals {

void import_project(std::istream& stream, project& proj, version& ver)
{
    xml_document _xml_doc;
    std::string _xml_data;
    gz_decompress(stream, _xml_data);
    _xml_doc.parse<0>(_xml_data.data());

    xml_node* _ableton_node = _xml_doc.first_node("Ableton");
    xml_get_value(_ableton_node, "MajorVersion", proj.major_version);
    xml_get_value(_ableton_node, "MinorVersion", proj.minor_version);
    xml_get_value(_ableton_node, "Creator", proj.creator);
    xml_get_value(_ableton_node, "Revision", proj.revision);
    ver = detect_version(proj.creator);
    if (ver >= version::v_11_0_0) {
        xml_get_value(_ableton_node, "SchemaChangeCount", proj.schema_change_count.emplace());
    }

    xml_node* _liveset_node = xml_get_node(_ableton_node, "LiveSet");
    xml_get_node_and_value(_liveset_node, "OverwriteProtectionNumber", proj.overwrite_protection_number);
    xml_get_node_and_value(_liveset_node, "LomId", proj.lom_id);
    xml_get_node_and_value(_liveset_node, "LomIdView", proj.lom_id_view);

    for (xml_node* _track_node : xml_get_nodes(xml_get_node(_liveset_node, "Tracks"))) {
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

        std::visit([&](auto& _track_visit) {
            using _track_type_t = std::decay_t<decltype(_track_visit)>;

            xml_get_value(_track_node, "Id", _track_visit.id);
            xml_get_node_and_value(_track_node, "LomId", _track_visit.lom_id);
            xml_get_node_and_value(_track_node, "LomIdView", _track_visit.lom_id_view);
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
                xml_get_node_and_value(_track_node, "SavedPlayingOffset", _track_visit.saved_playing_offset);
                xml_get_node_and_value(_track_node, "MidiFoldIn", _track_visit.midi_fold_in);
                xml_get_node_and_value(_track_node, "MidiPrelisten", _track_visit.midi_prelisten);
                xml_get_node_and_value(_track_node, "Freeze", _track_visit.freeze);
                xml_get_node_and_value(_track_node, "VelocityDetail", _track_visit.velocity_detail);
                xml_get_node_and_value(_track_node, "NeedArrangerRefreeze", _track_visit.need_arranger_refreeze);
                xml_get_node_and_value(_track_node, "PostProcessFreezeClips", _track_visit.post_process_freeze_clips);
                xml_get_node_and_value(_track_node, "MidiTargetPrefersFoldOrIsNotUniform", _track_visit.midi_target_prefers_fold_or_is_not_uniform);
            }

            xml_node* _device_chain_node = xml_get_node(_track_node, "DeviceChain");
            // TODO mixer
            xml_node* _main_sequencer_node = xml_get_node(_device_chain_node, "MainSequencer");
            xml_node* _sample_node = xml_get_node(_main_sequencer_node, "Sample");
            xml_node* _arranger_automation_node = xml_get_node(_sample_node, "ArrangerAutomation");

            // events
            for (xml_node* _event_node : xml_get_nodes(xml_get_node(_arranger_automation_node, "Events"))) {

                // audio events
                if constexpr (std::is_same_v<_track_type_t, project::audio_track>) {
                    if (std::string(_event_node->name()) == "AudioClip") {
                        project::audio_clip _audio_clip = _track_visit.events_audio_clips.emplace_back();
                        xml_get_value(_event_node, "Time", _audio_clip.time);
                        xml_get_node_and_value(_event_node, "LomId", _audio_clip.lom_id);
                        xml_get_node_and_value(_event_node, "LomIdView", _audio_clip.lom_id_view);

                        for (xml_node* _warp_marker_node : xml_get_nodes(xml_get_node(_event_node, "WarpMarkers"))) {
                            project::warp_marker& _warp_marker = _audio_clip.warp_markers.emplace_back();
                            xml_get_value(_warp_marker_node, "SecTime", _warp_marker.sec_time);
                            xml_get_value(_warp_marker_node, "BeatTime", _warp_marker.beat_time);
                        }

                        xml_get_node_and_value(_event_node, "MarkersGenerated", _audio_clip.markers_generated);
                        xml_get_node_and_value(_event_node, "CurrentStart", _audio_clip.current_start);
                        xml_get_node_and_value(_event_node, "CurrentEnd", _audio_clip.current_end);

                        xml_node* _loop_node = xml_get_node(_event_node, "Loop");
                        xml_get_node_and_value(_loop_node, "LoopStart", _audio_clip.loop_start);
                        xml_get_node_and_value(_loop_node, "LoopEnd", _audio_clip.loop_end);
                        xml_get_node_and_value(_loop_node, "StartRelative", _audio_clip.loop_start_relative);
                        xml_get_node_and_value(_loop_node, "LoopOn", _audio_clip.loop_on);
                        xml_get_node_and_value(_loop_node, "OutMarker", _audio_clip.loop_out_marker);
                        xml_get_node_and_value(_loop_node, "HiddenLoopStart", _audio_clip.hidden_loop_start);
                        xml_get_node_and_value(_loop_node, "HiddenLoopEnd", _audio_clip.hidden_loop_end);

                        xml_get_node_and_value(_event_node, "Name", _audio_clip.name);
                        xml_get_node_and_value(_event_node, "Annotation", _audio_clip.annotation);
                        if (ver >= version::v_12_0_0) {
                            xml_get_node_and_value(_event_node, "Color", _audio_clip.color.emplace());
                        } else {
                            xml_get_node_and_value(_event_node, "ColorIndex", _audio_clip.color_index.emplace());
                        }
                        xml_get_node_and_value(_event_node, "LaunchMode", _audio_clip.launch_mode);
                        xml_get_node_and_value(_event_node, "LaunchQuantisation", _audio_clip.launch_quantisation);

                        // TODO time signature

                        // TODO envelopes

                        // TODO ScrollerTimePreserver

                        // TODO TimeSelection

                        xml_get_node_and_value(_event_node, "Legato", _audio_clip.legato);
                        xml_get_node_and_value(_event_node, "Ram", _audio_clip.ram);
                        // groove settings ?
                        xml_get_node_and_value(_event_node, "Disabled", _audio_clip.disabled);
                        xml_get_node_and_value(_event_node, "VelocityAmount", _audio_clip.velocity_amount);
                        xml_get_node_and_value(_event_node, "FollowTime", _audio_clip.follow_time);
                        xml_get_node_and_value(_event_node, "FollowActionA", _audio_clip.follow_action_a);
                        xml_get_node_and_value(_event_node, "FollowActionB", _audio_clip.follow_action_b);
                        xml_get_node_and_value(_event_node, "FollowChanceA", _audio_clip.follow_chance_a);
                        xml_get_node_and_value(_event_node, "FollowChanceB", _audio_clip.follow_chance_b);

                        // grid
                        xml_node* _grid_node = xml_get_node(_event_node, "Grid");
                        xml_get_node_and_value(_grid_node, "FixedNumerator", _audio_clip.grid_fixed_numerator);
                        xml_get_node_and_value(_grid_node, "FixedDenominator", _audio_clip.grid_fixed_denominator);
                        xml_get_node_and_value(_grid_node, "GridIntervalPixel", _audio_clip.grid_interval_pixel);
                        xml_get_node_and_value(_grid_node, "Ntoles", _audio_clip.grid_ntoles);
                        xml_get_node_and_value(_grid_node, "SnapToGrid", _audio_clip.grid_snap_to_grid);
                        xml_get_node_and_value(_grid_node, "Fixed", _audio_clip.grid_fixed);

                        xml_get_node_and_value(_event_node, "FreezeStart", _audio_clip.freeze_start);
                        xml_get_node_and_value(_event_node, "FreezeEnd", _audio_clip.freeze_end);
                        xml_get_node_and_value(_event_node, "IsSongTempoMaster", _audio_clip.is_song_tempo_master);
                        xml_get_node_and_value(_event_node, "IsWarped", _audio_clip.is_warped);

                        // TODO many ahah
                    }
                }
            }
        },
            _user_track);

        proj.tracks.emplace_back(_user_track);
    }

    if (ver >= version::v_12_0_0) {
        xml_node* _main_track = xml_get_node(_liveset_node, "MainTrack");
        // TODO

    } else {
        xml_node* _master_track = xml_get_node(_liveset_node, "MasterTrack");
        // TODO
    }

    xml_node* _pre_hear_track = xml_get_node(_liveset_node, "PreHearTrack");
    // TODO

    for (xml_node* _sends_pre_node : xml_get_nodes(xml_get_node(_liveset_node, "SendsPre"))) {
        // TODO
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
    if (ver >= version::v_12_0_0) {
        // TODO metronome_tick_duration for 12
    }
    xml_get_node_and_value(_transport_node, "DrawMode", proj.transport_draw_mode);
    if (ver < version::v_12_0_0) {
        xml_get_node_and_value(_transport_node, "ComputerKeyboardIsEnabled", proj.transport_computer_keyboard_is_enabled.emplace());
    }

    xml_node* _song_master_values_node = xml_get_node(_liveset_node, "SongMasterValues");
    xml_node* _session_scroller_pos_node = xml_get_node(_song_master_values_node, "SessionScrollerPos");
    xml_get_value(_session_scroller_pos_node, "X", proj.song_master_values_scroll_pos_x);
    xml_get_value(_session_scroller_pos_node, "Y", proj.song_master_values_scroll_pos_y);

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

    xml_node* _sequencer_navigator_node = xml_get_node(_liveset_node, "SequencerNavigator");
    xml_node* _beat_time_helper_node = xml_get_node(_sequencer_navigator_node, "BeatTimeHelper");
    xml_get_node_and_value(_beat_time_helper_node, "CurrentZoom", proj.sequencer_navigator_current_zoom);
    xml_node* _scroller_pos_node = xml_get_node(_sequencer_navigator_node, "ScrollerPos");
    xml_get_value(_scroller_pos_node, "X", proj.sequencer_navigator_scroller_pos_x);
    xml_get_value(_scroller_pos_node, "Y", proj.sequencer_navigator_scroller_pos_y);
    xml_node* _client_size_node = xml_get_node(_sequencer_navigator_node, "ClientSize");
    xml_get_value(_client_size_node, "X", proj.sequencer_navigator_client_size_x);
    xml_get_value(_client_size_node, "Y", proj.sequencer_navigator_client_size_y);

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
    // AutoColorPickerForPlayerAndGroupTracks TODO
    // AutoColorPickerForReturnAndMasterTracks TODO
    // ViewData TODO
    // UseWarperLegacyHiQMode TODO
    // VideoWindowRect TODO
    // ShowVideoWindow TODO
    // TrackHeaderWidth TODO
    // ViewStateArrangerHasDetail TODO
    // ViewStateSessionHasDetail TODO
    // ViewStateDetailIsSample TODO
    // ViewStates TODO
}

void export_project(std::ostream& stream, const project& proj, const version& ver)
{
    xml_document _xml_doc;
    xml_node* _declaration_node = xml_create_node(_xml_doc, nullptr, std::string(), cereal::rapidxml::node_declaration);
    xml_create_value(_xml_doc, _declaration_node, "version", std::string("1.0"));
    xml_create_value(_xml_doc, _declaration_node, "encoding", std::string("UTF-8"));

    xml_node* _ableton_node = xml_create_node(_xml_doc, nullptr, "Ableton");
    xml_create_value(_xml_doc, _ableton_node, "MajorVersion", proj.major_version);
    xml_create_value(_xml_doc, _ableton_node, "MinorVersion", proj.minor_version);
    if (ver >= version::v_11_0_0) {
        xml_create_value(_xml_doc, _ableton_node, "SchemaChangeCount", proj.schema_change_count.value());
    }
    xml_create_value(_xml_doc, _ableton_node, "Creator", proj.creator);
    xml_create_value(_xml_doc, _ableton_node, "Revision", proj.revision);

    // liveset
    xml_node* _liveset_node = xml_create_node(_xml_doc, _ableton_node, "LiveSet");
    if (ver >= version::v_11_0_0) {
        // xml_create_node_and_value(_xml_doc, _liveset_node, "NextPointeeId", std::string("36073"));
    }
    xml_create_node_and_value(_xml_doc, _liveset_node, "OverwriteProtectionNumber", proj.overwrite_protection_number);
    xml_create_node_and_value(_xml_doc, _liveset_node, "LomId", proj.lom_id);
    xml_create_node_and_value(_xml_doc, _liveset_node, "LomIdView", proj.lom_id_view);

    // tracks
    xml_node* _tracks_node = xml_create_node(_xml_doc, _liveset_node, "Tracks");
    for (const project::user_track& _track : proj.tracks) {

        // track visit
        xml_node* _track_node = xml_create_node(_xml_doc, _tracks_node, "AudioTrack");
        std::visit([&](auto& _track_visit) {
            using _track_type_t = std::decay_t<decltype(_track_visit)>;

            xml_create_value(_xml_doc, _track_node, "Id", _track_visit.id);
            xml_create_node_and_value(_xml_doc, _track_node, "LomId", _track_visit.lom_id);
            xml_create_node_and_value(_xml_doc, _track_node, "LomIdView", _track_visit.lom_id_view);

            // TODO IsContentSelectedInDocument
            // TODO PreferredContentViewMode

            // track delay
            xml_node* _track_delay_node = xml_create_node(_xml_doc, _track_node, "TrackDelay");
            xml_create_node_and_value(_xml_doc, _track_delay_node, "Value", _track_visit.track_delay_value);
            xml_create_node_and_value(_xml_doc, _track_delay_node, "IsValueSampleBased", _track_visit.track_delay_is_value_sample_based);

            // name
            xml_node* _track_name_node = xml_create_node(_xml_doc, _track_node, "Name");
            xml_create_node_and_value(_xml_doc, _track_name_node, "EffectiveName", _track_visit.effective_name);
            xml_create_node_and_value(_xml_doc, _track_name_node, "UserName", _track_visit.user_name);
            xml_create_node_and_value(_xml_doc, _track_name_node, "Annotation", _track_visit.annotation);
            if (ver >= version::v_12_0_0) {
                xml_create_node_and_value(_xml_doc, _track_name_node, "MemorizedFirstClipName", _track_visit.memorized_first_clip_name.value());
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
                xml_node* _automation_lane_0_node = xml_create_node(_xml_doc, _automation_lanes_child_node, "AutomationLane");
                xml_create_value(_xml_doc, _automation_lane_0_node, "Id", 0);
                xml_create_value(_xml_doc, _automation_lane_0_node, "SelectedDevice", 0);
                xml_create_value(_xml_doc, _automation_lane_0_node, "SelectedEnvelope", 0);
                xml_create_value(_xml_doc, _automation_lane_0_node, "IsContentSelectedInDocument", false);
                xml_create_value(_xml_doc, _automation_lane_0_node, "LaneHeight", 68);
                xml_create_value(_xml_doc, _automation_lane_0_node, "AreAdditionalAutomationLanesFolded", false);

                // clip envelope chooser
                // xml_node* _clip_envelope_chooser_node = xml_create_node(_xml_doc, _device_chain_node, "ClipEnvelopeChooserViewState");
                // {
                //     xml_create_node(_xml_doc, _clip_envelope_chooser_node, "SelectedDevice", { { "Value", "0" } });
                //     xml_create_node(_xml_doc, _clip_envelope_chooser_node, "SelectedEnvelope", { { "Value", "0" } });
                //     xml_create_node(_xml_doc, _clip_envelope_chooser_node, "PreferModulationVisible", { { "Value", "false" } });
                // }

                // // audio input routing
                // xml_node* _audio_input_routing_node = xml_create_node(_xml_doc, _device_chain_node, "AudioInputRouting");
                // {
                //     xml_create_node(_xml_doc, _audio_input_routing_node, "Target", { { "Value", "AudioIn/External/M0" } });
                //     xml_create_node(_xml_doc, _audio_input_routing_node, "UpperDisplayString", { { "Value", "Ext. In" } });
                //     xml_create_node(_xml_doc, _audio_input_routing_node, "LowerDisplayString", { { "Value", "1" } });

                //     // mpe settings
                //     xml_node* _mpe_settings_node = xml_create_node(_xml_doc, _audio_input_routing_node, "MpeSettings");
                //     {
                //         xml_create_node(_xml_doc, _mpe_settings_node, "ZoneType", { { "Value", "0" } });
                //         xml_create_node(_xml_doc, _mpe_settings_node, "FirstNoteChannel", { { "Value", "1" } });
                //         xml_create_node(_xml_doc, _mpe_settings_node, "LastNoteChannel", { { "Value", "15" } });
                //     }
                // }

                // // midi input routing
                // xml_node* _midi_input_routing_node = xml_create_node(_xml_doc, _device_chain_node, "MidiInputRouting");
                // {
                //     xml_create_node(_xml_doc, _midi_input_routing_node, "Target", { { "Value", "MidiIn/External.All/-1" } });
                //     xml_create_node(_xml_doc, _midi_input_routing_node, "UpperDisplayString", { { "Value", "Ext: All Ins" } });
                //     xml_create_node(_xml_doc, _midi_input_routing_node, "LowerDisplayString", { { "Value", "" } });

                //     // mpe settings
                //     xml_node* _mpe_settings_node = xml_create_node(_xml_doc, _midi_input_routing_node, "MpeSettings");
                //     {
                //         xml_create_node(_xml_doc, _mpe_settings_node, "ZoneType", { { "Value", "0" } });
                //         xml_create_node(_xml_doc, _mpe_settings_node, "FirstNoteChannel", { { "Value", "1" } });
                //         xml_create_node(_xml_doc, _mpe_settings_node, "LastNoteChannel", { { "Value", "15" } });
                //     }
                // }

                // // audio output routing
                // xml_node* _audio_output_routing_node = xml_create_node(_xml_doc, _device_chain_node, "AudioOutputRouting");
                // {
                //     xml_create_node(_xml_doc, _audio_output_routing_node, "Target", { { "Value", "AudioOut/Main" } });
                //     xml_create_node(_xml_doc, _audio_output_routing_node, "UpperDisplayString", { { "Value", "Main" } });
                //     xml_create_node(_xml_doc, _audio_output_routing_node, "LowerDisplayString", { { "Value", "" } });

                //     // mpe settings
                //     xml_node* _mpe_settings_node = xml_create_node(_xml_doc, _audio_output_routing_node, "MpeSettings");
                //     {
                //         xml_create_node(_xml_doc, _mpe_settings_node, "ZoneType", { { "Value", "0" } });
                //         xml_create_node(_xml_doc, _mpe_settings_node, "FirstNoteChannel", { { "Value", "1" } });
                //         xml_create_node(_xml_doc, _mpe_settings_node, "LastNoteChannel", { { "Value", "15" } });
                //     }
                // }

                // // midi output routing
                // xml_node* _midi_output_routing_node = xml_create_node(_xml_doc, _device_chain_node, "MidiOutputRouting");
                // {
                //     xml_create_node(_xml_doc, _midi_output_routing_node, "Target", { { "Value", "MidiOut/None" } });
                //     xml_create_node(_xml_doc, _midi_output_routing_node, "UpperDisplayString", { { "Value", "None" } });
                //     xml_create_node(_xml_doc, _midi_output_routing_node, "LowerDisplayString", { { "Value", "" } });

                //     // mpe settings
                //     xml_node* _mpe_settings_node = xml_create_node(_xml_doc, _midi_output_routing_node, "MpeSettings");
                //     {
                //         xml_create_node(_xml_doc, _mpe_settings_node, "ZoneType", { { "Value", "0" } });
                //         xml_create_node(_xml_doc, _mpe_settings_node, "FirstNoteChannel", { { "Value", "1" } });
                //         xml_create_node(_xml_doc, _mpe_settings_node, "LastNoteChannel", { { "Value", "15" } });
                //     }
                // }

                // mixer
                xml_node* _mixer_node = xml_create_node(_xml_doc, _device_chain_node, "Mixer");
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

    xml_node* _transport_node = xml_create_node(_xml_doc, _liveset_node, "Transport");
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

    xml_node* _song_master_values_node = xml_create_node(_xml_doc, _liveset_node, "SongMasterValues");
    xml_node* _session_scroller_pos_node = xml_create_node(_xml_doc, _song_master_values_node, "SessionScrollerPos");
    xml_create_value(_xml_doc, _session_scroller_pos_node, "X", proj.song_master_values_scroll_pos_x);
    xml_create_value(_xml_doc, _session_scroller_pos_node, "Y", proj.song_master_values_scroll_pos_y);

    xml_create_node_and_value(_xml_doc, _liveset_node, "GlobalQuantisation", proj.global_quantisation);
    xml_create_node_and_value(_xml_doc, _liveset_node, "AutoQuantisation", proj.auto_quantisation);

    xml_node* _grid_node = xml_create_node(_xml_doc, _liveset_node, "Grid");
    xml_create_node_and_value(_xml_doc, _grid_node, "FixedNumerator", proj.grid_fixed_numerator);
    xml_create_node_and_value(_xml_doc, _grid_node, "FixedDenominator", proj.grid_fixed_denominator);
    xml_create_node_and_value(_xml_doc, _grid_node, "GridIntervalPixel", proj.grid_grid_interval_pixel);
    xml_create_node_and_value(_xml_doc, _grid_node, "Ntoles", proj.grid_ntoles);
    xml_create_node_and_value(_xml_doc, _grid_node, "SnapToGrid", proj.grid_snap_to_grid);
    xml_create_node_and_value(_xml_doc, _grid_node, "Fixed", proj.grid_fixed);

    xml_node* _scale_info_node = xml_create_node(_xml_doc, _liveset_node, "ScaleInformation");
    xml_create_node_and_value(_xml_doc, _scale_info_node, "RootNote", proj.scale_information_root_note);
    xml_create_node_and_value(_xml_doc, _scale_info_node, "Name", proj.scale_information_name);

    xml_create_node_and_value(_xml_doc, _liveset_node, "SmpteFormat", proj.smpte_format);

    xml_node* _time_selection_node = xml_create_node(_xml_doc, _liveset_node, "TimeSelection");
    xml_create_node_and_value(_xml_doc, _time_selection_node, "AnchorTime", proj.time_selection_anchor_time);
    xml_create_node_and_value(_xml_doc, _time_selection_node, "OtherTime", proj.time_selection_other_time);

    xml_node* _sequencer_navigator_node = xml_create_node(_xml_doc, _liveset_node, "SequencerNavigator");
    xml_node* _beat_time_helper_node = xml_create_node(_xml_doc, _sequencer_navigator_node, "BeatTimeHelper");
    xml_create_node_and_value(_xml_doc, _beat_time_helper_node, "CurrentZoom", proj.sequencer_navigator_current_zoom);
    xml_node* _scroller_pos_node = xml_create_node(_xml_doc, _sequencer_navigator_node, "ScrollerPos");
    xml_create_value(_xml_doc, _scroller_pos_node, "X", proj.sequencer_navigator_scroller_pos_x);
    xml_create_value(_xml_doc, _scroller_pos_node, "Y", proj.sequencer_navigator_scroller_pos_y);
    xml_node* _client_size_node = xml_create_node(_xml_doc, _sequencer_navigator_node, "ClientSize");
    xml_create_value(_xml_doc, _client_size_node, "X", proj.sequencer_navigator_client_size_x);
    xml_create_value(_xml_doc, _client_size_node, "Y", proj.sequencer_navigator_client_size_y);

    if (ver < version::v_12_0_0) {
        xml_create_node_and_value(_xml_doc, _liveset_node, "ViewStateLaunchPanel", proj.view_state_launch_panel.value());
        xml_create_node_and_value(_xml_doc, _liveset_node, "ViewStateEnvelopePanel", proj.view_state_envelope_panel.value());
        xml_create_node_and_value(_xml_doc, _liveset_node, "ViewStateSamplePanel", proj.view_state_sample_panel.value());
    }

    if (ver < version::v_12_0_0) {
        xml_node* _content_splitter_node = xml_create_node(_xml_doc, _liveset_node, "ContentSplitterProperties");
        xml_create_node_and_value(_xml_doc, _content_splitter_node, "Open", proj.content_splitter_properties_open.value());
        xml_create_node_and_value(_xml_doc, _content_splitter_node, "Size", proj.content_splitter_properties_size.value());
    }

    xml_create_node_and_value(_xml_doc, _liveset_node, "ViewStateFxSlotCount", proj.view_state_fx_slot_count);
    xml_create_node_and_value(_xml_doc, _liveset_node, "ViewStateSessionMixerHeight", proj.view_state_session_mixer_height);

    // locators TODO

    // detail clip keys midi TODO
    // tracks list wrapper TODO
    // visible tracks list wrapper TODO
    // return tracks list wrapper TODO
    // scenes list wrapper TODO
    // cue points list wrapper TODO

    xml_create_node_and_value(_xml_doc, _liveset_node, "ChooserBar", proj.chooser_bar);
    xml_create_node_and_value(_xml_doc, _liveset_node, "Annotation", proj.annotation);
    xml_create_node_and_value(_xml_doc, _liveset_node, "SoloOrPflSavedValue", proj.solo_or_pfl_saved_value);
    xml_create_node_and_value(_xml_doc, _liveset_node, "SoloInPlace", proj.solo_in_place);
    xml_create_node_and_value(_xml_doc, _liveset_node, "CrossfadeCurve", proj.crossfade_curve);
    xml_create_node_and_value(_xml_doc, _liveset_node, "LatencyCompensation", proj.latency_compensation);
    xml_create_node_and_value(_xml_doc, _liveset_node, "HighlightedTrackIndex", proj.highlighted_track_index);

    // grooves TODO
    // ArrangementOverdub TODO
    // ColorSequenceIndex TODO
    // AutoColorPickerForPlayerAndGroupTracks TODO
    // AutoColorPickerForReturnAndMasterTracks TODO
    // ViewData TODO
    // UseWarperLegacyHiQMode TODO
    // VideoWindowRect TODO
    // ShowVideoWindow TODO
    // TrackHeaderWidth TODO
    // ViewStateArrangerHasDetail TODO
    // ViewStateSessionHasDetail TODO
    // ViewStateDetailIsSample TODO
    // ViewStates TODO

    // compress
    std::string _xml_data;
    cereal::rapidxml::print(std::back_inserter(_xml_data), _xml_doc);
    std::cout << _xml_data; // lol
    gz_compress(stream, _xml_data);
}
}