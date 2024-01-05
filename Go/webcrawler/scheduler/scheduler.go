package scheduler

import (
	"context"
	"fmt"
	"net/http"
	"sync"
	"webcrawler/cmap"
	"webcrawler/module"
	"webcrawler/toolkit/buffer"
)

// 创建调度器实例
func NewScheduler() Scheduler {
	return &myScheduler{}
}

// Scheduler 代表调度器的接口类型
type Scheduler interface {
	//Init 用于初始化调度器
	//参数 requestArgs 代表请求相关的参数
	//参数 dataArgs 代表数据相关的参数
	//参数 moduleArgs 代表组件相关的参数
	Init(requestArgs RequestArgs, dataArgs DataArgs, moduleArgs ModuleArgs) error
	//Start 用于启动调度器并执行爬取流程
	//参数 firstHTTPReq 即代表首次请求。调度器会以此为起点开始执行爬取流程
	Start(firstHTTPReq *http.Request) error
	//Stop 用于停止调度器的运行
	//所有处理模块执行的流程都会被种植
	Stop() error
	//Status 用于获取调度器的状态
	Status() Status
	//ErrorChan 用于获得错误通道
	//调度器以及各个处理模块运行过程中出现的所有错误都会被发送到该通道
	//若值为 nil，则说明错误哦通道不可调用或调度器已被停止
	ErrorChan() <-chan error
	//Idle 用于判断所有处理模块是否都处于空闲状态
	Idle() bool
	//Summary 用于获取摘要实例
	Summary() SchedSummary
}

// myScheduler 代表调度器的实现类型
type myScheduler struct {
	//maxDepth 代表爬取的最大深度，首次请求深度为 0
	maxDepth uint32
	//acceptedDomainMap 代表可以接受的 URL 的主域名的字典
	acceptedDomainMap cmap.ConcurrentMap
	//registrar 代表组件注册器
	registrar module.Registrar
	//reqBufferPool 代表请求缓冲池
	reqBufferPool buffer.Pool
	//respBufferPool 代表响应的缓冲池
	respBufferPool buffer.Pool
	//itemBufferPool 代表条目的缓冲池
	itemBufferPool buffer.Pool
	//errorBufferPool 代表错误的缓冲池
	errorBufferPool buffer.Pool
	//urlMap 代表已处理的 URL 的字典
	urlMap cmap.ConcurrentMap
	//ctx 代表上下文，用于感知调度器的停止
	ctx context.Context
	//cancelFunc 代表取消函数，用于停止调度器
	cancelFunc context.CancelFunc
	//status 代表状态
	status Status
	//statusLock 代表专用于状态的读写锁
	statusLock sync.RWMutex
	//summary 代表摘要信息
	summary SchedSummary
}

func (sched *myScheduler) Init(requestArgs RequestArgs, dataArgs DataArgs, moduleArgs ModuleArgs) (err error) {
	fmt.Println("Check status for initialization...")
	var oldStatus Status
	oldStatus, err = sched.checkAndSetStatus(SCHED_STATUS_INITIALIZING)
	if err != nil {
		return nil
	}
	defer func() {
		sched.statusLock.Lock()
		if err != nil {
			sched.status = oldStatus
		} else {
			sched.status = SCHED_STATUS_INITIALIZED
		}
		sched.statusLock.Unlock()
	}()
	//检查参数
	fmt.Println("Check request argument...")
	if err = requestArgs.Check(); err != nil {
		return err
	}
	fmt.Println("Check data argument...")
	if err = dataArgs.Check(); err != nil {
		return err
	}
	fmt.Println("Data arguments are valid.")
	fmt.Println("Check module arguments...")
	if err = moduleArgs.Check(); err != nil {
		return err
	}
	fmt.Println("Module arguments are valid.")
	//初始化内部字段
	fmt.Println("Initialize scheduler's fields...")
	if sched.registrar == nil {
		sched.registrar = module.NewRegistrar()
	} else {
		sched.registrar.Clear()
	}
	sched.maxDepth = requestArgs.MaxDepth
	fmt.Printf("-- Max depth: %d", sched.maxDepth)
	sched.acceptedDomainMap, _ = cmap.NewConcurrentMap(1, nil)
	for _, domain := range requestArgs.AcceptedDomains {
		sched.acceptedDomainMap.Put(domain, struct{}{})
	}
	fmt.Printf("-- Accepted primary domains: %v", requestArgs.AcceptedDomains)
	sched.urlMap, _ = cmap.NewConcurrentMap(16, nil)
	fmt.Printf("-- URL map: length: %d, concurrency: %d", sched.urlMap.Len(), sched.urlMap.Concurrency())
	sched.initBufferPool(dataArgs)
	sched.resetContext()
	sched.summary = newSchedSummary(requestArgs, dataArgs, moduleArgs, sched)
	//注册组件
	fmt.Println("Register modules...")
	if err = sched.registerModules(moduleArgs); err != nil {
		return err
	}
	fmt.Println("Scheduler has been initialized")
	return nil
}

func (sched *myScheduler) Start(firstHTTPReq *http.Request) (err error) {
	defer func() {
		if p := recover(); p != nil {
			errMsg := fmt.Sprintf("Fatal scheduler error: %s", p)
			fmt.Println(errMsg)
			err = genError(errMsg)
		}
	}()
	fmt.Println("Start scheduler...")
	//检查状态
	fmt.Println("Check status for start...")
	var oldStatus Status
	oldStatus, err = sched.checkAndSetStatus(SCHED_STATUS_STARTING)
	defer func() {
		sched.statusLock.Lock()
		if err != nil {
			sched.status = oldStatus
		} else {
			sched.status = SCHED_STATUS_STARTED
		}
		sched.statusLock.Unlock()
	}()
	if err != nil {
		return
	}
	//检查参数
	fmt.Println("Check first HTTP request...")
	if firstHTTPReq == nil {
		err = genParameterError("nil first HTTP request")
		return
	}
	fmt.Println("The first HTTP request is valid.")
	//获得首次请求的主域名，并将其添加到可接受的主域名的字典
	fmt.Println("Get the primary domain...")
	fmt.Printf("-- Host: %s", firstHTTPReq.Host)
	var primaryDomain string
	primaryDomain, err = getPrimaryDomain(firstHTTPReq.Host)
	if err != nil {
		return
	}
	fmt.Printf("-- Primary domain: %s", primaryDomain)
	sched.acceptedDomainMap.Put(primaryDomain, struct{}{})
	//开始调度数据和组件
	if err = sched.checkBufferPoolForStart(); err != nil {
		return
	}
	sched.download()
	sched.analyze()
	sched.pick()
	fmt.Println("Scheduler has been started.")
	//放入第一个请求
	firstReq := module.NewRequest(firstHTTPReq, 0)
	sched.sendReq(firstReq)
	return nil
}

// 用于状态检查，并在条件满足时设置状态
func (sched *myScheduler) checkAndSetStatus(wantedStatus Status) (oldStatus Status, err error) {
	sched.statusLock.Lock()
	defer sched.statusLock.Unlock()
	oldStatus = sched.status
	err = checkStatus(oldStatus, wantedStatus, nil)
	if err == nil {
		sched.status = wantedStatus
	}
	return
}

// initBufferPool 用于按照给定的参数初始化缓冲池。
// 如果某个缓冲池可用且未关闭，就先关闭该缓冲池。
func (sched *myScheduler) initBufferPool(dataArgs DataArgs) {
	// 初始化请求缓冲池。
	if sched.reqBufferPool != nil && !sched.reqBufferPool.Closed() {
		sched.reqBufferPool.Close()
	}
	sched.reqBufferPool, _ = buffer.NewPool(
		dataArgs.ReqBufferCap, dataArgs.ReqMaxBufferNumber)
	fmt.Printf("-- Request buffer pool: bufferCap: %d, maxBufferNumber: %d",
		sched.reqBufferPool.BufferCap(), sched.reqBufferPool.MaxBufferNumber())
	// 初始化响应缓冲池。
	if sched.respBufferPool != nil && !sched.respBufferPool.Closed() {
		sched.respBufferPool.Close()
	}
	sched.respBufferPool, _ = buffer.NewPool(
		dataArgs.RespBufferCap, dataArgs.RespMaxBufferNumber)
	fmt.Printf("-- Response buffer pool: bufferCap: %d, maxBufferNumber: %d",
		sched.respBufferPool.BufferCap(), sched.respBufferPool.MaxBufferNumber())
	// 初始化条目缓冲池。
	if sched.itemBufferPool != nil && !sched.itemBufferPool.Closed() {
		sched.itemBufferPool.Close()
	}
	sched.itemBufferPool, _ = buffer.NewPool(
		dataArgs.ItemBufferCap, dataArgs.ItemMaxBufferNumber)
	fmt.Printf("-- Item buffer pool: bufferCap: %d, maxBufferNumber: %d",
		sched.itemBufferPool.BufferCap(), sched.itemBufferPool.MaxBufferNumber())
	// 初始化错误缓冲池。
	if sched.errorBufferPool != nil && !sched.errorBufferPool.Closed() {
		sched.errorBufferPool.Close()
	}
	sched.errorBufferPool, _ = buffer.NewPool(
		dataArgs.ErrorBufferCap, dataArgs.ErrorMaxBufferNumber)
	fmt.Printf("-- Error buffer pool: bufferCap: %d, maxBufferNumber: %d",
		sched.errorBufferPool.BufferCap(), sched.errorBufferPool.MaxBufferNumber())
}

// resetContext 用于重置调度器的上下文。
func (sched *myScheduler) resetContext() {
	sched.ctx, sched.cancelFunc = context.WithCancel(context.Background())
}

// registerModules 会注册所有给定的组件。
func (sched *myScheduler) registerModules(moduleArgs ModuleArgs) error {
	for _, d := range moduleArgs.Downloaders {
		if d == nil {
			continue
		}
		ok, err := sched.registrar.Register(d)
		if err != nil {
			return genErrorByError(err)
		}
		if !ok {
			errMsg := fmt.Sprintf("Couldn't register downloader instance with MID %q!", d.ID())
			return genError(errMsg)
		}
	}
	fmt.Printf("All downloads have been registered. (number: %d)",
		len(moduleArgs.Downloaders))
	for _, a := range moduleArgs.Analyzers {
		if a == nil {
			continue
		}
		ok, err := sched.registrar.Register(a)
		if err != nil {
			return genErrorByError(err)
		}
		if !ok {
			errMsg := fmt.Sprintf("Couldn't register analyzer instance with MID %q!", a.ID())
			return genError(errMsg)
		}
	}
	fmt.Printf("All analyzers have been registered. (number: %d)",
		len(moduleArgs.Analyzers))
	for _, p := range moduleArgs.Pipelines {
		if p == nil {
			continue
		}
		ok, err := sched.registrar.Register(p)
		if err != nil {
			return genErrorByError(err)
		}
		if !ok {
			errMsg := fmt.Sprintf("Couldn't register pipeline instance with MID %q!", p.ID())
			return genError(errMsg)
		}
	}
	fmt.Printf("All pipelines have been registered. (number: %d)",
		len(moduleArgs.Pipelines))
	return nil
}
