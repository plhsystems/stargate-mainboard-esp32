#ifndef _RINGU_H_
#define _RINGU_H_

class RingU
{
    public:

    private:
    RingU();

    public:
    // Singleton pattern
    RingU(RingU const&) = delete;
    void operator=(RingU const&) = delete;

    public:
    void Init();
    void Start();

    static RingU& getI()
    {
        static RingU instance;
        return instance;
    }
    private:
};

#endif
