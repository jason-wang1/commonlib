Singleton 单例模式


CommonCache 公共缓存模板
支持Hash, Vector, Single三种缓存方案
如果需要可以拓展 Set 等类似的结构
目前主要用于RedisProtoData/LocalCache


Function 公共函数
其中文件系统部分 IsFile 和 IsDir 建议使用 C++17 filesystem 替换.


CommonPool
资源池模板, 需要实现 Builder(资源创建) 与 Deleter(资源归还清理) 两个类.
注意: Deleter 在调用Release函数失败时, 说明资源池已关闭, 需要自行释放资源.



其他需求可以联系 tkxiong