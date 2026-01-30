# Lynx 🚀

**Lynx** 是一款基于 C++20 开发的高性能、非阻塞、事件驱动的 **Tcp** 服务器框架

它采用传统的 **Reactor** 模型，结合**有限状态机 (FSM)** 解析 HTTP 协议，并支持 **Zero-copy (零拷贝)** 静态资源分发

## 📊 压力测试 (Lynx-WebServer)


```bash
wrk -t12 -c400 -d30s --latency http://127.0.0.1:8080/
```

```text
Running 30s test @ http://127.0.0.1:8080/
  12 threads and 400 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency     1.28ms    1.69ms  73.97ms   87.81%
    Req/Sec    37.05k     4.83k   90.38k    81.65%
  Latency Distribution
     50%  721.00us
     75%    1.63ms
     90%    3.29ms
     99%    6.51ms
  13299068 requests in 30.10s, 15.42GB read
Requests/sec: 441840.86
Transfer/sec:    524.61MB
```
