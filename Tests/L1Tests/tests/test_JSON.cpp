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

TEST(JSONTest, solidus_JsonObject_FromString)
{
    const string example = _T("{\"url\":\"http://example.com\"}");
    const string exampleEscaped = _T("{\"url\":\"http:\\/\\/example.com\"}");
    JsonObject jsonObject;
    jsonObject.FromString(example);
    EXPECT_EQ(string(_T("http://example.com")), jsonObject[_T("url")].String());
    string cString;
    jsonObject.ToString(cString);
    EXPECT_EQ(exampleEscaped, cString);
}

TEST(JSONTest, solidus_JsonObject_FromString_Escaped)
{
    const string example = _T("{\"url\":\"http://example.com\"}");
    const string exampleEscaped = _T("{\"url\":\"http:\\/\\/example.com\"}");
    JsonObject jsonObject;
    jsonObject.FromString(exampleEscaped);
    EXPECT_EQ(string(_T("http://example.com")), jsonObject[_T("url")].String());
    string cString;
    jsonObject.ToString(cString);
    EXPECT_EQ(exampleEscaped, cString);
}

TEST(JSONTest, solidus_JSON_String_FromString)
{
    const string example = _T("http://example.com");
    const string exampleEscapedQuoted = _T("\"http:\\/\\/example.com\"");
    Core::JSON::String jsonString;
    jsonString.FromString(example);
    EXPECT_EQ(example, jsonString.Value());
    string cString;
    jsonString.ToString(cString);
    EXPECT_EQ(exampleEscapedQuoted, cString);
}

TEST(JSONTest, solidus_JSON_String_FromString_Escaped)
{
    const string example = _T("http://example.com");
    const string exampleEscaped = _T("http:\\/\\/example.com");
    const string exampleEscapedQuoted = _T("\"http:\\/\\/example.com\"");
    Core::JSON::String jsonString;
    jsonString.FromString(exampleEscaped);
    EXPECT_EQ(example, jsonString.Value());
    string cString;
    jsonString.ToString(cString);
    EXPECT_EQ(exampleEscapedQuoted, cString);
}

class Payload : public Core::JSON::Container {
public:
    Payload()
        : Core::JSON::Container()
    {
        Add(_T("url"), &URL);
    }

    Core::JSON::String URL;
};

TEST(JSONTest, JSONContainerFromString)
{
    const string example = _T("http://example.com");
    const string exampleContainer = _T("{\"url\":\"http://example.com\"}");
    Payload payload;
    EXPECT_TRUE(payload.FromString(exampleContainer));
    EXPECT_TRUE(payload.URL.IsSet());
    EXPECT_EQ(example, payload.URL.Value());
}

TEST(JSONTest, JSONContainerFromStringEmpty)
{
    const string example = _T("");
    const string exampleContainer = _T("{}");
    Payload payload;
    EXPECT_FALSE(payload.FromString(exampleContainer));
    EXPECT_FALSE(payload.URL.IsSet());
    EXPECT_EQ(example, payload.URL.Value());
}

TEST(JSONTest, JSONContainerFromStringExtra)
{
    const string example = _T("http://example.com");
    const string exampleContainer = _T("{\"url\":\"http://example.com\",\"hash\":\"7c35a3ce607a14953f070f0f83b5d74c2296ef93\"}");
    Payload payload;
    EXPECT_TRUE(payload.FromString(exampleContainer));
    EXPECT_TRUE(payload.URL.IsSet());
    EXPECT_EQ(example, payload.URL.Value());
}

TEST(JSONTest, JSONContainerFromStringDifferent)
{
    const string example = _T("");
    const string exampleContainer = _T("{\"hash\":\"7c35a3ce607a14953f070f0f83b5d74c2296ef93\"}");
    Payload payload;
    EXPECT_TRUE(payload.FromString(exampleContainer));
    EXPECT_FALSE(payload.URL.IsSet());
    EXPECT_EQ(example, payload.URL.Value());
}

TEST(JSONTest, JSONContainerFromStringWrong)
{
    const string example = _T("http://example.com");
    Payload payload;
    EXPECT_FALSE(payload.FromString(example));
    EXPECT_FALSE(payload.URL.IsSet());
    payload.URL = example;
    EXPECT_TRUE(payload.URL.IsSet());
    EXPECT_EQ(example, payload.URL.Value());
}
