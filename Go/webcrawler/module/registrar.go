package module

import (
	"fmt"
	"sync"
	"webcrawler/errors"
)

// 组件注册器的接口
type Registrar interface {
	//Register 用于注册组件实例
	Register(module Module) (bool, error)
	//用于注销组件实例
	Unregister(mid MID) (bool, error)
	//用于获取一个指定类型的组件的实例
	//该函数基于负载均衡策略返回实例
	Get(moduleType Type) (Module, error)
	//用于获取指定类型的所有组件实例
	GetAllByType(moduleType Type) (map[MID]Module, error)
	//用于获取所有被组件实例
	GetAll() map[MID]Module
	//清除所有组件注册记录
	Clear()
}

// myRegistrar 代表组件注册器的实现类型
type myRegistrar struct {
	//moduleTypeMap 代表组件类型与对应组件实例的映射
	moduleTypeMap map[Type]map[MID]Module
	//rwlock 代表组件注册专用读写锁
	rwlock sync.RWMutex
}

// NewRegistrar 用于创建一个组件注册器的实例
func NewRegistrar() Registrar {
	return &myRegistrar{
		moduleTypeMap: map[Type]map[MID]Module{},
	}
}

func (r *myRegistrar) Register(module Module) (bool, error) {
	if module == nil {
		return false, errors.NewIllegalParameterError("nil module instance")
	}
	mid := module.ID()
	parts, err := SplitMID(mid)
	if err != nil {
		return false, err
	}
	moduleType := legalLetterTypeMap[parts[0]]
	if !CheckType(moduleType, module) {
		errMsg := fmt.Sprintf("incorrect module type: %s", moduleType)
		return false, errors.NewIllegalParameterError(errMsg)
	}
	r.rwlock.Lock()
	defer r.rwlock.Unlock()
	modules := r.moduleTypeMap[moduleType]
	if modules == nil {
		modules = map[MID]Module{}
	}
	if _, ok := modules[mid]; ok {
		return false, nil
	}
	modules[mid] = module
	r.moduleTypeMap[moduleType] = modules
	return true, nil
}

func (r *myRegistrar) Unregister(mid MID) (bool, error) {
	parts, err := SplitMID(mid)
	if err != nil {
		return false, err
	}
	moduleType := legalLetterTypeMap[parts[0]]
	var deleted bool
	r.rwlock.Lock()
	defer r.rwlock.RUnlock()
	if modules, ok := r.moduleTypeMap[moduleType]; ok {
		if _, ok := modules[mid]; ok {
			delete(modules, mid)
			deleted = true
		}
	}
	return deleted, nil
}

func (r *myRegistrar) Get(moduleType Type) (Module, error) {
	modules, err := r.GetAllByType(moduleType)
	if err != nil {
		return nil, err
	}
	minScore := uint64(0)
	var selectedModule Module
	for _, module := range modules {
		SetScore(module)
		score := module.Score()
		if minScore == 0 || score < minScore {
			selectedModule = module
			minScore = score
		}
	}
	return selectedModule, nil
}

func (r *myRegistrar) GetAllByType(moduleType Type) (map[MID]Module, error) {
	if !LegalType(moduleType) {
		errMsg := fmt.Sprintf("illegal module type: %s", moduleType)
		return nil, errors.NewIllegalParameterError(errMsg)
	}
	r.rwlock.RLock()
	defer r.rwlock.RUnlock()
	modules := r.moduleTypeMap[moduleType]
	if len(modules) == 0 {
		return nil, ErrNotFoundModuleInstance
	}
	result := map[MID]Module{}
	for mid, module := range modules {
		result[mid] = module
	}
	return result, nil
}

func (r *myRegistrar) GetAll() map[MID]Module {
	result := map[MID]Module{}
	r.rwlock.RLock()
	defer r.rwlock.RUnlock()
	for _, modules := range r.moduleTypeMap {
		for mid, module := range modules {
			result[mid] = module
		}
	}
	return result
}

func (r *myRegistrar) Clear() {
	r.rwlock.Lock()
	defer r.rwlock.Unlock()
	r.moduleTypeMap = map[Type]map[MID]Module{}
}
