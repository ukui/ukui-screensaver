/**
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301, USA.
**/

#ifndef LOCKCOMMON_H
#define LOCKCOMMON_H

enum SwitchPage{
    SwitchToPin = 0,
    SwitchToWechat,
    SwitchToCheck,
    SwitchToResetPWD,
    SwitchToConfigPWD
};

// 和后台规定好的 错误码固定值
enum DBusMsgCode{

 Error_QR_Get_Timeout = -8, // 二维码获取时间超时
 Error_NetWork = -7, // 网络错误 未能如期获得对应的数据
 Error_VerifyCode_Timeout = -6, // 验证码失效
 Error_RepeatedRequests = -5, // 重复请求二维码服务
 Error_NoReply = -4, // DBus接口连接失败导致的无回复错误
 Error_UnknownReason = -3, // 未知原因
 Error_ArgCnt = -2, // 参数个数错误
 Error_SetPsw_SameAsOriPsw = -1, // 设置新密码时 与原密码相同
 No_Error = 0, // 获取消息无错误
 Error_TelHaveBinded = 9009, // 手机号已绑定 -> 更换手机号后再试
 Error_VerifyCodeDiff = 9000, // 验证码错误 -> 请填写正确的验证码
 Error_AccessTokenInvalid = 9007,// Access Token 失效              -> 登录状态已过期，请重新扫码登录
 Error_ErrTimesOverMax = 9014, // 手机号登录时验证码错误次数超限
 // 账号密码登录时密码错误次数超限
 // 1小时内连续错10次触发 -> 1小时后再试
 Error_UserInfo = 9017, // 用户微信信息不存在 ->老用户，联系管理员处理
 Error_WechatHaveBinded = 9022, // 微信号已被其他账号绑定 -> 更换微信号再试
 Error_DeviceHaveBinded = 9027, // 当前设备已被绑定 -> 请联系管理员或班主任解绑
 Error_HaveBindOtherDevice = 9028, // 用户已绑定其他设备 -> 请联系管理员或班主任解绑

 //以下错误码保留
 Error_NWUnused = 6, // 网络异常，请检查网络链接哦~
 Error_TencentUnused = 7, // 腾讯服务异常，请稍后再试~
 Error_NWDelay = 28, // 网络延迟大，请更换良好网络~
 Error_ResMalloc = 301, // 资源分配异常，请重启尝试
 Error_DBOpen = 101, // 数据库打开异常，请重试，或重启再试
 Error_DBGetKey = 102, // 获取数据库密钥失败，请重试
 Error_DBRetrieve = 103, // 数据库检索数据失败，请重试
 Error_DBDecryption = 105, // 数据库解密过程失败，请重试
};



/*!
 * \brief The QRCodeSwepState enum
 * WaitingSwep 在DBus、SSOBackend、1.617 版本中未启用
 */
enum QRCodeSwepState{
    WaitingSwep = 0, //等待用户扫码
    HaveSwep = 2, // 用户扫码、等待用户确认
    CancelSwep = 3, // 用户扫码后、取消确认
    ConfirmSuccess = 4, // 扫码确认成功
    QRCodeInvalid = 5, // 二维码失效
    QRCodeTimeout = 6, //二维码超时
};

#endif // LOCKCOMMON_H
