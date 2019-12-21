#include "gtest/gtest.h"
#include "Entries.h"
#include "CommitEntriesDisk.h"

using namespace raft;

CCommitEntriesDisk commit_disk("E:/file.cache");

TEST(CommitEntriesDisk, case1) {
    Entries entries;
    entries._entries = "it is a test.";
    entries._index = 0;
    entries._term = 20;
    EXPECT_TRUE(commit_disk.PushEntries(entries));
}

TEST(CommitEntriesDisk, case2) {
    EXPECT_TRUE(commit_disk.PushEntries(101, 1, "it is a test."));
}

TEST(CommitEntriesDisk, case3) {
    std::vector<Entries> entries_vec;
    EXPECT_TRUE(commit_disk.GetEntries(0, entries_vec));
}

TEST(CommitEntriesDisk, case4) {
    EXPECT_EQ(1, commit_disk.GetNewestIndex());
}