#ifndef STORAGE_LEVELDB_INCLUDE_STATUS_H_
#define STORAGE_LEVELDB_INCLUDE_STATUS_H_

#include <algorithm>
#include "slice.h"

namespace leveldb {
	// 用于记录状态信息的类，保存错误码和对应的错误信息
	class Status {
	public:
		Status() noexcept : state_(nullptr) {}
		~Status() { delete[] state_; }
		Status(const Status& rhs) { state_ = (rhs.state_ == nullptr) ? nullptr : CopyState(rhs.state_); }
		Status& operator=(const Status& rhs);
		Status(Status&& rhs) noexcept : state_(rhs.state_) { rhs.state_ = nullptr; }
		Status& operator=(Status && rhs) noexcept;

		// 返回对应的错误状态
		static Status NotFound(const Slice& msg, const Slice& msg2 = Slice()) { return Status(Code::kNotFound, msg, msg2); }
		static Status Corruption(const Slice& msg, const Slice& msg2 = Slice()) { return Status(Code::kCorruption, msg, msg2); }
		static Status NotSupported(const Slice& msg, const Slice& msg2 = Slice()) { return Status(Code::kNotSupported, msg, msg2); }
		static Status InvalidArgument(const Slice& msg, const Slice& msg2 = Slice()) { return Status(Code::kInvalidArgument, msg, msg2); }
		static Status IOError(const Slice& msg, const Slice& msg2 = Slice()) { return Status(Code::kIOError, msg, msg2); }

		bool ok() const { return state_ == nullptr; } // 是否是正常状态
		bool IsNotFound() const { return code() == Code::kNotFound; } // 是否是未找到状态
		bool IsCorruption() const { return code() == Code::kCorruption; } // 是否是损坏错误
		bool IsIOError() const { return code() == Code::kIOError; } // 是否是 IO 错误
		bool isNotSupportedError() const { return code() == Code::kNotSupported; } // 是否是不支持的错误
		bool IsInvalidArgument() const { return code() == Code::kInvalidArgument; } // 是否是参数错误
		std::string ToString() const; // 将当前状态信息转为 string

	private:
		// 保存当前状态信息，正常情况下，该值为 nullptr
		// 其他情况下
		// state_[0...3] 表示错误信息长度
		// state_[4] 表示对应的错误码
		// state_[5...] 表示具体的错误信息
		const char* state_;

		// 定义错误码的枚举值
		enum class Code {
			kOk = 0,
			kNotFound = 1,
			kCorruption = 2,
			kNotSupported =3,
			kInvalidArgument = 4,
			kIOError = 5
		};

		Code code() const { return (state_ == nullptr) ? Code::kOk : static_cast<Code>(state_[4]); } // 返回当前状态
		Status(Code code, const Slice& msg, const Slice& msg2);
		static const char* CopyState(const char* s);
	};

	inline Status& Status::operator=(const Status& rhs) {
		if (state_ != rhs.state_) {
			delete[] state_;
			state_ = (rhs.state_ == nullptr) ? nullptr : CopyState(rhs.state_);
		}
		return *this;
	}

	inline Status& Status::operator=(Status&& rhs) noexcept {
		std::swap(state_, rhs.state_);
		return *this;
	}

	Status::Status(Code code, const Slice& msg, const Slice& msg2) {
		assert(code != Code::kOk);
		const uint32_t len1 = static_cast<uint32_t>(msg.size());
		const uint32_t len2 = static_cast<uint32_t>(msg2.size());
		const uint32_t size = len1 + (len2 ? (2 + len2) : 0); // +2 是因为错误码后面会跟有 ":" 和 " " 两个字符
		char* result = new char[size + 5];
		std::memcpy(result, &size, sizeof(size)); // 设置消息长度
		result[4] = static_cast<char>(code); // 设置错误码
		std::memcpy(result + 5, msg.data(), len1); // 设置错误信息
		if (len2) {
			result[5 + len1] = ':';
			result[6 + len1] = ' ';
			std::memcpy(result + 7 + len1, msg2.data(), len2); // 设置错误信息
		}
		state_ = result;
	}

	const char* Status::CopyState(const char* s) {
		uint32_t size;
		std::memcpy(&size, s, sizeof(size));
		char* result = new char[size + 5]; // 前面有 4字节 存储消息长度，1字节 存储错误码
		std::memcpy(result, s, size + 5);
		return result;
	}

	std::string Status::ToString() const {
		if (state_ == nullptr) {
			return "OK";
		}
		else {
			char tmp[30];
			const char* type;
			switch (code()) {
			case Code::kOk:
				type = "OK";
				break;
			case Code::kNotFound:
				type = "NotFound: ";
				break;
			case Code::kCorruption:
				type = "Corruption: ";
				break;
			case Code::kNotSupported:
				type = "Not implemented: ";
			case Code::kInvalidArgument:
				type = "Invalid argument: ";
				break;
			case Code::kIOError:
				type = "IO error: ";
				break;
			default:
				std::snprintf(tmp, sizeof(tmp), "Unknown code(%d): ", static_cast<int>(code()));
				type = tmp;
				break;
			}
			std::string result(type);
			uint32_t length;
			std::memcpy(&length, state_, sizeof(length));
			result.append(state_ + 5, length);
			return result;
		}
	}
}

#endif  // STORAGE_LEVELDB_INCLUDE_STATUS_H_