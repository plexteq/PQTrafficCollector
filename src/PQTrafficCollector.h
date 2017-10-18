#pragma once
class PQTrafficCollector
{
    static void *sniffer;
public:
    static void InitAndStart(int argc, char** argv);
    static void Wait();
};

