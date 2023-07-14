#pragma once

#include "object.h"
#include "signal_impl.h"

namespace benchmarks {

using namespace mdo;

class MusicPlayer : public Object {
public:
  struct Song {
    std::string name;
    std::string singer;
  };

  explicit MusicPlayer(uint64_t iterations);

  uint64_t GetVolume() const noexcept;
  void SetVolume(uint64_t value) noexcept;

  const Song& GetSong() const noexcept;
  void SetSong(const Song& song);

  Signal<uint64_t> OnVolumeChanged;
  Signal<const Song&> OnSongChanged;

private:
  void OnThreadStarted();

private:
  uint64_t volume_;
  Song song_;
  uint64_t iterations_;
};

}