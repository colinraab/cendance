#pragma once

class StdoutRedirect final {
public:
    explicit StdoutRedirect(bool enabled);
    ~StdoutRedirect();
    void restore();

private:
    int savedStdoutFd = -1;
};
