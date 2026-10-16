-- Your SQL goes here
CREATE TABLE uploads (
    id					INTEGER PRIMARY KEY NOT NULL,
    name				TEXT NOT NULL,
    git_hash			TEXT NOT NULL,
    superdbc_hash 		TEXT NOT NULL,
    superdbc_version	TEXT NOT NULL,
    start_time_ms		INTEGER NOT NULL,
    upload_time_ms		INTEGER NOT NULL,
    short_comments		TEXT NOT NULL DEFAULT '',
    long_notes			TEXT NOT NULL DEFAULT ''
);

CREATE TABLE events (
    id 					INTEGER PRIMARY KEY NOT NULL,
    upload_id 			INTEGER NOT NULL
        REFERENCES uploads(id) ON DELETE CASCADE,
    name 				TEXT NOT NULL,
    color				TEXT NOT NULL,
    start_time_ms		INTEGER NOT NULL,
    end_time_ms			INTEGER,

    CHECK(end_time_ms IS NULL OR end_time_ms >= start_time_ms)
);

CREATE TABLE message_defs (
    id					INTEGER PRIMARY KEY NOT NULL,
    upload_id			INTEGER NOT NULL
        REFERENCES uploads(id) ON DELETE CASCADE,
    can_id 				INTEGER NOT NULL,
    name				TEXT NOT NULL,
    desc				TEXT NOT NULL DEFAULT '',

    CHECK(can_id >= 0)
);

CREATE TABLE message_samples (
    id					INTEGER PRIMARY KEY NOT NULL,
    message_def_id		INTEGER NOT NULL
        REFERENCES message_defs(id) ON DELETE CASCADE,
    daq_ticks			INTEGER NOT NULL,
    timestamp_ms		INTEGER NOT NULL,

    CHECK(daq_ticks >= 0)
);

CREATE TABLE signal_defs (
    id 					INTEGER PRIMARY KEY NOT NULL,
    message_def_id		INTEGER NOT NULL
        REFERENCES message_defs(id) ON DELETE CASCADE,
    name				TEXT NOT NULL,
    desc				TEXT NOT NULL DEFAULT '',
    unit				TEXT NOT NULL DEFAULT ''
);

CREATE TABLE signal_samples (
    id					INTEGER PRIMARY KEY NOT NULL,
    message_sample_id	INTEGER NOT NULL
        REFERENCES message_samples(id) ON DELETE CASCADE,
    signal_def_id		INTEGER NOT NULL
        REFERENCES signal_defs(id) ON DELETE CASCADE,
    value 				REAL NOT NULL
);

CREATE INDEX idx_events_upload_start
    ON events(upload_id, start_time_ms);

CREATE INDEX idx_message_samples_def_time
    ON message_samples(message_def_id, timestamp_ms);

CREATE INDEX idx_signal_samples_def_sample
    ON signal_samples(signal_def_id, message_sample_id);