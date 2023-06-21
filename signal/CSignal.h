#pragma once
#include <signal.h>

typedef void (*SignalHandler)(int sigNum, siginfo_t *sigInfo, void *context); // 信号处理函数

// 进程接收的信号值
namespace SignalNumber
{
	static const int ReloadConfig = SIGRTMIN + 1; // 更新配置文件
	static const int StopProcess = SIGRTMIN + 2;  // 停止退出进程
};

class CSignal
{
public:
	// 安装信号处理函数
	static bool installSignal(int sigNum, SignalHandler handler);

	// 忽略信号
	static bool ignoreSignal(int sigNum);
};
