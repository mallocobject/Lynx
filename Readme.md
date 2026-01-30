# Lynx 🚀

**Lynx** 是一款基于 C++20 开发的高性能、非阻塞、事件驱动的 **Tcp** 服务器框架

它采用传统的 **Reactor** 模型，结合**有限状态机 (FSM)** 解析 HTTP 协议，并支持 **Zero-copy (零拷贝)** 静态资源分发

## 📊 压力测试 (Lynx-WebServer)


```bash
wrk -t12 -c400 -d30s http://127.0.0.1:8080/hello
```

```text
Running 30s test @ http://127.0.0.1:8080/hello
  12 threads and 400 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency     1.45ms    2.14ms  63.48ms   89.64%
    Req/Sec    35.99k     5.26k   53.96k    78.67%
  12969639 requests in 28.72s, 3.93GB read
  Socket errors: connect 0, read 0, write 0, timeout 372
Requests/sec: 451513.59
Transfer/sec:    139.94MB
```

```bash
wrk -t12 -c400 -d30s http://127.0.0.1:8080/json
```

```text
Running 30s test @ http://127.0.0.1:8080/json
  12 threads and 400 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency   810.50us    2.02ms  96.07ms   95.56%
    Req/Sec    70.52k    10.55k  110.94k    80.36%
  25317624 requests in 28.67s, 2.62GB read
  Socket errors: connect 0, read 0, write 0, timeout 305
Requests/sec: 883091.35
Transfer/sec:     93.48MB
```