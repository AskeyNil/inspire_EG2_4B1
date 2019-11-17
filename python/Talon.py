#!/usr/bin/env python
# coding: utf-8


# 因时电动夹爪 EG2-4B1 驱动

import serial
from enum import IntEnum
import time


def bytes_to_array(datas):
    return [ord(item) for item in datas]


class Talon:
    class OrderType(IntEnum):
        PARA_SAVE = 0x01            # 参数保存到内部闪存，掉电不丢失
        PARA_ID_SET = 0x04          # 设置夹爪ID
        MOVE_CATCH_XG = 0x10        # 以设置的速度和力控阈值取夹取
        MOVE_HOLD_CATCH_XG = 0x18   # 以设置的速度和力控阈值持续夹取
        MOVE_RELEASE = 0x11         # 以设置的速度松开
        MOVE_STOPHERE = 0x16        # 急停
        SET_EG_PARA = 0x12          # 设置夹爪开口的最大最小值
        READ_EG_PARA = 0x13         # 读取夹爪开口的最大最小值
        READ_ACTPOS = 0xD9          # 读取当前开口度
        READ_EG_STATE = 0x14        # 读取当前夹爪的开口大小以及压力传感器的当前值和设置的阈值
        READ_EG_RUNSTATE = 0x41     # 读取夹爪运行状态
        SEEKPOS = 0x54              # 指定开口度

        def length(self):
            if self == Talon.OrderType.PARA_SAVE or \
                    self == Talon.OrderType.MOVE_STOPHERE or \
                    self == Talon.OrderType.READ_EG_PARA or \
                    self == Talon.OrderType.READ_EG_STATE or \
                    self == Talon.OrderType.READ_EG_RUNSTATE or \
                    self == Talon.OrderType.READ_ACTPOS:
                return 0
            elif self == Talon.OrderType.PARA_ID_SET or \
                    self == Talon.OrderType.MOVE_RELEASE or \
                    self == Talon.OrderType.SEEKPOS:
                return 2
            elif self == Talon.OrderType.MOVE_CATCH_XG or \
                    self == Talon.OrderType.MOVE_HOLD_CATCH_XG or \
                    self == Talon.OrderType.SET_EG_PARA:
                return 4

        def return_length(self):
            pass

    def __init__(self, port="", id_number=0x01):
        self.port = port
        self.id = id_number
        # self.driver = None

    # 连接
    def connect(self):
        self.driver = serial.Serial(port=self.port, baudrate=115200)

    # 断开连接
    def disconnect(self):
        if self.driver is not None:
            self.driver.close()

    # 固话参数
    def save(self):
        return self.__send_msg(order_type=Talon.OrderType.PARA_SAVE)

    # 设置 ID
    def set_id(self, id):
        return self.__send_msg(order_type=Talon.OrderType.PARA_ID_SET,
                               id=id)

    # 以设置的速度和力控阈值取夹取
    def catch_xg(self, speed=500, power=100):
        return self.__send_msg(order_type=Talon.OrderType.MOVE_CATCH_XG,
                               speed=speed, power=power)

    # 以设置的速度和力控阈值持续夹取
    def hold_catch_xg(self, speed=500, power=100):
        return self.__send_msg(order_type=Talon.OrderType.MOVE_HOLD_CATCH_XG,
                               speed=speed, power=power)

    # 以设置的速度松开
    def release(self, speed=500):
        return self.__send_msg(order_type=Talon.OrderType.MOVE_RELEASE,
                               speed=speed)

    # 指定开口度
    def seekpos(self, eg=1):
        return self.__send_msg(order_type=Talon.OrderType.SEEKPOS, eg=eg)

    # 急停
    def stophere(self):
        return self.__send_msg(order_type=Talon.OrderType.MOVE_STOPHERE)

    # 设置开口参数
    def set_eg_par(self, max_eg=1000, min_eg=0):
        return self.__send_msg(order_type=Talon.OrderType.SET_EG_PARA,
                               max_eg=max_eg, min_eg=min_eg)

    # 读取开口参数
    def read_eg_par(self):
        result = self.__send_msg(order_type=Talon.OrderType.READ_EG_PARA)
        if isinstance(result, bool):
            return
        else:
            return result

    # 读取当前开口度
    def read_actpos(self):
        result = self.__send_msg(order_type=Talon.OrderType.READ_ACTPOS)
        if isinstance(result, bool):
            return
        else:
            return result

    # 读取当前夹爪运行状态
    def read_eg_runstate(self):
        result = self.__send_msg(order_type=Talon.OrderType.READ_EG_RUNSTATE)
        if isinstance(result, bool):
            return
        else:
            return result

    def __send_msg(self,
                   order_type,
                   speed=500,
                   power=100,
                   max_eg=1000,
                   min_eg=0,
                   eg=1,
                   id=1):
        if not isinstance(order_type, int) or \
                not isinstance(speed, int) or \
                not isinstance(power, int) or \
                not isinstance(max_eg, int) or \
                not isinstance(min_eg, int) or \
                not isinstance(eg, int) or \
                not isinstance(id, int):
            raise "par is must be int"
        if speed < 1:
            speed = 1
        elif speed > 1000:
            speed = 1000
        if power < 50:
            power = 50
        elif power > 1000:
            power = 1000
        if max_eg < 0:
            max_eg = 0
        elif max_eg > 1000:
            max_eg = 1000
        if min_eg < 0:
            min_eg = 0
        elif min_eg > 1000:
            min_eg = 1000
        if eg > 1000:
            eg = 1000
        elif eg < 1:
            eg = 1
        if id < 0:
            id = 0
        elif id > 254:
            id = 254

        datas = [0xeb,
                 0x90,
                 self.id,
                 0x01 + order_type.length(),
                 order_type.value]
        if order_type == Talon.OrderType.MOVE_CATCH_XG or \
                order_type == Talon.OrderType.MOVE_HOLD_CATCH_XG:
            datas += [speed & 0xff, speed >> 8,
                      power & 0xff, power >> 8]
        elif order_type == Talon.OrderType.MOVE_RELEASE:
            datas += [speed & 0xff, speed >> 8]
        elif order_type == Talon.OrderType.SET_EG_PARA:
            datas += [max_eg & 0xff, max_eg >> 8,
                      min_eg & 0xff, min_eg >> 8]
        elif order_type == Talon.OrderType.SEEKPOS:
            datas += [eg & 0xff, eg >> 8]
        elif order_type == Talon.OrderType.PARA_ID_SET:
            datas += [id & 0xff, id >> 8]

        datas.append(sum(datas[2:]) & 0xff)
        self.driver.write(datas)

        time.sleep(0.01)
        return_result = self.__return__data(
            bytes_to_array(self.driver.read_all()))
        return return_result

    def __return__data(self, datas):
        if len(datas) == 0:
            return False
        if datas[0] != 0xee or datas[1] == 0x16:
            return False
        # 判断校验和
        if sum(datas[2:-1]) & 0xff != datas[-1]:
            return False
        orderType = Talon.OrderType(datas[4])
        if orderType == Talon.OrderType.MOVE_CATCH_XG or \
                orderType == Talon.OrderType.MOVE_HOLD_CATCH_XG or \
                orderType == Talon.OrderType.MOVE_RELEASE or \
                orderType == Talon.OrderType.SEEKPOS or \
                orderType == Talon.OrderType.MOVE_STOPHERE or \
                orderType == Talon.OrderType.SET_EG_PARA or \
                orderType == Talon.OrderType.PARA_SAVE or \
                orderType == Talon.OrderType.PARA_ID_SET:
            if datas[5] == 0x01:
                return True
            else:
                return False
        elif orderType == Talon.OrderType.READ_EG_PARA:
            max_eg = (datas[6] << 8) + datas[5]
            min_eg = (datas[8] << 8) + datas[7]
            return (max_eg, min_eg)
        elif orderType == Talon.OrderType.READ_ACTPOS:
            return (datas[6] << 8) + datas[5]
        elif orderType == Talon.OrderType.READ_EG_RUNSTATE:
            # 运行状态吗
            status = datas[5]
            # 故障码
            trouble = datas[6]
            # 温度
            temp = datas[7]
            # 开口度
            eg = (datas[9] << 8) + datas[8]
            # 夹持力设置
            power = (datas[11] << 8) + datas[10]
            return TalonStatus(status, trouble, temp, eg, power)


class TalonStatus():
    class RunStatus(IntEnum):
        OPEN_MAX_FREE = 0x01    # 夹爪张开到最大且空闲
        CLOSE_MIN_FREE = 0x02   # 夹爪闭合到最小且空闲
        STOP_FREE = 0x03        # 夹爪停止且空闲
        IS_CLOSING = 0x04       # 夹爪正在闭合
        IS_OPENING = 0x05       # 夹爪正在张开
        CLOSING_POWER = 0x06    # 夹爪在闭合过程中遇到力控停止

        def text(self):
            if self == TalonStatus.RunStatus.OPEN_MAX_FREE:
                return "夹爪张开到最大且空闲"
            elif self == TalonStatus.RunStatus.CLOSE_MIN_FREE:
                return "夹爪闭合到最小且空闲"
            elif self == TalonStatus.RunStatus.STOP_FREE:
                return "夹爪停止且空闲"
            elif self == TalonStatus.RunStatus.IS_CLOSING:
                return "夹爪正在闭合"
            elif self == TalonStatus.RunStatus.IS_OPENING:
                return "夹爪正在张开"
            elif self == TalonStatus.RunStatus.CLOSING_POWER:
                return "夹爪在闭合过程中遇到力控停止"

    class TroubleStatus(IntEnum):
        NORMAL = 0x00           # 正常
        COMMUNICATION = 0x10    # 内部通信故障
        DRIVER = 0x08           # 驱动器故障
        OVERCURRENT = 0x04      # 过流故障
        OVER_TEMP = 0x02        # 过温故障
        STALL = 0x01            # 堵转故障

        def text(self):
            if self == TalonStatus.TroubleStatus.NORMAL:
                return "正常"
            elif self == TalonStatus.TroubleStatus.COMMUNICATION:
                return "内部通信故障"
            elif self == TalonStatus.TroubleStatus.DRIVER:
                return "驱动器故障"
            elif self == TalonStatus.TroubleStatus.OVERCURRENT:
                return "过流故障"
            elif self == TalonStatus.TroubleStatus.OVER_TEMP:
                return "过温故障"
            elif self == TalonStatus.TroubleStatus.STALL:
                return "堵转故障"

    def __init__(self, run_status, touble_status, temp, eg, power):
        self.run_status = TalonStatus.RunStatus(run_status)
        self.touble_status = TalonStatus.TroubleStatus(touble_status)
        self.temp = temp
        self.eg = eg
        self.power = power

    def __str__(self):
        return "run_status: {}, touble_status: {}, temp: {}, eg: {}, power: {}".format(self.run_status, self.touble_status, self.temp, self.eg, self.power)
