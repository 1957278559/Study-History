package gee

import (
	"encoding/json"
	"fmt"
	"net/http"
)

/*
	在 HandlerFunc 中，希望能够访问到解析的参数，因此，需要对 Context 对象增加一个属性和方法，
	来提供对路由参数的访问。
	我们将解析后的参数存储到Params中，通过c.Param("lang")的方式获取到对应的值
*/

// 给 map[string]interface{} 取别名 gee.H,使得在构建 JSON 数据时显得更简洁
type H map[string]interface{}

type Context struct {
	// 两个必要的属性
	Writer http.ResponseWriter
	Req    *http.Request

	// 常用属性
	Path   string
	Method string
	Params map[string]string

	//响应信息
	StatusCode int

	//中间件
	handlers []HandlerFunc
	index    int

	engine *Engine
}

func (c *Context) HTML(code int, name string, data interface{}) {
	c.SetHeander("Content-Type", "text/html")
	c.Status(code)
	if err := c.engine.htmlTemplates.ExecuteTemplate(c.Writer, name, data); err != nil {
		c.Fail(500, err.Error())
	}
}

func newContext(w http.ResponseWriter, req *http.Request) *Context {
	return &Context{
		Writer: w,
		Req:    req,
		Path:   req.URL.Path,
		Method: req.Method,
		index:  -1,
	}
}

func (c *Context) Next() {
	c.index++
	s := len(c.handlers)
	for ; c.index < s; c.index++ {
		c.handlers[c.index](c)
	}
}

func (c *Context) Fail(code int, err string) {
	c.index = len(c.handlers)
	c.JSON(code, H{"message": err})
}

func (c *Context) Param(key string) string {
	value, _ := c.Params[key]
	return value
}

func (c *Context) PostForm(key string) string {
	return c.Req.FormValue(key)
}

func (c *Context) Query(key string) string {
	return c.Req.URL.Query().Get(key)
}
func (c *Context) Status(code int) {
	c.StatusCode = code
	c.Writer.WriteHeader(code)
}

func (c *Context) SetHeander(key string, value string) {
	c.Writer.Header().Set(key, value)
}

func (c *Context) String(code int, format string, values ...interface{}) {
	c.SetHeander("Context-Type", "text/plain")
	c.Status(code)
	c.Writer.Write([]byte(fmt.Sprintf(format, values...)))
}

func (c *Context) JSON(code int, obj interface{}) {
	c.SetHeander("Context-Type", "application/json")
	c.Status(code)
	encoder := json.NewEncoder(c.Writer)
	if err := encoder.Encode(obj); err != nil {
		http.Error(c.Writer, err.Error(), 500)
	}
}

func (c *Context) Data(code int, data []byte) {
	c.Status(code)
	c.Writer.Write(data)
}

// func (c *Context) HTML(code int, html string) {
// 	c.SetHeander("Context-Type", "text/html")
// 	c.Status(code)
// 	c.Writer.Write([]byte(html))
// }
