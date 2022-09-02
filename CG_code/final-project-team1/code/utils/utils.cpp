#include <utils.h>

// Utility functions

void NotImplemented() { Assert(false, "NotImplemented(): Not implemented"); }

string GetFilePath(const string& target, int depth/* = 5*/)
{
    string path = target;
    for (int i = 0; i < depth; ++i) {
        FILE *file = fopen(path.c_str(), "r");
        if (file) {
        fclose(file);
        return path;
        }
        path = "../" + path;
    }
    Error("GetFilePath(): Failed to get file: " + target);
    abort();
    return target;
}