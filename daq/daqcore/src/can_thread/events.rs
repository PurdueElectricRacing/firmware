use crate::can_thread;
use std::sync::mpsc;
/// The worker's only event sender. A missing receiver is a normal shutdown condition.
pub struct Events(pub mpsc::Sender<can_thread::CanThreadEvent>);

impl Events {
    pub fn emit(&self, event: can_thread::CanThreadEvent) -> bool {
        self.0.send(event).is_ok()
    }
}
