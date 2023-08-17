#ifndef STORAGE_LEVELDB_INCLUDE_STATUS_H_
#define STORAGE_LEVELDB_INCLUDE_STATUS_H_

#include <algorithm>
#include <string>

#include "export.h"
#include "slice.h"

namespace leveldb
{
	// Status 类用来表示函数执行的结果状态
	// 主要用于在 leveldb 内部和外部交互时传递函数执行的结果信息
	// 以便根据状态来判断函数是否执行成功，以及处理错误情况
	class LEVELDB_EXPORT Status
	{
	public:
		Status() noexcept : state_(nullptr) {}
		Status(const Status& rhs);
		Status& operator=(const Status& rhs);
		Status(Status&& rhs) noexcept : state_(rhs.state_) { rhs.state_ = nullptr; }
		Status& operator=(Status&& rhs) noexcept;
		~Status() { delete[] state_; }

		static Status OK() { return Status(); }
		static Status NotFound(const Slice& msg, const Slice& msg2 = Slice()) { return Status(kNotFound, msg, msg2); }
		static Status Corruption(const Slice& msg, const Slice& msg2 = Slice()) { return Status(kCorruption, msg, msg2); }
		static Status NotSupported(const Slice& msg, const Slice& msg2 = Slice()) { return Status(kNotSupported, msg, msg2); }
		static Status InvalidArgument(const Slice& msg, const Slice& msg2 = Slice()) { return Status(kInvalidArgument, msg, msg2); }
		static Status IOError(const Slice& msg, const Slice& msg2 = Slice()) { return Status(kIOError, msg, msg2); }

		// 以下方法用于判断状态的类型和内容
		bool ok() const { return (state_ == nullptr); }
		bool IsNotFound() const { return code() == kNotFound; }
		bool IsCorruption() const { return code() == kCorruption; }
		bool IsIOError() const { return code() == kIOError; }
		bool IsNotSupportedError() const { return code() == kNotSupported; }
		bool IsInvalidArgument() const { return code() == kInvalidArgument; }

		// 该方法用于将状态信息转换为字符串形式，便于输出和打印
		std::string ToString() const;

	private:
		// OK 状态该值为空，其余情况该值为一个 array
		// state_[0..3] == length of message
		// state_[4]	== code
		// state_[5..]	== message
		const char* state_;

		enum Code
		{
			kOk = 0,
			kNotFound = 1,
			kCorruption = 2,
			kNotSupported = 3,
			kInvalidArgument = 4,
			kIOError = 5
		};

		Code code() const
		{
			return (state_ == nullptr) ? kOk : static_cast<Code>(state_[4]);
		}

		Status(Code code, const Slice& msg, const Slice& msg2);
		static const char* CopyState(const char* s);
	};

	inline Status::Status(const Status& rhs)
	{
		state_ = (rhs.state_ == nullptr) ? nullptr : CopyState(rhs.state_);
	}

	inline Status& Status::operator=(const Status& rhs)
	{
		if (state_ != rhs.state_)
		{
			delete[] state_;
			state_ = (rhs.state_ == nullptr) ? nullptr : CopyState(rhs.state_);
		}
		return *this;
	}

	inline Status& Status::operator=(Status&& rhs) noexcept
	{
		std::swap(state_, rhs.state_);
		return *this;
	}
}















#endif  // STORAGE_LEVELDB_INCLUDE_STATUS_H_