#include <string.h>
#include "mynteye/util/log.h"
#include "mynteye/internal/channels.h"

#define PACKET_SIZE 64
#define DATA_SIZE 15

MYNTEYE_BEGIN_NAMESPACE

namespace {

inline std::uint8_t check_sum(std::uint8_t *buf, std::uint8_t length) {
  std::uint8_t crc8 = 0;
  while (length--) {
    crc8 = crc8 ^ (*buf++);
  }
  return crc8;
}

} // namespace

Channels::Channels() : is_hid_tracking_(false),
  hid_track_stop_(false),
  imu_callback_(nullptr),
  img_callback_(nullptr) {
    device_ = std::make_shared<hid::hid_device>();
}

Channels::~Channels() {
  StopHidTracking();
}

void Channels::SetImuCallback(imu_callback_t callback) {
  imu_callback_ = callback;
}

void Channels::SetImgInfoCallback(img_callback_t callback) {
  img_callback_ = callback;
}

void Channels::DoHidTrack() {
  ImuResPacket imu_res_packet;
  ImgInfoResPacket img_res_packet;

  if (!ExtractHidData(imu_res_packet, img_res_packet)) {
    return;
  }

  if (imu_callback_ && img_callback_) {
    for (auto &&imu_packet : imu_res_packet.packets) {
      imu_callback_(imu_packet);
    }
    for (auto &&img_packet : img_res_packet.packets) {
      img_callback_(img_packet);
    }
  }
}


void Channels::StartHidTracking() {
  if (is_hid_tracking_) {
    LOGE("Error:: imu device was opened already.");
    return;
  }

  is_hid_tracking_ = true;
  // open device
  if (device_->open(1, -1, -1) < 0) {
    if (device_->open(1, -1, -1) < 0) {
      LOGE("Error:: open imu device is failure.");
      return;
    }
  }

  hid_track_thread_ = std::thread([this]() {
    while (!hid_track_stop_) {
      DoHidTrack();
    }
  });
}

int Channels::ReadHidData(std::uint8_t *data, int length) {
  return device_->receive(0, data, length, 220);
}

bool Channels::ExtractHidData(ImuResPacket &imu, ImgInfoResPacket &img) {
  std::uint8_t data[PACKET_SIZE * 2]{};
  std::fill(data, data + PACKET_SIZE * 2, 0);

  int size = ReadHidData(data, PACKET_SIZE * 2);
  if (size < 0) {
    hid_track_stop_ = true;
    LOGE("Error:: Reading, device went offline !");
    return false;
  }


  for (int i = 0; i < size / PACKET_SIZE; i++) {
    std::uint8_t *packet = data + i * PACKET_SIZE;

    if (packet[PACKET_SIZE - 1] !=
        check_sum(&packet[3], packet[2])) {
      LOGW("check droped.");
      continue;
    }

    auto sn = *packet | *(packet + 1) << 8;
    if (package_sn_ == sn) { continue; }
    package_sn_ = sn;

    for (int offset = 3; offset <= PACKET_SIZE - DATA_SIZE;
        offset += DATA_SIZE) {
      if (*(packet + offset) == 2) {
        img.from_data(packet + offset);
      } else if (*(packet + offset) == 0 ||
          *(packet + offset) == 1) {
        imu.from_data(packet + offset);
      }
    }
  }

  return true;
}

bool Channels::StopHidTracking() {
  if (hid_track_stop_) {
    return false;
  }
  if (hid_track_thread_.joinable()) {
    hid_track_stop_ = true;
    hid_track_thread_.join();
    is_hid_tracking_ = false;
    hid_track_stop_ = false;
  }

  return true;
}

MYNTEYE_END_NAMESPACE
