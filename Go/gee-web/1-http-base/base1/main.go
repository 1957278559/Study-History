package main

import (
	"fmt"
	"log"
	"net/http"
)

func indexHandler(w http.ResponseWriter, req *http.Request) {
	fmt.Fprintf(w, "URL.Path = %q\n", req.URL.Path)
}

func helloHandler(w http.ResponseWriter, req *http.Request) {
	for k, v := range req.Header {
		fmt.Fprintf(w, "Handler[%q] = %q\n", k, v)
	}
}

func main() {
	// 设置 2 个路由，/ 和 //，分别绑定 indexHandler 和 helloHandler
	// 根据不同的 http 请求会调用不同的处理函数
	// 访问 / 响应是 URL.Path = /
	// 访问 /hello 相应是请求头中的键值对信息
	http.HandleFunc("/", indexHandler)
	http.HandleFunc("/hello", helloHandler)

	// 用来启动 Web 服务
	// 第一个参数是地址，:9999 表示在 9999 端口监听
	// 第二个参数代表处理所有的 HTTP 请求的实例，nil 表示使用标准库中的实例处理
	// 第二个参数则是我们基于 net/http 标准库实现 Web 框架的入口
	log.Fatal(http.ListenAndServe(":9999", nil))
}
