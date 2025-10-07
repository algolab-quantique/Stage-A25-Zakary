
class DPoint
{
private:
    unsigned int begin;
    unsigned int end;

public:
    DPoint(/* args */);
    ~DPoint();

    unsigned int get_begin() const { return begin; }
    unsigned int get_end() const { return end; }
    void set_begin(unsigned int b) { begin = b; }
    void set_end(unsigned int e) { end = e; }
};

DPoint::DPoint(/* args */)
{
}

DPoint::~DPoint()
{
}
