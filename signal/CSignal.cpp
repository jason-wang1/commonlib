#include "CSignal.h"
#include <string.h>
#include <errno.h>

// 安装信号处理函数
bool CSignal::installSignal(int sigNum, SignalHandler handler)
{
	struct sigaction sigAction;
	sigAction.sa_sigaction = handler;
	sigemptyset(&sigAction.sa_mask);
	sigAction.sa_flags = SA_SIGINFO;
	if (sigaction(sigNum, &sigAction, NULL) != 0)
	{
		return false;
	}
	return true;
}

// 忽略信号
bool CSignal::ignoreSignal(int sigNum)
{
	struct sigaction sigAction;
	sigAction.sa_handler = SIG_IGN;
	sigemptyset(&sigAction.sa_mask);
	sigAction.sa_flags = 0;
	if (sigaction(sigNum, &sigAction, NULL) != 0)
	{
		return false;
	}
	return true;
}
