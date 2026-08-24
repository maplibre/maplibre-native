#include <iostream>
#include <mln/test/util.hpp>

#include <mln/util/token.hpp>

using namespace mln;

TEST(Token, replaceTokens) {
    EXPECT_EQ("literal", mln::util::replaceTokens("literal", [](const std::string& token) -> std::string {
                  if (token == "name") return "14th St NW";
                  return "";
              }));
    EXPECT_EQ("14th St NW", mln::util::replaceTokens("{name}", [](const std::string& token) -> std::string {
                  if (token == "name") return "14th St NW";
                  return "";
              }));
    EXPECT_EQ("", mln::util::replaceTokens("{name}", [](const std::string& token) -> std::string {
                  if (token == "text") return "14th St NW";
                  return "";
              }));
    EXPECT_EQ("1400", mln::util::replaceTokens("{num}", [](const std::string& token) -> std::string {
                  if (token == "num") return "1400";
                  return "";
              }));
    EXPECT_EQ("500 m", mln::util::replaceTokens("{num} m", [](const std::string& token) -> std::string {
                  if (token == "num") return "500";
                  return "";
              }));
    EXPECT_EQ("3 Fine Fields", mln::util::replaceTokens("{a} {b} {c}", [](const std::string& token) -> std::string {
                  if (token == "a") return "3";
                  if (token == "b") return "Fine";
                  if (token == "c") return "Fields";
                  return "";
              }));
    EXPECT_EQ(" but still",
              mln::util::replaceTokens("{notset} but still", [](const std::string&) -> std::string { return ""; }));
    EXPECT_EQ("dashed", mln::util::replaceTokens("{dashed-property}", [](const std::string& token) -> std::string {
                  if (token == "dashed-property") return "dashed";
                  return "";
              }));
    EXPECT_EQ("colonized", mln::util::replaceTokens("{colon:property}", [](const std::string& token) -> std::string {
                  if (token == "colon:property") return "colonized";
                  return "";
              }));
    EXPECT_EQ("150 m", mln::util::replaceTokens("{HØYDE} m", [](const std::string& token) -> std::string {
                  if (token == "HØYDE") return "150";
                  return "";
              }));
    EXPECT_EQ("{unknown}", mln::util::replaceTokens("{unknown}", [](const std::string&) -> std::optional<std::string> {
                  return {};
              }));
}
