package errors

import (
	"bytes"
	"fmt"
	"strings"
)

// ErrorType 代表错误类型
type ErrorType string

// 错误类型常量。
const (
	ERROR_TYPE_DOWNLOADER ErrorType = "downloader error" // ERROR_TYPE_DOWNLOADER 代表下载器错误。
	ERROR_TYPE_ANALYZER   ErrorType = "analyzer error"   // ERROR_TYPE_ANALYZER 代表分析器错误。
	ERROR_TYPE_PIPELINE   ErrorType = "pipeline error"   // ERROR_TYPE_PIPELINE 代表条目处理管道错误。
	ERROR_TYPE_SCHEDULER  ErrorType = "scheduler error"  // ERROR_TYPE_SCHEDULER 代表调度器错误。
)

// 爬虫错误的接口类型
type CrawlerError interface {
	Type() ErrorType //用于获取错误的类型
	Error() string   //用于获取错误的提示信息
}

// 爬虫错误的实现类型
type myCrawlerError struct {
	errType    ErrorType //错误的类型
	errMsg     string    //错误的提示信息
	fullErrMsg string    //完整的错误提示信息
}

// 用于创建一个新的爬虫和错误值
func NewCrawlerError(errType ErrorType, errMsg string) CrawlerError {
	return &myCrawlerError{
		errType: errType,
		errMsg:  strings.TrimSpace(errMsg),
	}
}

// NewCrawlerErrorBy 用于根据给定的错误值创建一个新的爬虫错误值。
func NewCrawlerErrorBy(errType ErrorType, err error) CrawlerError {
	return NewCrawlerError(errType, err.Error())
}

func (ce *myCrawlerError) Type() ErrorType {
	return ce.errType
}

func (ce *myCrawlerError) Error() string {
	if ce.fullErrMsg == "" {
		ce.genFullErrMsg()
	}
	return ce.fullErrMsg
}

// 用于生成错误的提示信息，并给相应的字段赋值
func (ce *myCrawlerError) genFullErrMsg() {
	var buffer bytes.Buffer
	buffer.WriteString("crawler error: ")
	if ce.errType != "" {
		buffer.WriteString(string(ce.errType))
		buffer.WriteString(": ")
	}
	buffer.WriteString(ce.errMsg)
	ce.fullErrMsg = buffer.String()
}

// IllegalParameterError 代表非法的参数错误类型
type IllegalParameterError struct {
	msg string
}

// NewIllegalParameterError 会创建一个 IllegalParameterError 类型的实例
func NewIllegalParameterError(errMsg string) IllegalParameterError {
	return IllegalParameterError{
		msg: fmt.Sprintf("illegal parameter: %s", strings.TrimSpace(errMsg)),
	}
}

func (ipe IllegalParameterError) Error() string {
	return ipe.msg
}
