#include "jsonlite.h"
#include <gtest/gtest.h>

using namespace jsonlite;

TEST(jsonliteTest, testEscape)
{
  EXPECT_EQ("aaa\\\"aaa", escape_json_string("aaa\"aaa")) << "double quoation is escaped correclty";
  EXPECT_EQ("aaa\\\\", escape_json_string("aaa\\")) << "last back slash is escaped correclty";
  EXPECT_EQ("\\\"\\/\\\\\\b\\n\\r\\t\\\\m", escape_json_string("\\\"\\/\\\\\\b\\n\\r\\t\\m")) << "control chars passes";
  EXPECT_EQ("\\\"\\/\\\\\\b\\n\\r\\t\\\\m\\\\", escape_json_string("\\\"\\/\\\\\\b\\n\\r\\t\\m\\")) << "control chars passes and last backslash escaped";
  EXPECT_EQ("\\u0a0f\\u0A0F\\\\u090Z\\\\uZZZZ\\\\ubb", escape_json_string("\\u0a0f\\u0A0F\\u090Z\\uZZZZ\\ubb")) << "unicode is escaped correclty";
}

TEST(jsonliteTest, testBuildObject)
{
  std::string json = 
      json_object("Hoge", "Hoge")
                 ("Foo", true)
                 ("AAA", "\"aaa")
                 ("BBB", "\\n\\m")
                 ("Array", json_array("Hoge")("Foo")(false))
                 ("Obj", json_object("SubKey1", "SubValue1")
                                    ("SubKey2", "SubValue2")).str();
  EXPECT_EQ("{\"AAA\":\"\\\"aaa\",\"Array\":[\"Hoge\",\"Foo\",false],\"BBB\":\"\\n\\\\m\",\"Foo\":true,\"Hoge\":\"Hoge\",\"Obj\":{\"SubKey1\":\"SubValue1\",\"SubKey2\":\"SubValue2\"}}", json);
}

