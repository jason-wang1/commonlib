#pragma once

namespace TDPredict
{
    union FieldValue
    {
        long field_value() const noexcept
        {
            return fv;
        }

        int field() const noexcept
        {
            return f;
        }

        int value() const noexcept
        {
            return v;
        }

        void set_feild_value(const long field_value) noexcept
        {
            this->fv = field_value;
        }

        void set_feild_value(const int field, const int value) noexcept
        {
            this->f = field;
            this->v = value;
        }

        FieldValue()
        {
            this->fv = 0;
        }

        FieldValue(const FieldValue &other)
        {
            this->fv = other.fv;
        }

        FieldValue(const long field_value)
        {
            this->fv = field_value;
        }

        FieldValue(const int field, const int value)
        {
            this->f = field;
            this->v = value;
        }

        ~FieldValue() {}

        FieldValue &operator=(const FieldValue &other)
        {
            this->fv = other.fv;
            return *this;
        }

        FieldValue &operator=(const long field_value)
        {
            this->fv = field_value;
            return *this;
        }

    private:
        long fv;
        struct // 注意存储是小端序
        {
            int v;
            int f;
        };
    };
}
