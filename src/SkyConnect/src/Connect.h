#ifndef CONNECT_H
#define CONNECT_H

namespace Connect {
    enum State {
        Idle,
        Recording,
        RecordingPaused,
        Playback,
        PlaybackPaused,
        NoConnection
    };
}

#endif // CONNECT_H
