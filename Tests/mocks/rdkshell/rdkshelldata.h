#pragma once

#include <typeindex>
#include <string>

namespace RdkShell
{
    union RdkShellDataInfo
    {
        bool        booleanData;
        int8_t      integer8Data;
        int32_t     integer32Data;
        int64_t     integer64Data;
        uint8_t     unsignedInteger8Data;
        uint32_t    unsignedInteger32Data;
        uint64_t    unsignedInteger64Data;
        float       floatData;
        double      doubleData;
        std::string* stringData;
        void*       pointerData;
        RdkShellDataInfo() {}
        ~RdkShellDataInfo() {}
    };

    class RdkShellData
    {
        public:
            RdkShellData(): mDataTypeIndex(typeid(void*))
           {
                   mData.pointerData = nullptr;
                   mData.stringData = nullptr;
           }
            ~RdkShellData()
            {
              if (mDataTypeIndex == typeid(std::string))
              {
                if (nullptr != mData.stringData)
               {
                 delete mData.stringData;
                 mData.stringData = nullptr;
               }
              }
            }
            RdkShellData(bool data);
            RdkShellData(int8_t data);
            RdkShellData(int32_t data);
            RdkShellData(int64_t data);
            RdkShellData(uint8_t data);
            RdkShellData(uint32_t data);
            RdkShellData(uint64_t data);
            RdkShellData(float data);
            RdkShellData(double data);
            RdkShellData(std::string data);
            RdkShellData(void* data);
            
            bool toBoolean() const{return true;}
            int8_t toInteger8() const{return 0;}
            int32_t toInteger32() const{return 0;}
            int64_t toInteger64() const{return 0;}
            uint8_t toUnsignedInteger8() const{return 0;}
            uint32_t toUnsignedInteger32() const{return 0;}
            uint64_t toUnsignedInteger64() const{return 0;}
            float toFloat() const{return 0.0;}
            double toDouble() const{return 0.0;}
            std::string toString() const{return "a";}
            void* toVoidPointer() const{return NULL;}

            RdkShellData& operator=(bool value){return *this;}
            RdkShellData& operator=(int8_t value){return *this;}
            RdkShellData& operator=(int32_t value){return *this;}
            RdkShellData& operator=(int64_t value){return *this;}
            RdkShellData& operator=(uint8_t value){return *this;}
            RdkShellData& operator=(uint32_t value){return *this;}
            RdkShellData& operator=(uint64_t value){return *this;}
            RdkShellData& operator=(float value){return *this;}
            RdkShellData& operator=(double value){return *this;}
            RdkShellData& operator=(const char* value){return *this;}
            RdkShellData& operator=(const std::string& value){return *this;}
            RdkShellData& operator=(void* value){return *this;}
            RdkShellData& operator=(const RdkShellData& value){return *this;}

            std::type_index dataTypeIndex();

        private:
            std::type_index mDataTypeIndex;
            RdkShellDataInfo mData;

            void setData(std::type_index typeIndex, void* data){return;}
    };
}

