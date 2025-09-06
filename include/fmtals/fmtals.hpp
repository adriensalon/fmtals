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
enum struct version : unsigned int {
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
        unsigned int selected_device;
        unsigned int selected_envelope;
        bool is_content_selected;
        unsigned int lane_height;
        bool fade_view_visible;
    };

    struct warp_marker {
        float sec_time;
        float beat_time;
    };
    
    struct audio_clip {
        unsigned int lom_id;
        unsigned int lom_id_view;
        unsigned int time;
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
        std::optional<unsigned int> color_index;
        std::optional<unsigned int> color;
        unsigned int launch_mode;
        unsigned int launch_quantisation;
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
        unsigned int follow_time;
        unsigned int follow_action_a;
        unsigned int follow_action_b;
        unsigned int follow_chance_a;
        unsigned int follow_chance_b;
        unsigned int grid_fixed_numerator;
        unsigned int grid_fixed_denominator;
        unsigned int grid_interval_pixel;
        unsigned int grid_ntoles;
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
        unsigned int id;

        // base
        unsigned int lom_id;
        unsigned int lom_id_view;
        bool envelope_mode_preferred;
        unsigned int track_delay_value;
        bool track_delay_is_value_sample_based;
        std::string effective_name;
        std::string user_name;
        std::string annotation;
        std::optional<std::string> memorized_first_clip_name; // Not in 9.7.7
        std::optional<unsigned int> color;
        std::optional<unsigned int> color_index;
        int track_group_id;
        bool track_unfolded;      
        unsigned int devices_list_wrapper_lom_id;  
        unsigned int clip_slots_list_wrapper_lom_id;  
        std::string view_data;

        // user track
        int saved_playing_slot;
        int saved_playing_offset;
        bool midi_fold_in;
        bool midi_prelisten;
        bool freeze;
        unsigned int velocity_detail;
        bool need_arranger_refreeze;
        unsigned int post_process_freeze_clips;
        bool midi_target_prefers_fold_or_is_not_uniform;

        // device chain
        std::vector<automation_lane> automation_lanes;
        bool permanent_lanes_are_visible;
        unsigned int envelope_chooser_selected_device;
        unsigned int envelope_chooser_selected_envelope;
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
        unsigned int mixer_lom_id;
        unsigned int mixer_lom_id_view;
        bool is_expanded;
        // on... /mixer

        // main sequencer

        std::vector<audio_clip> events_audio_clips;

        // freeze sequencer

        // device chain
        

    };

    struct midi_track {
        unsigned int id;

        // base
        unsigned int lom_id;
        unsigned int lom_id_view;
        bool envelope_mode_preferred;
        unsigned int track_delay_value;
        bool track_delay_is_value_sample_based;
        std::string effective_name;
        std::string user_name;
        std::string annotation;
        std::optional<std::string> memorized_first_clip_name; // Not in 9.7.7
        std::optional<unsigned int> color;
        std::optional<unsigned int> color_index;
        int track_group_id;
        bool track_unfolded;      
        unsigned int devices_list_wrapper_lom_id;  
        unsigned int clip_slots_list_wrapper_lom_id;  
        std::string view_data;

        // user track
        int saved_playing_slot;
        int saved_playing_offset;
        bool midi_fold_in;
        bool midi_prelisten;
        bool freeze;
        unsigned int velocity_detail;
        bool need_arranger_refreeze;
        unsigned int post_process_freeze_clips;
        bool midi_target_prefers_fold_or_is_not_uniform;

        // device chain
        std::vector<automation_lane> automation_lanes;
        bool permanent_lanes_are_visible;
        unsigned int envelope_chooser_selected_device;
        unsigned int envelope_chooser_selected_envelope;
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
        unsigned int mixer_lom_id;
        unsigned int mixer_lom_id_view;
        bool is_expanded;

    };

    struct group_track {
        unsigned int id;

        // base
        unsigned int lom_id;
        unsigned int lom_id_view;
        bool envelope_mode_preferred;
        unsigned int track_delay_value;
        bool track_delay_is_value_sample_based;
        std::string effective_name;
        std::string user_name;
        std::string annotation;
        std::optional<std::string> memorized_first_clip_name; // Not in 9.7.7
        std::optional<unsigned int> color;
        std::optional<unsigned int> color_index;
        int track_group_id;
        bool track_unfolded;      
        unsigned int devices_list_wrapper_lom_id;  
        unsigned int clip_slots_list_wrapper_lom_id;  
        std::string view_data;

        // user track
        int saved_playing_slot;
        int saved_playing_offset;
        bool midi_fold_in;
        bool midi_prelisten;
        bool freeze;
        unsigned int velocity_detail;
        bool need_arranger_refreeze;
        unsigned int post_process_freeze_clips;
        bool midi_target_prefers_fold_or_is_not_uniform;

        // device chain
        std::vector<automation_lane> automation_lanes;
        bool permanent_lanes_are_visible;
        unsigned int envelope_chooser_selected_device;
        unsigned int envelope_chooser_selected_envelope;
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
        unsigned int mixer_lom_id;
        unsigned int mixer_lom_id_view;
        bool is_expanded;

    };

    struct return_track {
        unsigned int id;

        // base
        unsigned int lom_id;
        unsigned int lom_id_view;
        bool envelope_mode_preferred;
        unsigned int track_delay_value;
        bool track_delay_is_value_sample_based;
        std::string effective_name;
        std::string user_name;
        std::string annotation;
        std::optional<std::string> memorized_first_clip_name; // Not in 9.7.7
        std::optional<unsigned int> color;
        std::optional<unsigned int> color_index;
        int track_group_id;
        bool track_unfolded;      
        unsigned int devices_list_wrapper_lom_id;  
        unsigned int clip_slots_list_wrapper_lom_id;  
        std::string view_data;

        // user track
        int saved_playing_slot;
        int saved_playing_offset;
        bool midi_fold_in;
        bool midi_prelisten;
        bool freeze;
        unsigned int velocity_detail;
        bool need_arranger_refreeze;
        unsigned int post_process_freeze_clips;
        bool midi_target_prefers_fold_or_is_not_uniform;

        // device chain
        std::vector<automation_lane> automation_lanes;
        bool permanent_lanes_are_visible;
        unsigned int envelope_chooser_selected_device;
        unsigned int envelope_chooser_selected_envelope;
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
        unsigned int mixer_lom_id;
        unsigned int mixer_lom_id_view;
        bool is_expanded;

    };


    struct master_track {

        // base
        unsigned int lom_id;
        unsigned int lom_id_view;
        bool envelope_mode_preferred;
        unsigned int track_delay_value;
        bool track_delay_is_value_sample_based;
        std::string effective_name;
        std::string user_name;
        std::string annotation;
        std::optional<std::string> memorized_first_clip_name; // Not in 9.7.7
        std::optional<unsigned int> color;
        std::optional<unsigned int> color_index;
        int track_group_id;
        bool track_unfolded;      
        unsigned int devices_list_wrapper_lom_id;  
        unsigned int clip_slots_list_wrapper_lom_id;  
        std::string view_data;

        // device chain
        std::vector<automation_lane> automation_lanes;
        bool permanent_lanes_are_visible;
        unsigned int envelope_chooser_selected_device;
        unsigned int envelope_chooser_selected_envelope;
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
        unsigned int mixer_lom_id;
        unsigned int mixer_lom_id_view;
        bool is_expanded;
    };

    struct pre_hear_track {
        
        // base
        unsigned int lom_id;
        unsigned int lom_id_view;
        bool envelope_mode_preferred;
        unsigned int track_delay_value;
        bool track_delay_is_value_sample_based;
        std::string effective_name;
        std::string user_name;
        std::string annotation;
        std::optional<std::string> memorized_first_clip_name; // Not in 9.7.7
        std::optional<unsigned int> color;
        std::optional<unsigned int> color_index;
        int track_group_id;
        bool track_unfolded;      
        unsigned int devices_list_wrapper_lom_id;  
        unsigned int clip_slots_list_wrapper_lom_id;  
        std::string view_data;

        // device chain
        std::vector<automation_lane> automation_lanes;
        bool permanent_lanes_are_visible;
        unsigned int envelope_chooser_selected_device;
        unsigned int envelope_chooser_selected_envelope;
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
        unsigned int mixer_lom_id;
        unsigned int mixer_lom_id_view;
        bool is_expanded;
    };

    struct scene {
        std::string value;
        std::string annotation;
        unsigned int color_index;
        unsigned int lom_id;
        unsigned int clip_slots_list_wrapper_lom_id;
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
    int overwrite_protection_number;
    unsigned int lom_id;
    unsigned int lom_id_view;                   // Version >= 12.0.0
    std::vector<user_track> tracks;
    std::vector<return_track> return_tracks;
    master_track project_master_track;
    pre_hear_track project_prehear_track;
    std::vector<bool> sends_pre;
    std::vector<scene> scene_names;
    unsigned int transport_phase_nudge_tempo;
    bool transport_loop_on;
    unsigned int transport_loop_start;
    unsigned int transport_loop_length;
    bool transport_loop_is_song_start;
    unsigned int transport_current_time;
    bool transport_punch_in;
    bool transport_punch_out;
    std::optional<unsigned int> transport_metronome_tick_duration;      // Version > 9.0.0
    bool transport_draw_mode;
    std::optional<bool> transport_computer_keyboard_is_enabled;         // Version < 12.0.0
    unsigned int song_master_values_scroller_pos_x;
    unsigned int song_master_values_scroller_pos_y;
    unsigned int global_quantisation;
    unsigned int auto_quantisation;
    unsigned int grid_fixed_numerator;
    unsigned int grid_fixed_denominator;
    unsigned int grid_grid_interval_pixel;
    unsigned int grid_ntoles;
    bool grid_snap_to_grid;
    bool grid_fixed;
    unsigned int scale_information_root_note;
    std::string scale_information_name;
    std::optional<bool> in_key;                                         // Version >= 12.0.0
    unsigned int smpte_format;
    unsigned int time_selection_anchor_time;
    unsigned int time_selection_other_time;
    double sequencer_navigator_current_zoom;
    unsigned int sequencer_navigator_scroller_pos_x;
    unsigned int sequencer_navigator_scroller_pos_y;
    unsigned int sequencer_navigator_client_size_x;
    unsigned int sequencer_navigator_client_size_y;
    std::optional<bool> is_content_splitter_open;                       // Version >= 12.0.0
    std::optional<bool> is_expression_splitter_open;                    // Version >= 12.0.0
    std::optional<bool> view_state_launch_panel;                        // Version < 12.0.0
    std::optional<bool> view_state_envelope_panel;                      // Version < 12.0.0
    std::optional<bool> view_state_sample_panel;                        // Version < 12.0.0
    std::optional<bool> content_splitter_properties_open;               // Version < 12.0.0
    std::optional<unsigned int> content_splitter_properties_size;       // Version < 12.0.0
    unsigned int view_state_fx_slot_count;
    unsigned int view_state_session_mixer_height;
    std::vector<locator> locators;
    // detail clip keys midi
    unsigned int tracks_list_wrapper_lom_id;
    unsigned int visible_tracks_list_wrapper_lom_id;
    unsigned int return_tracks_list_wrapper_lom_id;
    unsigned int scenes_list_wrapper_lom_id;
    unsigned int cue_points_list_wrapper_lom_id;
    unsigned int chooser_bar;
    std::string annotation;
    bool solo_or_pfl_saved_value;
    bool solo_in_place;
    unsigned int crossfade_curve;
    unsigned int latency_compensation;
    int highlighted_track_index;
    std::vector<groove> groove_pool;
    bool arrangement_overdub;
    unsigned int color_sequence_index;
    unsigned int auto_color_picker_for_player_and_group_tracks;
    unsigned int auto_color_picker_for_return_and_master_tracks;
    std::string view_data;
    bool use_warper_legacy_hiq_mode;
    int video_window_rect_top;
    int video_window_rect_bottom;
    int video_window_rect_left;
    int video_window_rect_right;
    bool show_video_window;
    unsigned int track_header_width;
    bool view_state_arranger_has_detail;
    bool view_state_session_has_detail;
    bool view_state_detail_is_sample;
    unsigned int view_states_session_io;
    unsigned int view_states_session_sends;
    unsigned int view_states_session_returns;
    unsigned int view_states_session_mixer;
    unsigned int view_states_session_track_delay;
    unsigned int view_states_session_cross_fade;
    unsigned int view_states_session_show_over_view;
    unsigned int view_states_arranger_io;
    unsigned int view_states_arranger_returns;
    unsigned int view_states_arranger_mixer;
    unsigned int view_states_arranger_track_delay;
    unsigned int view_states_arranger_show_over_view;
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
