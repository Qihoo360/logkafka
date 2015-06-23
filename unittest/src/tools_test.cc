#define protected public
#define private public
#include "base/tools.h"
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#undef protected
#undef private
#include "gtest/gtest.h"

class ToolsTest: public ::testing::Test {
protected:
    ToolsTest() {
    }

    virtual ~ToolsTest() {
    }
    
    virtual void SetUp() {
    }

    virtual void TearDown() {
    }

};

TEST_F (ToolsTest, Explode) {
    vector<string> v_in;
    v_in.push_back("a");
    v_in.push_back("bb");
    v_in.push_back("ccc");
    
    string str;
    vector<string>::iterator iter = v_in.begin();
    for (; iter < v_in.end(); ++iter) {
        str.append(*iter);
        str.append(":");
    }

    vector<string> v_out = explode(str, ':');

    EXPECT_EQ(true, v_in == v_out);
}

TEST_F (ToolsTest, SearchFiles) {
    const string dir = "/tmp/logkafka_unittest_ToolsTest_SearchFiles";
    int status = mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);    

    const string file_prefix = dir + "/test.txt";
    const int file_num = 10;
    vector<string> files;
    for (int i = 0; i < file_num; ++i) {
        string file = file_prefix + int2Str(i);
        files.push_back(file);
        std::ofstream outfile (file.c_str());
        outfile << "my text here!" << std::endl;
        outfile.close();
    }

    string pattern = file_prefix + "[0-9]*";
    vector<string> files_searched = searchFiles(pattern);
    EXPECT_EQ(true, files == files_searched);
}

TEST_F (ToolsTest, Ltrim) {
    char str_raw[] = "   /abcdefg\\/*abc)!`~     ";
    const char *str_ltrimed = "/abcdefg\\/*abc)!`~     ";
    ltrim(str_raw);
    EXPECT_STREQ(str_ltrimed, str_raw);
}

TEST_F (ToolsTest, Rtrim) {
    char str_raw[] = "   /abcdefg\\/*abc)!`~     ";
    const char *str_rtrimed = "   /abcdefg\\/*abc)!`~";
    rtrim(str_raw);
    EXPECT_STREQ(str_rtrimed, str_raw);
}
