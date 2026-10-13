//! Monotonic bus-load measurements owned by the CAN worker.

use crate::connection;
use std::{collections::VecDeque, time};

#[derive(Default)]
pub struct BusLoadTracker {
    frames: VecDeque<(time::Instant, usize)>,
}

impl BusLoadTracker {
    pub fn record_frame(&mut self, data_bytes: usize, now: time::Instant) {
        self.frames.push_back((now, data_bytes * 8 + 66));
        self.cleanup(now);
    }

    pub fn cleanup(&mut self, now: time::Instant) {
        while self.frames.front().is_some_and(|(t, _)| {
            now.saturating_duration_since(*t) >= time::Duration::from_secs(30)
        }) {
            self.frames.pop_front();
        }
    }

    pub fn get_load(
        &self,
        seconds: u64,
        speed: connection::CanBusSpeed,
        now: time::Instant,
    ) -> f32 {
        let bits: usize = self
            .frames
            .iter()
            .filter(|(t, _)| now.saturating_duration_since(*t) < time::Duration::from_secs(seconds))
            .map(|(_, n)| n)
            .sum();
        bits as f32 / (speed.to_bps() as f32 * seconds as f32) * 100.0
    }
}
