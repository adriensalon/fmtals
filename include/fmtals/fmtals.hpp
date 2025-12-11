#pragma once

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

/// @brief Represents a subset of an Ableton Live project. Subset contains everything except
/// Live effects, instruments, modulators and max devices that are represented as info strings.
/// VST2&3 plugins are supported
struct project {

    struct automation_lane {
        std::uint32_t selected_device;
        std::uint32_t selected_envelope;
        bool is_content_selected;
        std::uint32_t lane_height;
        bool fade_view_visible;
    };

    struct warp_marker {
        float sec_time;
        float beat_time;
    };

    struct audio_clip {
        std::uint32_t lom_id;
        std::uint32_t lom_id_view;
        std::uint32_t time;
        std::vector<warp_marker> warp_markers;
        bool markers_generated;
        float current_start;
        float current_end;
        float loop_start;
        float loop_end;
        float loop_start_relative;
        bool loop_on;
        float loop_out_marker;
        float hidden_loop_start;
        float hidden_loop_end;
        std::string name;
        std::string annotation;
        std::optional<std::uint32_t> color_index;
        std::optional<std::uint32_t> color;
        std::uint32_t launch_mode;
        std::uint32_t launch_quantisation;
        // time signature bizar
        // envelopes bizar
        float scroller_time_preserver_left_time;
        float scroller_time_preserver_right_time;
        float time_selection_anchor_time;
        float time_selection_other_time;
        bool legato;
        bool ram;
        // groove settings
        bool disabled;
        float velocity_amount;
        std::uint32_t follow_time;
        std::uint32_t follow_action_a;
        std::uint32_t follow_action_b;
        std::uint32_t follow_chance_a;
        std::uint32_t follow_chance_b;
        std::uint32_t grid_fixed_numerator;
        std::uint32_t grid_fixed_denominator;
        std::uint32_t grid_interval_pixel;
        std::uint32_t grid_ntoles;
        bool grid_snap_to_grid;
        bool grid_fixed;
        float freeze_start;
        float freeze_end;
        bool is_song_tempo_master;
        bool is_warped;
    };

    struct midi_clip {
    };

    struct audio_track {
        std::uint32_t id;

        // base
        std::uint32_t lom_id;
        std::uint32_t lom_id_view;
        bool envelope_mode_preferred;
        std::uint32_t track_delay_value;
        bool track_delay_is_value_sample_based;
        std::string effective_name;
        std::string user_name;
        std::string annotation;
        std::optional<std::string> memorized_first_clip_name; // Not in 9.7.7
        std::optional<std::uint32_t> color;
        std::optional<std::uint32_t> color_index;
        std::int32_t track_group_id;
        bool track_unfolded;
        std::uint32_t devices_list_wrapper_lom_id;
        std::uint32_t clip_slots_list_wrapper_lom_id;
        std::string view_data;

        // user track
        std::int32_t saved_playing_slot;
        std::int32_t saved_playing_offset;
        bool midi_fold_in;
        bool midi_prelisten;
        bool freeze;
        std::uint32_t velocity_detail;
        bool need_arranger_refreeze;
        std::uint32_t post_process_freeze_clips;
        bool midi_target_prefers_fold_or_is_not_uniform;

        // device chain
        std::vector<automation_lane> automation_lanes;
        bool permanent_lanes_are_visible;
        std::uint32_t envelope_chooser_selected_device;
        std::uint32_t envelope_chooser_selected_envelope;
        std::string audio_input_routing_target;
        std::string audio_input_routing_upper_display_string;
        std::string audio_input_routing_lower_display_string;
        std::string midi_input_routing_target;
        std::string midi_input_routing_upper_display_string;
        std::string midi_input_routing_lower_display_string;
        std::string audio_output_routing_target;
        std::string audio_output_routing_upper_display_string;
        std::string audio_output_routing_lower_display_string;
        std::string midi_output_routing_target;
        std::string midi_output_routing_upper_display_string;
        std::string midi_output_routing_lower_display_string;
        std::uint32_t mixer_lom_id;
        std::uint32_t mixer_lom_id_view;
        bool is_expanded;
        // on... /mixer

        // main sequencer

        std::vector<audio_clip> events_audio_clips;

        // freeze sequencer

        // device chain
    };

    struct midi_track {
        std::uint32_t id;

        // base
        std::uint32_t lom_id;
        std::uint32_t lom_id_view;
        bool envelope_mode_preferred;
        std::uint32_t track_delay_value;
        bool track_delay_is_value_sample_based;
        std::string effective_name;
        std::string user_name;
        std::string annotation;
        std::optional<std::string> memorized_first_clip_name; // Not in 9.7.7
        std::optional<std::uint32_t> color;
        std::optional<std::uint32_t> color_index;
        std::int32_t track_group_id;
        bool track_unfolded;
        std::uint32_t devices_list_wrapper_lom_id;
        std::uint32_t clip_slots_list_wrapper_lom_id;
        std::string view_data;

        // user track
        std::int32_t saved_playing_slot;
        std::int32_t saved_playing_offset;
        bool midi_fold_in;
        bool midi_prelisten;
        bool freeze;
        std::uint32_t velocity_detail;
        bool need_arranger_refreeze;
        std::uint32_t post_process_freeze_clips;
        bool midi_target_prefers_fold_or_is_not_uniform;

        // device chain
        std::vector<automation_lane> automation_lanes;
        bool permanent_lanes_are_visible;
        std::uint32_t envelope_chooser_selected_device;
        std::uint32_t envelope_chooser_selected_envelope;
        std::string audio_input_routing_target;
        std::string audio_input_routing_upper_display_string;
        std::string audio_input_routing_lower_display_string;
        std::string midi_input_routing_target;
        std::string midi_input_routing_upper_display_string;
        std::string midi_input_routing_lower_display_string;
        std::string audio_output_routing_target;
        std::string audio_output_routing_upper_display_string;
        std::string audio_output_routing_lower_display_string;
        std::string midi_output_routing_target;
        std::string midi_output_routing_upper_display_string;
        std::string midi_output_routing_lower_display_string;
        std::uint32_t mixer_lom_id;
        std::uint32_t mixer_lom_id_view;
        bool is_expanded;
    };

    struct group_track {
        std::uint32_t id;

        // base
        std::uint32_t lom_id;
        std::uint32_t lom_id_view;
        bool envelope_mode_preferred;
        std::uint32_t track_delay_value;
        bool track_delay_is_value_sample_based;
        std::string effective_name;
        std::string user_name;
        std::string annotation;
        std::optional<std::string> memorized_first_clip_name; // Not in 9.7.7
        std::optional<std::uint32_t> color;
        std::optional<std::uint32_t> color_index;
        std::int32_t track_group_id;
        bool track_unfolded;
        std::uint32_t devices_list_wrapper_lom_id;
        std::uint32_t clip_slots_list_wrapper_lom_id;
        std::string view_data;

        // user track
        std::int32_t saved_playing_slot;
        std::int32_t saved_playing_offset;
        bool midi_fold_in;
        bool midi_prelisten;
        bool freeze;
        std::uint32_t velocity_detail;
        bool need_arranger_refreeze;
        std::uint32_t post_process_freeze_clips;
        bool midi_target_prefers_fold_or_is_not_uniform;

        // device chain
        std::vector<automation_lane> automation_lanes;
        bool permanent_lanes_are_visible;
        std::uint32_t envelope_chooser_selected_device;
        std::uint32_t envelope_chooser_selected_envelope;
        std::string audio_input_routing_target;
        std::string audio_input_routing_upper_display_string;
        std::string audio_input_routing_lower_display_string;
        std::string midi_input_routing_target;
        std::string midi_input_routing_upper_display_string;
        std::string midi_input_routing_lower_display_string;
        std::string audio_output_routing_target;
        std::string audio_output_routing_upper_display_string;
        std::string audio_output_routing_lower_display_string;
        std::string midi_output_routing_target;
        std::string midi_output_routing_upper_display_string;
        std::string midi_output_routing_lower_display_string;
        std::uint32_t mixer_lom_id;
        std::uint32_t mixer_lom_id_view;
        bool is_expanded;
    };

    struct return_track {
        std::uint32_t id;

        // base
        std::uint32_t lom_id;
        std::uint32_t lom_id_view;
        bool envelope_mode_preferred;
        std::uint32_t track_delay_value;
        bool track_delay_is_value_sample_based;
        std::string effective_name;
        std::string user_name;
        std::string annotation;
        std::optional<std::string> memorized_first_clip_name; // Not in 9.7.7
        std::optional<std::uint32_t> color;
        std::optional<std::uint32_t> color_index;
        std::int32_t track_group_id;
        bool track_unfolded;
        std::uint32_t devices_list_wrapper_lom_id;
        std::uint32_t clip_slots_list_wrapper_lom_id;
        std::string view_data;

        // user track
        std::int32_t saved_playing_slot;
        std::int32_t saved_playing_offset;
        bool midi_fold_in;
        bool midi_prelisten;
        bool freeze;
        std::uint32_t velocity_detail;
        bool need_arranger_refreeze;
        std::uint32_t post_process_freeze_clips;
        bool midi_target_prefers_fold_or_is_not_uniform;

        // device chain
        std::vector<automation_lane> automation_lanes;
        bool permanent_lanes_are_visible;
        std::uint32_t envelope_chooser_selected_device;
        std::uint32_t envelope_chooser_selected_envelope;
        std::string audio_input_routing_target;
        std::string audio_input_routing_upper_display_string;
        std::string audio_input_routing_lower_display_string;
        std::string midi_input_routing_target;
        std::string midi_input_routing_upper_display_string;
        std::string midi_input_routing_lower_display_string;
        std::string audio_output_routing_target;
        std::string audio_output_routing_upper_display_string;
        std::string audio_output_routing_lower_display_string;
        std::string midi_output_routing_target;
        std::string midi_output_routing_upper_display_string;
        std::string midi_output_routing_lower_display_string;
        std::uint32_t mixer_lom_id;
        std::uint32_t mixer_lom_id_view;
        bool is_expanded;
    };

    struct master_track {

        // base
        std::uint32_t lom_id;
        std::uint32_t lom_id_view;
        bool envelope_mode_preferred;
        std::uint32_t track_delay_value;
        bool track_delay_is_value_sample_based;
        std::string effective_name;
        std::string user_name;
        std::string annotation;
        std::optional<std::string> memorized_first_clip_name; // Not in 9.7.7
        std::optional<std::uint32_t> color;
        std::optional<std::uint32_t> color_index;
        std::int32_t track_group_id;
        bool track_unfolded;
        std::uint32_t devices_list_wrapper_lom_id;
        std::uint32_t clip_slots_list_wrapper_lom_id;
        std::string view_data;

        // device chain
        std::vector<automation_lane> automation_lanes;
        bool permanent_lanes_are_visible;
        std::uint32_t envelope_chooser_selected_device;
        std::uint32_t envelope_chooser_selected_envelope;
        std::string audio_input_routing_target;
        std::string audio_input_routing_upper_display_string;
        std::string audio_input_routing_lower_display_string;
        std::string midi_input_routing_target;
        std::string midi_input_routing_upper_display_string;
        std::string midi_input_routing_lower_display_string;
        std::string audio_output_routing_target;
        std::string audio_output_routing_upper_display_string;
        std::string audio_output_routing_lower_display_string;
        std::string midi_output_routing_target;
        std::string midi_output_routing_upper_display_string;
        std::string midi_output_routing_lower_display_string;
        std::uint32_t mixer_lom_id;
        std::uint32_t mixer_lom_id_view;
        bool is_expanded;
    };

    struct pre_hear_track {

        // base
        std::uint32_t lom_id;
        std::uint32_t lom_id_view;
        bool envelope_mode_preferred;
        std::uint32_t track_delay_value;
        bool track_delay_is_value_sample_based;
        std::string effective_name;
        std::string user_name;
        std::string annotation;
        std::optional<std::string> memorized_first_clip_name; // Not in 9.7.7
        std::optional<std::uint32_t> color;
        std::optional<std::uint32_t> color_index;
        std::int32_t track_group_id;
        bool track_unfolded;
        std::uint32_t devices_list_wrapper_lom_id;
        std::uint32_t clip_slots_list_wrapper_lom_id;
        std::string view_data;

        // device chain
        std::vector<automation_lane> automation_lanes;
        bool permanent_lanes_are_visible;
        std::uint32_t envelope_chooser_selected_device;
        std::uint32_t envelope_chooser_selected_envelope;
        std::string audio_input_routing_target;
        std::string audio_input_routing_upper_display_string;
        std::string audio_input_routing_lower_display_string;
        std::string midi_input_routing_target;
        std::string midi_input_routing_upper_display_string;
        std::string midi_input_routing_lower_display_string;
        std::string audio_output_routing_target;
        std::string audio_output_routing_upper_display_string;
        std::string audio_output_routing_lower_display_string;
        std::string midi_output_routing_target;
        std::string midi_output_routing_upper_display_string;
        std::string midi_output_routing_lower_display_string;
        std::uint32_t mixer_lom_id;
        std::uint32_t mixer_lom_id_view;
        bool is_expanded;
    };

    struct scene {
        std::string value;
        std::string annotation;
        std::uint32_t color_index;
        std::uint32_t lom_id;
        std::uint32_t clip_slots_list_wrapper_lom_id;
    };

    struct locator {
    };

    struct groove {
    };

    struct vst2_plugin {
    };

    struct vst3_plugin {
    };

    // We do not use polymorphism but std::variant instead
    using user_track = std::variant<audio_track, midi_track, group_track, return_track>;

    std::string major_version;
    std::string minor_version;
    std::string creator;
    std::string revision;
    std::optional<std::string> schema_change_count;
    std::int32_t overwrite_protection_number;
    std::uint32_t lom_id;
    std::uint32_t lom_id_view; // Version >= 12.0.0
    std::vector<user_track> tracks;
    std::vector<return_track> return_tracks;
    master_track project_master_track;
    pre_hear_track project_prehear_track;
    std::vector<bool> sends_pre;
    std::vector<scene> scene_names;
    std::uint32_t transport_phase_nudge_tempo;
    bool transport_loop_on;
    std::uint32_t transport_loop_start;
    std::uint32_t transport_loop_length;
    bool transport_loop_is_song_start;
    std::uint32_t transport_current_time;
    bool transport_punch_in;
    bool transport_punch_out;
    std::optional<std::uint32_t> transport_metronome_tick_duration; // Version > 9.0.0
    bool transport_draw_mode;
    std::optional<bool> transport_computer_keyboard_is_enabled; // Version < 12.0.0
    std::uint32_t song_master_values_scroller_pos_x;
    std::uint32_t song_master_values_scroller_pos_y;
    std::uint32_t global_quantisation;
    std::uint32_t auto_quantisation;
    std::uint32_t grid_fixed_numerator;
    std::uint32_t grid_fixed_denominator;
    std::uint32_t grid_grid_interval_pixel;
    std::uint32_t grid_ntoles;
    bool grid_snap_to_grid;
    bool grid_fixed;
    std::uint32_t scale_information_root_note;
    std::string scale_information_name;
    std::optional<bool> in_key; // Version >= 12.0.0
    std::uint32_t smpte_format;
    std::uint32_t time_selection_anchor_time;
    std::uint32_t time_selection_other_time;
    double sequencer_navigator_current_zoom;
    std::uint32_t sequencer_navigator_scroller_pos_x;
    std::uint32_t sequencer_navigator_scroller_pos_y;
    std::uint32_t sequencer_navigator_client_size_x;
    std::uint32_t sequencer_navigator_client_size_y;
    std::optional<bool> is_content_splitter_open; // Version >= 12.0.0
    std::optional<bool> is_expression_splitter_open; // Version >= 12.0.0
    std::optional<bool> view_state_launch_panel; // Version < 12.0.0
    std::optional<bool> view_state_envelope_panel; // Version < 12.0.0
    std::optional<bool> view_state_sample_panel; // Version < 12.0.0
    std::optional<bool> content_splitter_properties_open; // Version < 12.0.0
    std::optional<std::uint32_t> content_splitter_properties_size; // Version < 12.0.0
    std::uint32_t view_state_fx_slot_count;
    std::uint32_t view_state_session_mixer_height;
    std::vector<locator> locators;
    // detail clip keys midi
    std::uint32_t tracks_list_wrapper_lom_id;
    std::uint32_t visible_tracks_list_wrapper_lom_id;
    std::uint32_t return_tracks_list_wrapper_lom_id;
    std::uint32_t scenes_list_wrapper_lom_id;
    std::uint32_t cue_points_list_wrapper_lom_id;
    std::uint32_t chooser_bar;
    std::string annotation;
    bool solo_or_pfl_saved_value;
    bool solo_in_place;
    std::uint32_t crossfade_curve;
    std::uint32_t latency_compensation;
    std::int32_t highlighted_track_index;
    std::vector<groove> groove_pool;
    bool arrangement_overdub;
    std::uint32_t color_sequence_index;
    std::uint32_t auto_color_picker_for_player_and_group_tracks;
    std::uint32_t auto_color_picker_for_return_and_master_tracks;
    std::string view_data;
    bool use_warper_legacy_hiq_mode;
    std::int32_t video_window_rect_top;
    std::int32_t video_window_rect_bottom;
    std::int32_t video_window_rect_left;
    std::int32_t video_window_rect_right;
    bool show_video_window;
    std::uint32_t track_header_width;
    bool view_state_arranger_has_detail;
    bool view_state_session_has_detail;
    bool view_state_detail_is_sample;
    std::uint32_t view_states_session_io;
    std::uint32_t view_states_session_sends;
    std::uint32_t view_states_session_returns;
    std::uint32_t view_states_session_mixer;
    std::uint32_t view_states_session_track_delay;
    std::uint32_t view_states_session_cross_fade;
    std::uint32_t view_states_session_show_over_view;
    std::uint32_t view_states_arranger_io;
    std::uint32_t view_states_arranger_returns;
    std::uint32_t view_states_arranger_mixer;
    std::uint32_t view_states_arranger_track_delay;
    std::uint32_t view_states_arranger_show_over_view;
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
