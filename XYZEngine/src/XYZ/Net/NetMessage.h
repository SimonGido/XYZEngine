#pragma once
#include "Core.h"

namespace XYZ {
	namespace Net {

		template <typename T>
		struct MessageHeader
		{
			T		 ID;
			uint32_t Size = 0;
		};

		template <typename T>
		struct Message
		{
			MessageHeader<T>     Header;
			std::vector<uint8_t> Body;
			

			size_t Size() const
			{
				return sizeof(MessageHeader<T>) + Body.size();
			}

			friend std::ostream& operator << (std::ostream& os, const Message<T>& msg)
			{
				os << "ID: " << int(msg.Header.ID) << " Size: " << msg.Header.Size;
				return os;	
			}

			template <typename DataType>
			friend Message<T>& operator << (Message<T>& msg, const DataType& data)
			{
				static_assert(std::is_standard_layout<DataType>::value, "Data is not trivial");

				size_t oldSize = msg.Body.size();

				msg.Body.resize(msg.Body.size() + sizeof(DataType));

				std::memcpy(msg.Body.data() + oldSize, &data, sizeof(DataType));
				
				msg.Header.Size = msg.Size();

				return msg;
			}
		};

		template <typename T>
		class Connection;

		template <typename T>
		struct OwnedMessage
		{
			std::shared_ptr<Connection<T>> Remote = nullptr;
			Message<T> Message;

			friend std::ostream& operator << (std::ostream& os, const OwnedMessage<T>& msg)
			{
				os << msg.Message;
				return os;
			}
		};
	}
}
