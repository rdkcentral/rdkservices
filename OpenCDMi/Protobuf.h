/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2022 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include "Module.h"

namespace WPEFramework {

namespace Protobuf {

    struct IElement {
        enum class WireType : uint8_t {
            VARINT = 0,
            FIXED64 = 1,
            LENGTH_DELIMITED = 2,
            GROUP_START = 3,
            GROUP_END = 4,
            FIXED32 = 5
        };

        virtual ~IElement() = default;
        virtual uint32_t Deserialize(const uint8_t data[], const uint32_t length) = 0;
        virtual bool IsSet() const = 0;
        virtual WireType Type() const = 0;
    }; // struct IElement

    template<typename T>
    class ValueElementType : public IElement {
    public:
        using type = T;
        using IElement::WireType;
        ValueElementType(const ValueElementType&) = default;
        ValueElementType& operator=(const ValueElementType&) = default;
        ValueElementType()
            : _value()
            , _set(false)
        { }
        ~ValueElementType() = default;

    public:
        bool IsSet() const override {
            return (_set);
        }
        const T& Value() const {
            return (_value);
        }
        T& Value() {
            return (_value);
        }

    protected:
        void Set(const bool set) {
            _set = set;
        }

    private:
        T _value;
        bool _set;
    }; // class ValueElementType

    template<typename T>
    class VarintType : public ValueElementType<T> {
        static_assert(std::is_integral<T>::value || std::is_enum<T>::value, "Varint requires int type");

    public:
        VarintType()
            : ValueElementType<T>()
        { }
        VarintType(const VarintType<T>&) = default;
        VarintType<T>& operator=(const VarintType<T>&) = default;
        ~VarintType() = default;

    public:
        using typename ValueElementType<T>::WireType;
        uint32_t Deserialize(const uint8_t data[], const uint32_t length) override
        {
            ASSERT(data != nullptr);
            uint32_t result = 0;
            result = ReadVarint(data, length, ValueElementType<T>::Value());
            ValueElementType<T>::Set(result != 0);
            return (result);
        }
        WireType Type() const override {
            return (WireType::VARINT);
        }

    private:
        static uint8_t ReadVarint(const uint8_t data[], const uint32_t length, T& out)
        {
            ASSERT(data != nullptr);
            uint8_t result = 0;
            const uint8_t* ptr = data;
            uint8_t shift = 0;
            uint64_t value = 0;
            uint8_t size = sizeof(value);
            while (size-- && (ptr < (data + length))) {
                value |= (static_cast<uint64_t>(*ptr & 0x7F) << shift);
                if (((*ptr++) & 0x80) == 0) {
                    out = static_cast<T>(value);
                    result = static_cast<uint8_t>(ptr - data);
                    break;
                }
                shift += 7;
            }
            return (result);
        }
    }; // class VarintType

    template<typename T>
    class ZigzagVarintType : public VarintType<T> {
        static_assert(std::is_signed<T>::value, "Zigzaged varint is meant for signed integer type");

    public:
        ZigzagVarintType()
            : ValueElementType<T>()
        { }
        ZigzagVarintType(const ZigzagVarintType<T>&) = default;
        ZigzagVarintType<T>& operator=(const ZigzagVarintType<T>&) = default;
        ~ZigzagVarintType() = default;

    public:
        uint32_t Deserialize(const uint8_t data[], const uint32_t length) override
        {
            ASSERT(data != nullptr);
            uint32_t result = 0;
            result = VarintType<T>::ReadVarint(data, length, ValueElementType<T>::Value());
            if (result != 0) {
                T& value = ValueElementType<T>::Value();
                // unzigzag the value...
                if (value & 1) {
                    value ^= static_cast<T>(-1);
                }
                value >>= 1;
            }
            ValueElementType<T>::Set(result != 0);
            return (result);
        }
    }; // class ZigzagVarintType

    template<typename T>
    class FixedType : public ValueElementType<T> {
    protected:
        FixedType()
            : ValueElementType<T>()
        { }
        FixedType(const FixedType<T>&) = default;
        FixedType<T>& operator=(const FixedType<T>&) = default;
        ~FixedType() = default;

    public:
        uint32_t Deserialize(const uint8_t data[], const uint32_t length) override
        {
            ASSERT(data != nullptr);
            uint32_t result = 0;
            result = ReadFixed(data, length, ValueElementType<T>::Value());
            ValueElementType<T>::Set(result != 0);
            return (result);
        }

    private:
        static uint8_t ReadFixed(const uint8_t data[], const uint32_t length, T& out)
        {
            ASSERT(data != nullptr);
            uint8_t result = 0;
            if (length >= sizeof(out)) {
                result = static_cast<uint8_t>(sizeof(out));
#ifdef LITTLE_ENDIAN_PLATFORM
                /* Can be int, float or double here... */
                ::memcpy(&out, data, sizeof(out));
#else
#error ReadFixed not implemented for big endian
#endif
            }
            return (result);
        }
    }; // class FixedType

    template<typename T>
    class Fixed32Type : public FixedType<T> {
        static_assert(std::is_arithmetic<T>::value && (sizeof(T) == sizeof(uint32_t)), "Fixed32 requires int32 or float type");

    public:
        Fixed32Type()
            : FixedType<T>()
        { }
        Fixed32Type(const Fixed32Type<T>&) = default;
        Fixed32Type<T>& operator=(const Fixed32Type<T>&) = default;
        ~Fixed32Type() = default;

    public:
        using typename ValueElementType<T>::WireType;
        WireType Type() const override {
            return (WireType::FIXED32);
        }
    }; // class Fixed32Type

    template<typename T>
    class Fixed64Type : public FixedType<T> {
        static_assert(std::is_arithmetic<T>::value && (sizeof(T) == sizeof(uint64_t)), "Fixed64 requires int64 or double type");

    public:
        Fixed64Type()
            : FixedType<T>()
        { }
        Fixed64Type(const Fixed64Type<T>&) = default;
        Fixed64Type<T>& operator=(const Fixed64Type<T>&) = default;
        ~Fixed64Type() = default;

    public:
        using typename ValueElementType<T>::WireType;
        WireType Type() const override {
            return (WireType::FIXED64);
        }
    }; // class Fixed64Type

    template<typename T>
    class BytesType : public ValueElementType<std::basic_string<T>> {
    public:
        using type = std::basic_string<T>;
        BytesType()
            : ValueElementType<type>()
        { }
        BytesType(const BytesType&) = default;
        BytesType& operator=(const BytesType&) = default;
        ~BytesType() = default;

    public:
        using typename ValueElementType<type>::WireType;
        uint32_t Deserialize(const uint8_t data[], const uint32_t length) override
        {
            ASSERT(data != nullptr);
            uint32_t result = 0;
            const uint8_t* ptr = data;
            VarintType<uint32_t> size{};
            result = size.Deserialize(ptr, length);
            if ((result != 0) && (size.IsSet() == true) && (size.Value() != 0) && (size.Value() <= length)) {
                ptr += result;
                if ((ptr + size.Value()) <= (data + length)) {
                    ValueElementType<type>::Value().append(reinterpret_cast<const T*>(ptr), size.Value());
                    result += size.Value();
                }
            }
            return (result);
        }
        WireType Type() const override {
            return (WireType::LENGTH_DELIMITED);
        }
    }; // class BytesType

    template<typename ELEMENT>
    class RepeatedType : public IElement{
    public:
        using type = typename ELEMENT::type;
        RepeatedType()
            : _elements()
        { }
        RepeatedType(const RepeatedType&) = default;
        RepeatedType& operator=(const RepeatedType&) = default;
        ~RepeatedType() = default;

    public:
        bool IsSet() const override {
            return (_elements.empty() == false);
        }
        const std::list<ELEMENT>& Elements() const {
            return (_elements);
        }
        void Clear() {
            _elements.clear();
        }
        uint32_t Deserialize(const uint8_t data[], const uint32_t length) override
        {
            ASSERT(data != nullptr);
            ELEMENT element{};
            uint32_t result = element.Deserialize(data, length);
            if (result != 0) {
                _elements.push_back(std::move(element));
            }
            return (result);
        }
        WireType Type() const override {
            ELEMENT element{};
            return (element.Type());
        }
    private:
        std::list<ELEMENT> _elements;
    }; // class RepeatedType

    template<typename ELEMENT>
    class PackedRepeatedType : public IElement {
        static_assert(std::is_arithmetic<typename ELEMENT::type>::value, "PackedRepeated requires elements of numerical type");

    public:
        using type = typename ELEMENT::type;
        PackedRepeatedType()
            : _elements()
        { }
        PackedRepeatedType(const PackedRepeatedType&) = default;
        PackedRepeatedType& operator=(const PackedRepeatedType&) = default;
        ~PackedRepeatedType() = default;

    public:
        bool IsSet() const override {
            return (_elements.empty() == false);
        }
        const std::list<ELEMENT>& Elements() const {
            return (_elements);
        }
        void Clear() {
            _elements.clear();
        }
        uint32_t Deserialize(const uint8_t data[], const uint32_t length) override
        {
            ASSERT(data != nullptr);
            uint32_t result = 0;
            const uint8_t* ptr = data;
            VarintType<uint32_t> size{};
            result = size.Deserialize(ptr, length);
            ptr += result;
            if ((result != 0) && ((size.IsSet() == true) && (size.Value() != 0) && (size.Value() <= (length - result)))) {
                const uint8_t* end = (ptr + size.Value());
                while (ptr < end) {
                    ELEMENT element{};
                    uint32_t consumed = element.Deserialize(ptr, static_cast<uint32_t>(end - ptr));
                    if (consumed != 0) {
                        ptr += consumed;
                        result += consumed;
                        _elements.push_back(element);
                    } else {
                        // TRACE_L1("Failed to deserialize packed element");
                        result = 0;
                        break;
                    }
                }
            }
            return (result);
        }

    private:
        std::list<ELEMENT> _elements;
    }; // class PackedRepeatedType

    class Message : public IElement {
    private:
        struct Entry {
            IElement* element;
            bool required;
        };

    public:
        using type = void;
        Message()
            : _elements()
        { }
        Message(const Message&) = default;
        Message& operator=(const Message&) = default;
        ~Message() = default;

    public:
        bool IsSet() const override {
            return (_elements.empty() == false);
        }
        void Clear() {
            _elements.clear();
        }
        void Add(const uint8_t index, IElement* element, bool required = false)
        {
            ASSERT(index != 0); // 0 is not allowed as key
            ASSERT(element != nullptr);
            _elements.emplace(index, Entry{ element, required });
        }
        bool IsValid() const
        {
            bool valid = true;
            for (auto const& entry : _elements) {
                VARIABLE_IS_NOT_USED const IElement* const& element = entry.second.element;
                ASSERT(element != nullptr);
                if ((entry.second.required == true) && (entry.second.element->IsSet() == false)) {
                    valid = false;
                    break;
                }
            }
            return (valid);
        }
        bool FromBuffer(const uint8_t data[], const uint32_t length)
        {
            ASSERT(data != nullptr);
            bool result = true;
            const uint8_t* ptr = data;
            const uint8_t* end = (ptr + length);
            while (ptr < end) {
                VarintType<uint64_t> tag{};
                uint32_t tagSize = tag.Deserialize(ptr, static_cast<uint32_t>(end - ptr));
                if (tagSize > 0) {
                    const IElement::WireType type = static_cast<IElement::WireType>(tag.Value() & 0x7);
                    const uint32_t key = static_cast<uint32_t>(tag.Value() >> 3);
                    const uint32_t available = static_cast<uint32_t>(end - ptr - tagSize);
                    uint32_t consumed = 0;
                    ptr += tagSize;

                    if (key != 0) {
                        auto it = _elements.find(key);
                        if (it != _elements.end()) {
                            IElement* const& element = (it->second).element;
                            ASSERT(element != nullptr);
                            if (type == element->Type()) {
                                consumed = element->Deserialize(ptr, available);
                            } else {
                                // TRACE_L1("Wire type mismatch, check proto definition");
                            }
                        } else {
                            consumed = Skip(ptr, available, type);
                        }
                    }

                    if (consumed == 0) {
                        // TRACE_L1("Failed to parse element");
                        result = false;
                        break;
                    }

                    ptr += consumed;
                } else {
                    // TRACE_L1("Failed to parse key/value pair");
                    result = false;
                    break;
                }
            }
            return (result);
        }

    public:
        uint32_t Deserialize(const uint8_t data[], const uint32_t length) override
        {
            ASSERT(data != nullptr);
            uint32_t result = 0;
            const uint8_t* ptr = data;
            VarintType<uint32_t> size{};
            result = size.Deserialize(ptr, length);
            if ((result != 0) && (size.IsSet() == true) && (size.Value() > 0) && (size.Value() <= (length - result))) {
                ptr += result;
                if (FromBuffer(ptr, size.Value()) == true) {
                    result += size.Value();
                } else {
                    // TRACE_L1("Failed to parse message");
                }
            }
            return (result);
        }
        WireType Type() const {
            return (WireType::LENGTH_DELIMITED);
        }

    private:
        static uint32_t Skip(const uint8_t data[], const uint32_t length, const WireType type)
        {
            ASSERT(data != nullptr);
            uint32_t result = 0;
            switch (type) {
            case WireType::VARINT:
                result = Skip<VarintType<uint64_t>>(data, length);
                break;
            case WireType::FIXED64:
                result = Skip<Fixed64Type<uint64_t>>(data, length);
                break;
            case WireType::LENGTH_DELIMITED:
                result = Skip<BytesType<uint8_t>>(data, length);
                break;
            case WireType::FIXED32:
                result = Skip<Fixed32Type<uint32_t>>(data, length);
                break;
            case WireType::GROUP_START:
            case WireType::GROUP_END:
                /* deprecated and not supported here */
                // TRACE_L1("Groups are not supported!");
                break;
            default:
                // TRACE_L1("Unknown wire type!");
                break;
            }
            return (result);
        }
        template<typename ELEMENT>
        static uint32_t Skip(const uint8_t data[], const uint32_t length)
        {
            ASSERT(data != nullptr);
            ELEMENT element{};
            return (element.Deserialize(data, length));
        }

    private:
        std::map<uint8_t, Entry> _elements;
    }; // class Message

    using Bytes = BytesType<uint8_t>;
    using Utf8String = BytesType<char>;
    using Bool = VarintType<bool>;
    using Int32 = VarintType<int32_t>;
    using Int64 = VarintType<int64_t>;
    using UInt32 = VarintType<uint32_t>;
    using UInt64 = VarintType<uint64_t>;
    using SInt32 = ZigzagVarintType<int32_t>;
    using SInt64 = ZigzagVarintType<int64_t>;
    using Fixed32 = Fixed32Type<uint32_t>;
    using Fixed64 = Fixed64Type<uint64_t>;
    using SFixed32 = Fixed32Type<int32_t>;
    using SFixed64 = Fixed64Type<int64_t>;
    using Float = FixedType<float>;
    using Double = FixedType<double>;
    template<typename T> using EnumType = VarintType<T>;

}; // namespace Protobuf

} // namespace
