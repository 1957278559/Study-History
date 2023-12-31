package buffer

import (
	"fmt"
	"sync"
	"sync/atomic"
	"webcrawler/errors"
)

// Pool 代表数据缓冲池的接口类型
type Pool interface {
	//BufferCap 用于获取池中缓冲器的统一容量
	BufferCap() uint32
	//MaxBufferNumber 用于获取池中缓冲器的最大数量
	MaxBufferNumber() uint32
	//BufferNumer 用于获取池中缓冲器的数量
	BufferNumer() uint32
	//Total 用于获取缓冲池中数据的总数
	Total() uint64
	//Put 用于向缓冲池中放入数据
	//注意，该方法应该是阻塞的
	//若缓冲池已关闭则会直接返回非 nil 的错误值
	Put(datum interface{}) error
	//Get 用于从缓冲池获取数据
	//注意，该方法应该是阻塞的
	//若缓冲池已关闭则会直接返回非 nil 的错误值
	Get() (datum interface{}, err error)
	//Close 用于关闭缓冲池
	//若缓冲池之前已关闭则返回 false，否则返回 true
	Close() bool
	//Cloed 用于判断缓冲池是否已关闭
	Closed() bool
}

// myPool 代表数据缓冲池接口的实现类型
type myPool struct {
	//bufferCap 代表缓冲器的统一容量
	bufferCap uint32
	//maxBufferNumber 代表缓冲器的最大数量
	maxBufferNumber uint32
	//bufferNumber 代表缓冲器的实际数量
	bufferNumber uint32
	//total 代表池中数据的总数
	total uint64
	//bufCh 代表存放缓冲器的通道
	buffCh chan Buffer
	//closed 代表缓冲池的关闭状态: 0-未关闭; 1-已关闭
	closed uint32
	//lock 代表保护内部共享资源的读写锁
	rwlock sync.RWMutex
}

// NewPool 用于创建一个数据缓冲池
// 参数 bufferCap 代表池内缓冲器的统一容量
// 参数 maxBufferNumber 代表池中最多包含的缓冲器的数量
func NewPool(bufferCap uint32, maxBufferNumber uint32) (Pool, error) {
	if bufferCap == 0 {
		errMsg := fmt.Sprintf("illegal buffer cap for buffer pool: %d", bufferCap)
		return nil, errors.NewIllegalParameterError(errMsg)
	}
	if maxBufferNumber == 0 {
		errMsg := fmt.Sprintf("illegal max buffer number for buffer pool: %d", maxBufferNumber)
		return nil, errors.NewIllegalParameterError(errMsg)
	}
	bufCh := make(chan Buffer, maxBufferNumber)
	buf, _ := NewBuffer(bufferCap)
	bufCh <- buf
	return &myPool{
		bufferCap:       bufferCap,
		maxBufferNumber: maxBufferNumber,
		bufferNumber:    1,
		buffCh:          bufCh,
	}, nil
}

func (pool *myPool) BufferCap() uint32 {
	return pool.bufferCap
}

func (pool *myPool) MaxBufferNumber() uint32 {
	return pool.maxBufferNumber
}

func (pool *myPool) BufferNumer() uint32 {
	return atomic.LoadUint32(&pool.bufferNumber)
}

func (pool *myPool) Total() uint64 {
	return atomic.LoadUint64(&pool.total)
}

func (pool *myPool) Put(datum interface{}) (err error) {
	if pool.Closed() {
		return ErrClosedBufferPool
	}
	var count uint32
	maxCount := pool.BufferNumer() * 5
	var ok bool
	for buf := range pool.buffCh {
		ok, err = pool.putData(buf, datum, &count, maxCount)
		if ok || err != nil {
			break
		}
	}
	return
}

func (pool *myPool) Get() (datum interface{}, err error) {
	if pool.Closed() {
		return nil, ErrClosedBufferPool
	}
	var count uint32
	maxCount := pool.BufferNumer() * 10
	for buf := range pool.buffCh {
		datum, err = pool.getData(buf, &count, maxCount)
		if datum != nil || err != nil {
			break
		}
	}
	return
}

func (pool *myPool) Close() bool {
	if !atomic.CompareAndSwapUint32(&pool.closed, 0, 1) {
		return false
	}
	pool.rwlock.Lock()
	defer pool.rwlock.Unlock()
	close(pool.buffCh)
	for buf := range pool.buffCh {
		buf.Close()
	}
	return true
}

func (pool *myPool) Closed() bool {
	return atomic.LoadUint32(&pool.closed) == 1
}

func (pool *myPool) putData(buf Buffer, datum interface{}, count *uint32, maxCount uint32) (ok bool, err error) {
	if pool.Closed() {
		return false, ErrClosedBufferPool
	}
	defer func() {
		pool.rwlock.RLock()
		if pool.Closed() {
			atomic.AddUint32(&pool.bufferNumber, ^uint32(0))
			err = ErrClosedBufferPool
		} else {
			pool.buffCh <- buf
		}
		pool.rwlock.RUnlock()
	}()
	ok, err = buf.Put(datum)
	if ok {
		atomic.AddUint64(&pool.total, 1)
		return
	}
	if err != nil {
		return
	}
	//若因缓冲器已满而未放入数据，就递增计数
	(*count)++
	//如果尝试向缓冲器放入数据的失败次数达到阈值
	//并且池中缓冲器的数量未达到最大值
	//那么就尝试创建一个新的缓冲器，先放入数据再把它放入池
	if *count >= maxCount && pool.BufferNumer() < pool.MaxBufferNumber() {
		pool.rwlock.Lock()
		if pool.BufferNumer() < pool.MaxBufferNumber() {
			if pool.Closed() {
				pool.rwlock.Unlock()
				return
			}
			newBuf, _ := NewBuffer(pool.bufferCap)
			newBuf.Put(datum)
			pool.buffCh <- newBuf
			atomic.AddUint32(&pool.bufferNumber, 1)
			atomic.AddUint64(&pool.total, 1)
			ok = true
		}
		pool.rwlock.Unlock()
		*count = 0
	}
	return
}

// 用于从给定的缓冲器获取数据，并在必要时把缓冲器归还给池
func (pool *myPool) getData(buf Buffer, count *uint32, maxCount uint32) (datum interface{}, err error) {
	if pool.Closed() {
		return nil, ErrClosedBufferPool
	}
	defer func() {
		//如果尝试从缓冲器获取数据的失败次数达到阈值
		//同时当前缓冲器已空且池中缓冲器的数量大于1
		//那么就直接关掉当前缓冲器，并不归还给池
		if *count >= maxCount &&
			buf.Len() == 0 &&
			pool.BufferNumer() > 1 {
			buf.Close()
			atomic.AddUint32(&pool.bufferNumber, ^uint32(0))
			*count = 0
			return
		}
		pool.rwlock.RLock()
		if pool.Closed() {
			atomic.AddUint32(&pool.bufferNumber, ^uint32(0))
			err = ErrClosedBufferPool
		} else {
			pool.buffCh <- buf
		}
		pool.rwlock.RUnlock()
	}()
	datum, err = buf.Get()
	if datum != nil {
		atomic.AddUint64(&pool.total, ^uint64(0))
		return
	}
	if err != nil {
		return
	}
	//若因缓冲器已空未取出数据，就递增计数
	(*count)++
	return
}
