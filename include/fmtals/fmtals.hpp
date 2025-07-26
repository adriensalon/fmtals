#pragma once

// clang-format off

#include <iostream>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace fmtals {

/// @brief Represents Ableton Live version with an enum that will not collide with versions from other DAWs.
/// Ableton Live DAW id is defined as 1. We support versions from Ableton Live 9.0.0 to 12.0.0.
enum struct version : std::uint32_t {
    v_9_0_0 = 10900,
    v_9_1_0 = 10910,
    v_9_2_0 = 10920,
    v_9_7_7 = 10977,
    v_11_0_0 = 11100,
    v_12_0_0 = 11200,
};

/// @brief Represents Ableton Live project data
struct project {

    struct version_info {
        std::string major_version;
        std::string minor_version;
        std::string creator;
        std::string revision;
    };

    struct name {
        std::string effective_name;
        std::string user_name;
        std::string annotation;
        std::optional<std::string> memorized_first_clip_name; // Not in 9.7.7
    };

    struct vec2 {
        std::uint32_t x;
        std::uint32_t y;
    };

    struct track_delay {
        float value;
        bool is_value_sample_based;
    };

    struct automation_lane {

    };

    struct device_chain {

    };

    struct audio_track {
        std::uint32_t id;
        bool envelope_mode_preferred;
        track_delay delay;
        name track_name;
        std::optional<std::uint32_t> color;
        std::optional<std::uint32_t> color_index;
        std::int32_t track_group_id;
        bool track_unfolded;
        
        // devices list weapper
        // clip slots list weapper
        // view data

        std::int32_t saved_playing_slot;
        std::int32_t saved_playing_offset;
        bool midi_fold_in;
        bool midi_prelisten;
        bool freeze;
        std::uint32_t velocity_detail;
        bool need_arranger_refreeze;
        std::uint32_t post_process_freeze_clips;
        bool midi_target_prefers_fold_or_is_not_uniform;
        device_chain track_device_chain;
        

    };

    struct midi_track {
        std::uint32_t id;
        bool envelope_mode_preferred;
        track_delay delay;
        name track_name;
        std::optional<std::uint32_t> color;
        std::optional<std::uint32_t> color_index;
        std::int32_t track_group_id;
        bool track_unfolded;
        
        // devices list weapper
        // clip slots list weapper
        // view data

        std::int32_t saved_playing_slot;
        std::int32_t saved_playing_offset;
        bool midi_fold_in;
        bool midi_prelisten;
        bool freeze;
        std::uint32_t velocity_detail;
        bool need_arranger_refreeze;
        std::uint32_t post_process_freeze_clips;
        bool midi_target_prefers_fold_or_is_not_uniform;
        device_chain track_device_chain;

    };

    struct group_track {
        std::uint32_t id;
        bool envelope_mode_preferred;
        track_delay delay;
        name track_name;
        std::optional<std::uint32_t> color;
        std::optional<std::uint32_t> color_index;
        std::int32_t track_group_id;
        bool track_unfolded;
        
        // devices list weapper
        // clip slots list weapper
        // view data

        std::int32_t saved_playing_slot;
        std::int32_t saved_playing_offset;
        bool midi_fold_in;
        bool midi_prelisten;
        bool freeze;
        std::uint32_t velocity_detail;
        bool need_arranger_refreeze;
        std::uint32_t post_process_freeze_clips;
        bool midi_target_prefers_fold_or_is_not_uniform;
        device_chain track_device_chain;

    };

    struct return_track {
        std::uint32_t id;
        bool envelope_mode_preferred;
        track_delay delay;
        name track_name;
        std::optional<std::uint32_t> color;
        std::optional<std::uint32_t> color_index;
        std::int32_t track_group_id;
        bool track_unfolded;
        
        // devices list weapper
        // clip slots list weapper
        // view data

        std::int32_t saved_playing_slot;
        std::int32_t saved_playing_offset;
        bool midi_fold_in;
        bool midi_prelisten;
        bool freeze;
        std::uint32_t velocity_detail;
        bool need_arranger_refreeze;
        std::uint32_t post_process_freeze_clips;
        bool midi_target_prefers_fold_or_is_not_uniform;
        device_chain track_device_chain;

    };

    using user_track = std::variant<audio_track, midi_track, group_track, return_track>;


    struct master_track {

    };

    struct pre_hear_track {

    };

    struct scene {
        std::string annotation;
        std::uint32_t color_index;
        // lomid
        // clips slots list wrapper
    };

    struct transport {
        std::uint32_t phase_nudge_tempo;
        bool loop_on;
        std::uint32_t loop_start;
        std::uint32_t loop_length;
        bool loop_is_song_start;
        std::uint32_t current_time;
        bool punch_in;
        bool punch_out;
        std::optional<std::uint32_t> metronome_tick_duration; // Version > 9.0.0
        bool draw_mode;
        std::optional<bool> computer_keyboard_is_enabled; // Version < 12.0.0
    };

    struct song_master_values {
        vec2 scroller_pos;
    };

    struct grid {
        std::uint32_t fixed_numerator;
        std::uint32_t fixed_denominator;
        std::uint32_t grid_interval_pixel;
        std::uint32_t ntoles;
        bool snap_to_grid;
        bool fixed;
    };

    struct scale_information {
        std::uint32_t root_note;
        std::string name;
    };

    struct time_selection {
        float anchor_time;
        float other_time;
    };

    struct sequencer_navigator {
        struct beat_time_helper {
            float current_zoom;
        };

        beat_time_helper beat_time;
        vec2 scroller_pos;
        vec2 client_size;
    };

    struct content_splitter_properties {
        bool open;
        std::uint32_t size;
    };

    struct locator {

    };

    struct groove {

    };

    version_info project_version_info;
    std::int32_t overwrite_protection_number;
    std::vector<user_track> tracks;                                     // Always present
    std::vector<return_track> return_tracks;                            // Always present
    master_track project_master_track;                                  // Always present, named 'main' track for version >= 12.0.0
    pre_hear_track project_prehear_track;                                // Always present
    std::vector<bool> sends_pre;
    std::vector<scene> scene_names;                                     // Always present
    transport project_transport;
    song_master_values master_values;
    std::uint32_t global_quantisation;                                  // Always present
    std::uint32_t auto_quantisation;                                    // Always present
    grid project_grid;                                                  // Always present
    scale_information project_scale_information;                        // Always present
    std::optional<bool> in_key;                                         // Version >= 12.0.0
    std::uint32_t smpte_format;                                         // Always present
    time_selection project_time_selection;                              // Always present
    sequencer_navigator project_sequencer_navigator;                    // Always present
    std::optional<bool> is_content_splitter_open;                       // Version >= 12.0.0
    std::optional<bool> is_expression_splitter_open;                    // Version >= 12.0.0
    std::optional<bool> view_state_launch_panel;                        // Version < 12.0.0
    std::optional<bool> view_state_envelope_panel;                      // Version < 12.0.0
    std::optional<bool> view_state_sample_panel;                        // Version < 12.0.0
    std::optional<content_splitter_properties> content_splitter;        // Version < 12.0.0
    // expression lanes for > 9
    // content lanes for > 9
    std::uint32_t view_state_fx_slot_count;
    std::uint32_t view_state_session_mixer_height;
    std::vector<locator> locators;
    // detail clip keys midi
    // tracks list wrapper
    // visible tracks list wrapper
    // return tracks list wrapper
    // scenes list wrapper
    // cue points list wrapper
    std::uint32_t chooser_bar;
    std::string annotation;
    bool solo_or_pfl_saved_value;
    bool solo_in_place;
    std::uint32_t crossfade_curve;
    std::uint32_t latency_compensation;
    std::uint32_t highlighted_track_index;
    std::vector<groove> groove_pool;
    bool arrangement_overdub;
    std::uint32_t color_sequence_index;
};

/// @brief 
/// @param stream 
/// @param proj 
/// @param ver 
void import_project(std::istream& stream, project& proj, version& ver);

/// @brief 
/// @param stream 
/// @param proj 
/// @param ver 
void export_project(std::ostream& stream, const project& proj, const version& ver);

}

// clang-format on
