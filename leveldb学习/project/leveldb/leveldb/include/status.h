#ifndef STORAGE_LEVELDB_INCLUDE_STATUS_H_
#define STORAGE_LEVELDB_INCLUDE_STATUS_H_

#include <algorithm>
#include "slice.h"

namespace leveldb {
	// ���ڼ�¼״̬��Ϣ���࣬���������Ͷ�Ӧ�Ĵ�����Ϣ
	class Status {
	public:
		Status() noexcept : state_(nullptr) {}
		~Status() { delete[] state_; }
		Status(const Status& rhs) { state_ = (rhs.state_ == nullptr) ? nullptr : CopyState(rhs.state_); }
		Status& operator=(const Status& rhs);
		Status(Status&& rhs) noexcept : state_(rhs.state_) { rhs.state_ = nullptr; }
		Status& operator=(Status && rhs) noexcept;

		// ���ض�Ӧ�Ĵ���״̬
		static Status NotFound(const Slice& msg, const Slice& msg2 = Slice()) { return Status(Code::kNotFound, msg, msg2); }
		static Status Corruption(const Slice& msg, const Slice& msg2 = Slice()) { return Status(Code::kCorruption, msg, msg2); }
		static Status NotSupported(const Slice& msg, const Slice& msg2 = Slice()) { return Status(Code::kNotSupported, msg, msg2); }
		static Status InvalidArgument(const Slice& msg, const Slice& msg2 = Slice()) { return Status(Code::kInvalidArgument, msg, msg2); }
		static Status IOError(const Slice& msg, const Slice& msg2 = Slice()) { return Status(Code::kIOError, msg, msg2); }

		bool ok() const { return state_ == nullptr; } // �Ƿ�������״̬
		bool IsNotFound() const { return code() == Code::kNotFound; } // �Ƿ���δ�ҵ�״̬
		bool IsCorruption() const { return code() == Code::kCorruption; } // �Ƿ����𻵴���
		bool IsIOError() const { return code() == Code::kIOError; } // �Ƿ��� IO ����
		bool isNotSupportedError() const { return code() == Code::kNotSupported; } // �Ƿ��ǲ�֧�ֵĴ���
		bool IsInvalidArgument() const { return code() == Code::kInvalidArgument; } // �Ƿ��ǲ�������
		std::string ToString() const; // ����ǰ״̬��ϢתΪ string

	private:
		// ���浱ǰ״̬��Ϣ����������£���ֵΪ nullptr
		// ���������
		// state_[0...3] ��ʾ������Ϣ����
		// state_[4] ��ʾ��Ӧ�Ĵ�����
		// state_[5...] ��ʾ����Ĵ�����Ϣ
		const char* state_;

		// ����������ö��ֵ
		enum class Code {
			kOk = 0,
			kNotFound = 1,
			kCorruption = 2,
			kNotSupported =3,
			kInvalidArgument = 4,
			kIOError = 5
		};

		Code code() const { return (state_ == nullptr) ? Code::kOk : static_cast<Code>(state_[4]); } // ���ص�ǰ״̬
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
		const uint32_t size = len1 + (len2 ? (2 + len2) : 0); // +2 ����Ϊ������������� ":" �� " " �����ַ�
		char* result = new char[size + 5];
		std::memcpy(result, &size, sizeof(size)); // ������Ϣ����
		result[4] = static_cast<char>(code); // ���ô�����
		std::memcpy(result + 5, msg.data(), len1); // ���ô�����Ϣ
		if (len2) {
			result[5 + len1] = ':';
			result[6 + len1] = ' ';
			std::memcpy(result + 7 + len1, msg2.data(), len2); // ���ô�����Ϣ
		}
		state_ = result;
	}

	const char* Status::CopyState(const char* s) {
		uint32_t size;
		std::memcpy(&size, s, sizeof(size));
		char* result = new char[size + 5]; // ǰ���� 4�ֽ� �洢��Ϣ���ȣ�1�ֽ� �洢������
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