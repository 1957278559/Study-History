package main

import (
	"fmt"
	"log"
	"net/http"
)

type Engine struct{}

// ServeHttp 的第二个参数 Request 包含了该 HTTP 请求的所有的信息，比如请求地址、Header 和 Body 等信息
// 第一个参数 ResponseWriter 可以用来构造针对该请求的响应
func (engine *Engine) ServeHttp(w http.ResponseWriter, req *http.Request) {
	switch req.URL.Path {
	case "/":
		fmt.Fprintf(w, "URL.Path = %q\n", req.URL.Path)
	case "/hello":
		for k, v := range req.Header {
			fmt.Fprintf(w, "Header[%q] = %q\n", k, v)
		}
	default:
		fmt.Fprintf(w, "404 NOT FOUND: %s\n", req.URL)
	}
}

func main() {
	engine := new(Engine)
	log.Fatal(http.ListenAndServe(":9999", engine))
}
