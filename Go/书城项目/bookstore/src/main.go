package main

import (
	"net/http"
	"text/template"
)

// IndexHandler 去首页
func IndexHanler(w http.ResponseWriter, r *http.Request) {
	//解析模板
	t := template.Must(template.ParseFiles("views/index.html"))
	//执行
	t.Execute(w, "")
}

func main() {
	http.HandleFunc("/main", IndexHanler)
	http.ListenAndServe(":8080", nil)
}
