#include "music_player.h"
#include "thread.h"

namespace benchmarks {

MusicPlayer::MusicPlayer(uint64_t iterations)
    : OnVolumeChanged{this},
      OnSongChanged{this},
      volume_{},
      iterations_{iterations} {
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
  for (uint64_t i = 0; i < iterations_; ++i) {
    SetVolume(i + 1);
  }

  for (uint64_t i = 0; i < iterations_; ++i) {
    SetSong({
      "You're A Woman",
      "BAD BOYES BLUE"
    });
  }
}

}