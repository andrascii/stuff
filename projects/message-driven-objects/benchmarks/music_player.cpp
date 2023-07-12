#include "music_player.h"
#include "thread.h"

namespace benchmarks {

MusicPlayer::MusicPlayer()
  : OnVolumeChanged{this},
    OnSongChanged{this},
    volume_ {} {
  Thread()->Started.Connect(this, &MusicPlayer::OnThreadStarted);
}

uint64_t MusicPlayer::GetVolume() const noexcept {
  return volume_;
}

void MusicPlayer::SetVolume(uint64_t value) noexcept {
  if (value == volume_) {
    return;
  }

  volume_ = value;
  OnVolumeChanged(volume_);
}

const MusicPlayer::Song& MusicPlayer::GetSong() const noexcept {
  return song_;
}

void MusicPlayer::SetSong(const Song& song) {
  song_ = song;
  OnSongChanged(song_);
}

void MusicPlayer::OnThreadStarted() {
  constexpr size_t kMsgsCount = 10000000;

  for (size_t i = 0; i < kMsgsCount; ++i) {
    SetVolume(i + 1);
  }

  for (size_t i = 0; i < kMsgsCount; ++i) {
    SetSong({
      "You're A Woman",
      "BAD BOYES BLUE"
    });
  }
}

}