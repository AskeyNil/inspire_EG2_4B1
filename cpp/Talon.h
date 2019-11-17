
#ifndef ASKEYNIL__TALON_H
#define ASKEYNIL__TALON_H

#include <string>

class TalonStatus;

class Talon {
private:
  const std::string port;
  const int id;
  int idNumber;
public:
  enum OrderType {
    PARA_SAVE = 0x01,     // 参数保存到内部闪存，掉电不丢失
    PARA_ID_SET = 0x04,   // 设置夹爪ID
    MOVE_CATCH_XG = 0x10, // 以设置的速度和力控阈值取夹取
    MOVE_HOLD_CATCH_XG = 0x18, // 以设置的速度和力控阈值持续夹取
    MOVE_RELEASE = 0x11,       // 以设置的速度松开
    MOVE_STOPHERE = 0x16,      // 急停
    SET_EG_PARA = 0x12,        // 设置夹爪开口的最大最小值
    READ_EG_PARA = 0x13,       // 读取夹爪开口的最大最小值
    READ_ACTPOS = 0xD9,        // 读取当前开口度
    READ_EG_STATE =
    0x14, // 读取当前夹爪的开口大小以及压力传感器的当前值和设置的阈值
    READ_EG_RUNSTATE = 0x41, // 读取夹爪运行状态
    SEEKPOS = 0x54           // 指定开口度
  };

  explicit Talon(std::string port, int idNumber = 0x01);

  /**
   * 连接端口
   * @return            是否成功
   */
  bool connect();

  /**
   * 断开连接
   */
  void disConnect();

  /**
   * 固话参数
   * @return            是否成功
   */
  bool save();

  /**
   * 设置 ID
   * @return            是否成功
   */
  bool set_id(int newIdNumber);

  /**
   * 以设置的速度和力控阈值夹取
   * @param speed       速度
   * @param power       力
   * @return            是否成功
   *
   */
  bool catch_xg(int speed = 500, int power = 100);

  /**
   * 以设置的速度和力控阈值持续夹取
   * @param speed       速度
   * @param power       力
   * @return            是否成功
   */
  bool hold_catch_xg(int speed = 500, int power = 100);

  /**
   * 以设置的速度松开
   * @param speed       速度
   * @return            是否成功
   */
  bool release(int speed = 500);

  /**
   * 指定开口度
   * @param eg          开口度
   * @return            是否成功
   */
  bool seekpos(int eg = 1);

  /**
   *  急停
   *  @return            是否成功
   */
  bool stophere();

  /**
   * 设置最大和最小的开口度
   * @param max_eg      设置的最大开口度
   * @param min_eg      设置的最小开口度
   * @return            是否成功
   */
  bool set_eg_par(int max_eg, int min_eg);

  /**
   * 读取开口参数
   * @param max_eg      返回的最大开口度
   * @param min_eg      返回的最小开口度
   * @return            是否成功
   */
  bool read_eg_par(int &max_eg, int &min_eg);

  /**
   * 读取当前开口度
   * @param eg          返回的当前开口度
   * @return            是否成功
   */
  bool read_actpos(int &eg);

  /**
   * 读取当前夹爪的运行状态
   * @param status      返回的当前状态
   * @return            是否成功
   */
  bool read_eg_runstate(TalonStatus &status);

  ~Talon();

private:
  bool return_data(int count, ...);
  bool isConnected = false;
};

class TalonStatus {

public:
  enum RunStatus {
    OPEN_MAX_FREE = 0x01,     // 夹爪张开到最大且空闲
    CLOSE_MIN_FREE = 0x02,    // 夹爪闭合到最小且空闲
    STOP_FREE = 0x03,         // 夹爪停止且空闲
    IS_CLOSING = 0x04,        // 夹爪正在闭合
    IS_OPENING = 0x05,        // 夹爪正在张开
    CLOSING_POWER = 0x06,     // 夹爪在闭合过程中遇到力控停止
  };

  enum TroubleStatus {
    NORMAL = 0x00,            // 正常
    COMMUNICATION = 0x10,     // 内部通信故障
    DRIVER = 0x08,            // 驱动器故障
    OVERCURRENT = 0x04,       // 过流故障
    OVER_TEMP = 0x02,         // 过温故障
    STALL = 0x01,             // 堵转故障
  };

  // 运行状态
  RunStatus runStatus{};
  // 故障状态
  TroubleStatus troubleStatus{};

  // 温度
  int temp{};
  // 开口度
  int eg{};
  // 夹持力
  int power{};

  /**
   * 获取当前运行状态的文字描述
   */
  std::string getRunStatusText();

  /**
   * 获取当前故障状态的文字描述
   * @return
   */
  std::string getTroubleStatusText();

private:
  friend Talon;
  void setupData(const unsigned char *buf);
};

#endif //ASKEYNIL__TALON_H
