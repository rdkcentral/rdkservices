#include <gtest/gtest.h>

#include "Module.h"

using namespace WPEFramework;

TEST(JSONTest, EMPTY)
{
    JsonObject object;
    EXPECT_FALSE(object.HasLabel(_T("key")));
    EXPECT_EQ(Core::JSON::Variant::type::EMPTY, object[_T("key")].Content());
    EXPECT_EQ(0, object[_T("key")].Number());
    EXPECT_EQ(0.0f, object[_T("key")].Float());
    EXPECT_EQ(0.0, object[_T("key")].Double());
    EXPECT_EQ(_T("null"), object[_T("key")].String());
    EXPECT_EQ(false, object[_T("key")].Boolean());
}

/**
 * FIXME
 * | ===== $$ [6755]: ASSERT [JSON.h:138] (loaded <= payload)
 */
TEST(JSONTest, DOUBLE)
{
    JsonObject object;
    object[_T("key")] = 1.2345;
    EXPECT_TRUE(object.HasLabel(_T("key")));
    EXPECT_EQ(Core::JSON::Variant::type::DOUBLE, object[_T("key")].Content());
    //    EXPECT_EQ(0, object[_T("key")].Number());
    //    EXPECT_EQ(1.2345f, object[_T("key")].Float());
    //    EXPECT_EQ(1.2345, object[_T("key")].Double());
    EXPECT_EQ(_T("1.2345"), object[_T("key")].String());
    EXPECT_EQ(false, object[_T("key")].Boolean());
}

/**
 * FIXME
 * | ===== $$ [6755]: ASSERT [JSON.h:138] (loaded <= payload)
 */
TEST(JSONTest, FLOAT)
{
    JsonObject object;
    object[_T("key")] = 1.2345f;
    EXPECT_TRUE(object.HasLabel(_T("key")));
    EXPECT_EQ(Core::JSON::Variant::type::FLOAT, object[_T("key")].Content());
    //    EXPECT_EQ(0, object[_T("key")].Number());
    //    EXPECT_EQ(1.2345f, object[_T("key")].Float());
    //    EXPECT_EQ(0.0, object[_T("key")].Double());
    EXPECT_EQ(_T("1.2345"), object[_T("key")].String());
    EXPECT_EQ(false, object[_T("key")].Boolean());
}

/**
 * FIXME
 * 1.2345 is deserialized to NUMBER 1 ???
 */
TEST(JSONTest, FloatingPointDeserialized)
{
    JsonObject object;
    object.FromString(_T("{\"key\":1.2345}"));
    EXPECT_TRUE(object.HasLabel(_T("key")));
    EXPECT_EQ(Core::JSON::Variant::type::NUMBER, object[_T("key")].Content());
    EXPECT_EQ(1, object[_T("key")].Number());
    EXPECT_EQ(1.0f, object[_T("key")].Float());
    EXPECT_EQ(1, object[_T("key")].Double());
    EXPECT_EQ(_T("1.2345"), object[_T("key")].String());
    EXPECT_EQ(false, object[_T("key")].Boolean());
}

TEST(JSONTest, NUMBER)
{
    JsonObject object;
    object[_T("key")] = 12345;
    EXPECT_TRUE(object.HasLabel(_T("key")));
    EXPECT_EQ(Core::JSON::Variant::type::NUMBER, object[_T("key")].Content());
    EXPECT_EQ(12345, object[_T("key")].Number());
    EXPECT_EQ(12345.0f, object[_T("key")].Float());
    EXPECT_EQ(12345.0, object[_T("key")].Double());
    EXPECT_EQ(_T("12345"), object[_T("key")].String());
    EXPECT_EQ(false, object[_T("key")].Boolean());
}

TEST(JSONTest, NumberAsStringDeserialized)
{
    JsonObject object;
    object.FromString(_T("{\"key\":\"12345\"}"));
    EXPECT_TRUE(object.HasLabel(_T("key")));
    EXPECT_EQ(Core::JSON::Variant::type::STRING, object[_T("key")].Content());
    EXPECT_EQ(0, object[_T("key")].Number());
    EXPECT_EQ(0.0f, object[_T("key")].Float());
    EXPECT_EQ(0.0, object[_T("key")].Double());
    EXPECT_EQ(_T("12345"), object[_T("key")].String());
    EXPECT_EQ(false, object[_T("key")].Boolean());
}

TEST(JSONTest, BOOLEAN)
{
    JsonObject object;
    object[_T("key")] = true;
    EXPECT_TRUE(object.HasLabel(_T("key")));
    EXPECT_EQ(Core::JSON::Variant::type::BOOLEAN, object[_T("key")].Content());
    EXPECT_EQ(0, object[_T("key")].Number());
    EXPECT_EQ(0.0f, object[_T("key")].Float());
    EXPECT_EQ(0.0, object[_T("key")].Double());
    EXPECT_EQ(_T("true"), object[_T("key")].String());
    EXPECT_EQ(true, object[_T("key")].Boolean());
}

TEST(JSONTest, BooleanTrueAsStringDeserialized)
{
    JsonObject object;
    object.FromString(_T("{\"key\":\"true\"}"));
    EXPECT_TRUE(object.HasLabel(_T("key")));
    EXPECT_EQ(Core::JSON::Variant::type::STRING, object[_T("key")].Content());
    EXPECT_EQ(0, object[_T("key")].Number());
    EXPECT_EQ(0.0f, object[_T("key")].Float());
    EXPECT_EQ(0.0, object[_T("key")].Double());
    EXPECT_EQ(_T("true"), object[_T("key")].String());
    EXPECT_EQ(false, object[_T("key")].Boolean());
}

TEST(JSONTest, BooleanFalseAsStringDeserialized)
{
    JsonObject object;
    object.FromString(_T("{\"key\":\"false\"}"));
    EXPECT_TRUE(object.HasLabel(_T("key")));
    EXPECT_EQ(Core::JSON::Variant::type::STRING, object[_T("key")].Content());
    EXPECT_EQ(0, object[_T("key")].Number());
    EXPECT_EQ(0.0f, object[_T("key")].Float());
    EXPECT_EQ(0.0, object[_T("key")].Double());
    EXPECT_EQ(_T("false"), object[_T("key")].String());
    EXPECT_EQ(false, object[_T("key")].Boolean());
}

TEST(JSONTest, STRING)
{
    JsonObject object;
    object[_T("key")] = _T("value");
    EXPECT_TRUE(object.HasLabel(_T("key")));
    EXPECT_EQ(Core::JSON::Variant::type::STRING, object[_T("key")].Content());
    EXPECT_EQ(0, object[_T("key")].Number());
    EXPECT_EQ(0.0f, object[_T("key")].Float());
    EXPECT_EQ(0.0, object[_T("key")].Double());
    EXPECT_EQ(_T("value"), object[_T("key")].String());
    EXPECT_EQ(false, object[_T("key")].Boolean());
}

TEST(JSONTest, ARRAY)
{
    JsonObject object;
    object[_T("key")] = JsonArray();
    EXPECT_TRUE(object.HasLabel(_T("key")));
    EXPECT_EQ(Core::JSON::Variant::type::ARRAY, object[_T("key")].Content());
    EXPECT_EQ(0, object[_T("key")].Number());
    EXPECT_EQ(0.0f, object[_T("key")].Float());
    EXPECT_EQ(0.0, object[_T("key")].Double());
    EXPECT_EQ(_T("[]"), object[_T("key")].String());
    EXPECT_EQ(false, object[_T("key")].Boolean());
}

TEST(JSONTest, OBJECT)
{
    JsonObject object;
    object[_T("key")] = JsonObject();
    EXPECT_TRUE(object.HasLabel(_T("key")));
    EXPECT_EQ(Core::JSON::Variant::type::OBJECT, object[_T("key")].Content());
    EXPECT_EQ(0, object[_T("key")].Number());
    EXPECT_EQ(0.0f, object[_T("key")].Float());
    EXPECT_EQ(0.0, object[_T("key")].Double());
    EXPECT_EQ(_T("{}"), object[_T("key")].String());
    EXPECT_EQ(false, object[_T("key")].Boolean());
}
