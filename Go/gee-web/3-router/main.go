package main

import (
	"gee"
	"net/http"
)

func main() {
	r := gee.New()
	r.Get("/", func(ctx *gee.Context) {
		ctx.HTML(http.StatusOK, "<h1>Hello Gee</h1>")
	})

	r.Get("/hello", func(ctx *gee.Context) {
		ctx.String(http.StatusOK, "hello %s, you are at %s\n", ctx.Query("name"), ctx.Path)
	})

	r.Get("/hello/:name", func(ctx *gee.Context) {
		ctx.String(http.StatusOK, "hello %s, you are at %s\n", ctx.Query("name"), ctx.Path)
	})

	r.Get("/assets/*filepath", func(c *gee.Context) {
		c.JSON(http.StatusOK, gee.H{"filepath": c.Param("filepath")})
	})

	r.Run(":9999")
}
