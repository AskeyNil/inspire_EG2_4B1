
#include "ser.h"
#include "rs232.h"
#include <utility>
#include <vector>
#include <numeric>
#include <iostream>
#include <cstdarg>

int getLength(Talon::OrderType type) {
  switch (type) {
  case Talon::PARA_SAVE:
  case Talon::MOVE_STOPHERE:
  case Talon::READ_EG_STATE:
  case Talon::READ_EG_RUNSTATE:
  case Talon::READ_EG_PARA:
  case Talon::READ_ACTPOS:return 0;
  case Talon::PARA_ID_SET:
  case Talon::MOVE_RELEASE:
  case Talon::SEEKPOS:return 2;
  case Talon::MOVE_CATCH_XG:
  case Talon::MOVE_HOLD_CATCH_XG:
  case Talon::SET_EG_PARA:return 4;
  }
}

void send_msg(Talon::OrderType type,
              int id,
              int count, ...) {
  int speed, power, max_eg, min_eg, eg, idNumber;

  std::vector<int> datas{
      0xeb,
      0x90,
      id,
      0x01 + getLength(type),
      type
  };

  va_list v1;
  va_start(v1, count);

  switch (type) {
  case Talon::MOVE_CATCH_XG:
  case Talon::MOVE_HOLD_CATCH_XG:

    speed = va_arg(v1, int);
    power = va_arg(v1, int);
    speed = std::max(1, speed);
    speed = std::min(1000, speed);
    power = std::max(50, power);
    power = std::min(1000, power);

    datas.push_back(speed & 0xff);
    datas.push_back(speed >> 8);
    datas.push_back(power & 0xff);
    datas.push_back(power >> 8);
    break;
  case Talon::MOVE_RELEASE:

    power = va_arg(v1, int);
    power = std::max(50, power);
    power = std::min(1000, power);

    datas.push_back(power & 0xff);
    datas.push_back(power >> 8);
    break;
  case Talon::SET_EG_PARA:

    max_eg = va_arg(v1, int);
    min_eg = va_arg(v1, int);
    max_eg = std::max(0, max_eg);
    max_eg = std::min(1000, max_eg);
    min_eg = std::max(0, min_eg);
    min_eg = std::min(1000, min_eg);

    datas.push_back(max_eg & 0xff);
    datas.push_back(max_eg >> 8);
    datas.push_back(min_eg & 0xff);
    datas.push_back(min_eg >> 8);
    break;
  case Talon::SEEKPOS:

    eg = va_arg(v1, int);
    eg = std::max(1, eg);
    eg = std::min(1000, eg);

    datas.push_back(eg & 0xff);
    datas.push_back(eg >> 8);
    break;
  case Talon::PARA_ID_SET:

    idNumber = va_arg(v1, int);
    idNumber = std::max(0, idNumber);
    idNumber = std::min(254, idNumber);

    datas.push_back(idNumber & 0xff);
    datas.push_back(idNumber >> 8);
    break;
  case Talon::PARA_SAVE:break;
  case Talon::MOVE_STOPHERE:break;
  case Talon::READ_EG_PARA:break;
  case Talon::READ_ACTPOS:break;
  case Talon::READ_EG_STATE:break;
  case Talon::READ_EG_RUNSTATE:break;
  }
  va_end(v1);

  datas.push_back(std::accumulate(datas.begin() + 2, datas.end(), 0) & 0xff);
  for (auto &data: datas) {
    RS232_SendByte(16, data);
  }
}

bool Talon::return_data(int count, ...) {

  std::cout << "开始获取数据" << std::endl;
  usleep(10 * 1000);
  int n;
  unsigned char buf[16];
  n = RS232_PollComport(id, buf, 16);
  if (n == 0) {
    return false;
  }
  if (buf[0] != 0xee || buf[1] != 0x16) {
    return false;
  }
  int sum{};
  for (int i = 2; i < n - 1; i++) {
    sum += buf[i];
  }
  if ((sum & 0xff) != buf[n - 1]) {
    return false;
  }
  int *max_eg{}, *min_eg{}, *eg{};
  TalonStatus *status;

  va_list v1;
  va_start(v1, count);
  auto type = (Talon::OrderType) buf[4];
  switch (type) {
  case Talon::MOVE_CATCH_XG:
  case Talon::MOVE_HOLD_CATCH_XG:
  case Talon::MOVE_RELEASE:
  case Talon::SEEKPOS:
  case Talon::MOVE_STOPHERE:
  case Talon::SET_EG_PARA:
  case Talon::PARA_SAVE:
  case Talon::PARA_ID_SET:return buf[5] == 0x01;

  case Talon::READ_EG_PARA:

    max_eg = va_arg(v1, int*);
    min_eg = va_arg(v1, int*);
    *max_eg = (buf[6] << 8) + buf[5];
    *min_eg = (buf[8] << 8) + buf[7];
    return true;

  case Talon::READ_ACTPOS:

    eg = va_arg(v1, int *);
    *eg = (buf[6] << 8) + buf[5];

  case Talon::READ_EG_STATE:break;
  case Talon::READ_EG_RUNSTATE:

    status = va_arg(v1, TalonStatus*);
    status->setupData(buf);

    break;
  }

  va_end(v1);
  return true;
}

Talon::Talon(std::string
             port, int
             idNumber) :
    port(std::move(port)),
    idNumber(idNumber),
    id(RS232_GetPortnr(this->port.c_str())) {
  std::cout << id << std::endl;
  if (this->id < 0) {
    std::cerr << "设备号有误，请重新输入" << std::endl;
    throw;
  }
}

bool Talon::connect() {
  if (RS232_OpenComport(id, 115200, "8N1", 0) == 0) {
    isConnected = true;
    return true;
  }
  return false;
}

void Talon::disConnect() {
  RS232_CloseComport(id);
  isConnected = false;
}

bool Talon::save() {
  if (!isConnected) {
    std::cerr << "端口未连接..." << std::endl;
    return false;
  }
  send_msg(PARA_SAVE, idNumber, 0);
  return return_data(0);
}

bool Talon::set_id(int newIdNumber) {
  if (!isConnected) {
    std::cerr << "端口未连接..." << std::endl;
    return false;
  }
  send_msg(PARA_ID_SET, idNumber, 1, newIdNumber);
  if (return_data(0)) {
    idNumber = newIdNumber;
    return true;
  } else {
    return false;
  }

}

bool Talon::catch_xg(int speed, int power) {
  if (!isConnected) {
    std::cerr << "端口未连接..." << std::endl;
    return false;
  }
  send_msg(MOVE_CATCH_XG, idNumber, 2, speed, power);
  return return_data(0);
}

bool Talon::hold_catch_xg(int speed, int power) {
  if (!isConnected) {
    std::cerr << "端口未连接..." << std::endl;
    return false;
  }
  send_msg(MOVE_HOLD_CATCH_XG, idNumber, 2, speed, power);
  return return_data(0);
}

bool Talon::release(int speed) {
  if (!isConnected) {
    std::cerr << "端口未连接..." << std::endl;
    return false;
  }
  send_msg(MOVE_RELEASE, idNumber, 1, speed);
  return return_data(0);
}

bool Talon::seekpos(int eg) {
  if (!isConnected) {
    std::cerr << "端口未连接..." << std::endl;
    return false;
  }
  send_msg(SEEKPOS, idNumber, 1, eg);
  return return_data(0);
}

bool Talon::stophere() {
  if (!isConnected) {
    std::cerr << "端口未连接..." << std::endl;
    return false;
  }
  send_msg(MOVE_STOPHERE, idNumber, 0);
  return return_data(0);
}

bool Talon::set_eg_par(int max_eg, int min_eg) {
  if (!isConnected) {
    std::cerr << "端口未连接..." << std::endl;
    return false;
  }
  send_msg(SET_EG_PARA, idNumber, 0);
  return return_data(0);
}

bool Talon::read_eg_par(int &max_eg, int &min_eg) {
  if (!isConnected) {
    std::cerr << "端口未连接..." << std::endl;
    return false;
  }
  send_msg(READ_EG_PARA, idNumber, 0);
  return return_data(2, &max_eg, &min_eg);
}

bool Talon::read_eg_runstate(TalonStatus &status) {
  if (!isConnected) {
    std::cerr << "端口未连接..." << std::endl;
    return false;
  }
  send_msg(READ_EG_RUNSTATE, idNumber, 0);
  return return_data(1, &status);
}

bool Talon::read_actpos(int &eg) {
  if (!isConnected) {
    std::cerr << "端口未连接..." << std::endl;
    return false;
  }
  send_msg(READ_ACTPOS, idNumber, 0);
  return return_data(1, &eg);
}

Talon::~Talon() = default;

void TalonStatus::setupData(const unsigned char *buf) {
  runStatus = (RunStatus) buf[5];
  troubleStatus = (TroubleStatus) buf[6];
  temp = buf[7];
  eg = (buf[9] << 8) + buf[8];
  power = (buf[11] << 8) + buf[10];
}

std::string TalonStatus::getRunStatusText() {
  switch (runStatus) {
  case OPEN_MAX_FREE:return "夹爪张开到最大且空闲";
  case CLOSE_MIN_FREE:return "夹爪闭合到最小且空闲";
  case STOP_FREE:return "夹爪停止且空闲";
  case IS_CLOSING:return "夹爪正在闭合";
  case IS_OPENING:return "夹爪正在张开";
  case CLOSING_POWER:return "夹爪在闭合过程中遇到力控停止";
  }
}
std::string TalonStatus::getTroubleStatusText() {
  switch (troubleStatus) {
  case NORMAL: return "正常";
  case COMMUNICATION: return "内部通信故障";
  case DRIVER: return "驱动器故障";
  case OVERCURRENT: return "过流故障";
  case OVER_TEMP: return "过温故障";
  case STALL: return "堵转故障";
  }
}

