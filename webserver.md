1. SIGPIPE —— 写已关闭的连接
- 触发时机：你往一个对端已经关闭的 socket/pipe 写数据时，内核发 SIGPIPE。
- 默认动作：直接终止进程。如果客户端断开后服务器还在尝试回包，进程会莫名其妙被杀。
- 这里 SIG_IGN 表示忽略它，进程不退出；此时 write 会返回 -1 且 errno=EPIPE，你在代码里自己判断、关连接即可。
- 为什么直接忽略而不是走 sig_handler：因为 SIGPIPE 不需要在主循环里做任何事，忽略最干净。
2. SIGALRM —— 定时器心跳
- 触发时机：调用 alarm(TIMESLOT) 后，过 TIMESLOT 秒内核发一次 SIGALRM，且只发一次。
- 用途：作为服务器的「心跳时钟」，用来周期性触发超时检测（timer_handler → tick() 关闭空闲连接）。
- 注册到 sig_handler：信号到来时，sig_handler 把 SIGALRM 编号写进管道 → 主循环 dealwithsignal 里设 timeout=true → 主循环末尾调 timer_handler() → tick() 后再次 alarm(TIMESLOT) 重新设下一次闹钟，形成周期循环。
3. SIGTERM —— 优雅退出
- 触发时机：别人对你进程发 kill <pid>（默认就是 SIGTERM），或系统关机时。
- 用途：请求服务器「正常退出」。注册到 sig_handler 后，dealwithsignal 里设 stop_server=true，主循环 while(!stop_server) 自然结束，释放资源后退出。
- 对比 SIGKILL（kill -9）：SIGKILL 不能被捕获也不能忽略，强制秒杀；SIGTERM 可以被捕获，所以能优雅收尾。
- 补充：你在终端按 Ctrl+C 发的是 SIGINT，不是 SIGTERM。若想 Ctrl+C 也能优雅退出，可再加一行 m_utils.addsig(SIGINT, utils::sig_handler, false);。