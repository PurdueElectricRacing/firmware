// @generated automatically by Diesel CLI.

diesel::table! {
    events (id) {
        id -> Integer,
        upload_id -> Integer,
        name -> Text,
        color -> Text,
        start_time_ms -> BigInt,
        end_time_ms -> Nullable<BigInt>,
    }
}

diesel::table! {
    message_defs (id) {
        id -> Integer,
        upload_id -> Integer,
        can_id -> Integer,
        desc -> Text,
    }
}

diesel::table! {
    message_samples (id) {
        id -> Integer,
        message_def_id -> Integer,
        daq_ticks -> BigInt,
        timestamp_ms -> BigInt,
    }
}

diesel::table! {
    signal_defs (id) {
        id -> Integer,
        message_def_id -> Integer,
        name -> Text,
        desc -> Text,
        unit -> Text,
    }
}

diesel::table! {
    signal_samples (id) {
        id -> Integer,
        message_sample_id -> Integer,
        signal_def_id -> Integer,
        value -> Double,
    }
}

diesel::table! {
    uploads (id) {
        id -> Integer,
        name -> Text,
        git_hash -> Text,
        superdbc_hash -> Text,
        superdbc_version -> Text,
        start_time_ms -> BigInt,
        upload_time_ms -> BigInt,
        short_comments -> Text,
        long_notes -> Text,
    }
}

diesel::joinable!(events -> uploads (upload_id));
diesel::joinable!(message_defs -> uploads (upload_id));
diesel::joinable!(message_samples -> message_defs (message_def_id));
diesel::joinable!(signal_defs -> message_defs (message_def_id));
diesel::joinable!(signal_samples -> message_samples (message_sample_id));
diesel::joinable!(signal_samples -> signal_defs (signal_def_id));

diesel::allow_tables_to_appear_in_same_query!(
    events,
    message_defs,
    message_samples,
    signal_defs,
    signal_samples,
    uploads,
);
