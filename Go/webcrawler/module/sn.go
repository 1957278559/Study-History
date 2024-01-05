package module

import (
	"math"
	"sync"
)

// 序列号生成器的接口类型
type SNGenertor interface {
	//Start 用于获取预设的最小序列号
	Start() uint64
	//Max 用于获取预设的最大序列号
	Max() uint64
	//Next 用于获取下一个序列号
	Next() uint64
	//CycleCount 用于获取循环计数
	CycleCount() uint64
	//Get 用于获得一个序列号并准备下一个序列号
	Get() uint64
}

// mySNGenertor 代表序列号生成器的实现类型
type mySNGenertor struct {
	//start 代表序列号的最小值
	start uint64
	//max 代表序列号的最大值
	max uint64
	//next 代表下一个序列号
	next uint64
	//cycleCount 代表循环的计数
	cycleCount uint64
	//lock 代表读写锁
	lock sync.RWMutex
}

// NewSNGenertor 会创建一个序列号生成器
// 参数 start 用于指定第一个序列号的值
// 参数 max 用于指定序列号的最大值
func NewSNGenertor(start, max uint64) SNGenertor {
	if max == 0 {
		max = math.MaxUint64
	}
	return &mySNGenertor{
		start: start,
		max:   max,
		next:  start,
	}
}

func (s *mySNGenertor) Start() uint64 {
	return s.start
}

func (s *mySNGenertor) Max() uint64 {
	return s.max
}

func (s *mySNGenertor) Next() uint64 {
	s.lock.RLock()
	defer s.lock.RUnlock()
	return s.next
}

func (s *mySNGenertor) CycleCount() uint64 {
	s.lock.RLock()
	defer s.lock.RUnlock()
	return s.cycleCount
}

func (s *mySNGenertor) Get() uint64 {
	s.lock.RLock()
	defer s.lock.RUnlock()
	id := s.next
	if id == s.max {
		s.next = s.start
		s.cycleCount++
	} else {
		s.next++
	}
	return id
}
