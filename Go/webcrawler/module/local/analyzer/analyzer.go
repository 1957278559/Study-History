package analyzer

import (
	"fmt"
	"webcrawler/module"
	"webcrawler/module/stub"
	"webcrawler/toolkit/reader"
)

// 分析器的实现类型
type myAnalyzer struct {
	//组件基础实例
	stub.ModuleInternal
	//响应解析器列表
	respParsers []module.ParseResponse
}

// New 用于创建一个分析器实例
func New(mid module.MID, respParsers []module.ParseResponse, scoreCalculator module.CalculateScore) (module.Analyzer, error) {
	moduleBase, err := stub.NewModuleInternal(mid, scoreCalculator)
	if err != nil {
		return nil, err
	}
	if respParsers == nil {
		return nil, genParameterError("nil response parsers")
	}
	if len(respParsers) == 0 {
		return nil, genParameterError("empty response parse list")
	}
	var innerParsers []module.ParseResponse
	for i, parser := range respParsers {
		if parser == nil {
			return nil, genParameterError(fmt.Sprintf("nil response parser[%d]", i))
		}
		innerParsers = append(innerParsers, parser)
	}
	return &myAnalyzer{
		ModuleInternal: moduleBase,
		respParsers:    innerParsers,
	}, nil
}

func (analyzer *myAnalyzer) RespParsers() []module.ParseResponse {
	parsers := make([]module.ParseResponse, len(analyzer.respParsers))
	copy(parsers, analyzer.respParsers)
	return parsers
}

func (analyzer *myAnalyzer) Analyze(resp *module.Response) (dataList []module.Data, errList []error) {
	analyzer.ModuleInternal.IncrHandlingNumber()
	defer analyzer.ModuleInternal.DecrHandlingNumber()
	analyzer.ModuleInternal.IncrCalledCount()
	if resp == nil {
		errList = append(errList, genParameterError("nil response"))
		return
	}
	httpResp := resp.HTTPResp()
	if httpResp == nil {
		errList = append(errList, genParameterError("nil HTTP response"))
		return
	}
	httpReq := httpResp.Request
	if httpReq == nil {
		errList = append(errList, genParameterError("nil HTTP request"))
		return
	}
	var reqURL = httpReq.URL
	if reqURL == nil {
		errList = append(errList, genParameterError("nil HTTP request URL"))
		return
	}

	analyzer.ModuleInternal.IncrAcceptedCount()
	respDepth := resp.Depth()
	fmt.Printf("Parse the response (URL: %s, depth: %d)...\n", reqURL, respDepth)
	//解析 HTTP 响应
	originalRespBody := httpReq.Body
	if originalRespBody != nil {
		defer originalRespBody.Close()
	}
	multipleReader, err := reader.NewMultipleReader(originalRespBody)
	if err != nil {
		errList = append(errList, genError(err.Error()))
		return
	}
	dataList = []module.Data{}
	for _, respParser := range analyzer.respParsers {
		httpResp.Body = multipleReader.Reader()
		pDataList, pErrorList := respParser(httpResp, respDepth)
		if pDataList != nil {
			for _, pData := range pDataList {
				if pData == nil {
					continue
				}
				dataList = appendDataList(dataList, pData, respDepth)
			}
		}
		if pErrorList != nil {
			for _, pError := range pErrorList {
				if pError == nil {
					continue
				}
				errList = append(errList, pError)
			}
		}
	}
	if len(errList) == 0 {
		analyzer.ModuleInternal.IncrCompletedCount()
	}
	return dataList, errList
}

// appendDataList 用于添加请求值或条目值到列表
func appendDataList(dataList []module.Data, data module.Data, respDepth uint32) []module.Data {
	if data == nil {
		return dataList
	}
	req, ok := data.(*module.Request)
	if !ok {
		return append(dataList, data)
	}
	newDepth := respDepth + 1
	if req.Depth() != newDepth {
		req = module.NewRequest(req.HTTPReq(), newDepth)
	}
	return append(dataList, req)
}
