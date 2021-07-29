#pragma once
#include "Core.h"

namespace XYZ {
	namespace Net {

		template <typename T>
		struct MessageHeader
		{
			T		 ID{};
			uint32_t Size = 0;
		};

		template <typename T>
		struct Message
		{
			MessageHeader<T>     Header;
			std::vector<uint8_t> Body;
			

			size_t Size() const
			{
				return Body.size();
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
				
				msg.Header.Size = (uint32_t)msg.Size();

				return msg;
			}
			// Pulls any POD-like data form the message buffer
			template<typename DataType>
			friend Message<T>& operator >> (Message<T>& msg, DataType& data)
			{
				// Check that the type of the data being pushed is trivially copyable
				static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pulled from vector");

				// Cache the location towards the end of the vector where the pulled data starts
				size_t i = msg.Body.size() - sizeof(DataType);

				// Physically copy the data from the vector into the user variable
				std::memcpy(&data, msg.Body.data() + i, sizeof(DataType));

				// Shrink the vector to remove read bytes, and reset end position
				msg.Body.resize(i);

				// Recalculate the message size
				msg.Header.Size = msg.Size();

				// Return the target message so it can be "chained"
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
