
template <typename T, unsigned int D>
struct Vec {
    static_assert( (std::is_void<T>::value || GL_enum<T>::value)  &&  D <= 4,  "Invalid dimension for vector!" );
    static constexpr int dim = 0;
    using type = void;
};


template <typename T>
struct Vec<T, 1> {
    using type = T;
    static constexpr int dim = 1;
    T x;
    Vec<T, 1>  operator- ()                   const {return {-x};}
    Vec<T, 1>  operator+ (const Vec<T, 1> &o) const {return {x + o.x};}
    Vec<T, 1>  operator- (const Vec<T, 1> &o) const {return {x - o.x};}
    Vec<T, 1> &operator+=(const Vec<T, 1> &o)       {x += o.x; return *this;}
    Vec<T, 1> &operator-=(const Vec<T, 1> &o)       {x -= o.x; return *this;}
};

template <typename T>
struct Vec<T, 2> {
    using type = T;
    static constexpr int dim = 2;
    union {T x, u;};
    union {T y, v;};
    Vec<T, 2>  operator- ()                   const {return {-x, -y};}
    Vec<T, 2>  operator+ (const Vec<T, 2> &o) const {return {x + o.x, y + o.y};}
    Vec<T, 2>  operator- (const Vec<T, 2> &o) const {return {x - o.x, y - o.y};}
    Vec<T, 2> &operator+=(const Vec<T, 2> &o)       {x += o.x; y += o.y; return *this;}
    Vec<T, 2> &operator-=(const Vec<T, 2> &o)       {x -= o.x; y -= o.y; return *this;}
};

template <typename T>
struct Vec<T, 3> {
    using type = T;
    static constexpr int dim = 3;
    union {T x, r;};
    union {T y, g;};
    union {T z, b;};
    Vec<T, 3>  operator- ()                   const {return {-x, -y, -z};}
    Vec<T, 3>  operator+ (const Vec<T, 3> &o) const {return {x + o.x, y + o.y, z + o.z};}
    Vec<T, 3>  operator- (const Vec<T, 3> &o) const {return {x - o.x, y - o.y, z - o.z};}
    Vec<T, 3>  operator* (const Vec<T, 3> &o) const {return {y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x};}
    Vec<T, 3> &operator+=(const Vec<T, 3> &o)       {x += o.x; y += o.y; z += o.z; return *this;}
    Vec<T, 3> &operator-=(const Vec<T, 3> &o)       {x -= o.x; y -= o.y; z -= o.z; return *this;}
    Vec<T, 3> &operator*=(const Vec<T, 3> &o)       {*this = *this * o; return *this;}
};

template <typename T>
struct Vec<T, 4> {
    using type = T;
    static constexpr int dim = 4;
    union {T x, r;};
    union {T y, g;};
    union {T z, b;};
    union {T w, a;};
    Vec<T, 4>  operator- ()                   const {return {-x, -y, -z, -w};}
    Vec<T, 4>  operator+ (const Vec<T, 4> &o) const {return {x + o.x, y + o.y, z + o.z, w + o.w};}
    Vec<T, 4>  operator- (const Vec<T, 4> &o) const {return {x - o.x, y - o.y, z - o.z, w - o.w};}
    Vec<T, 4> &operator+=(const Vec<T, 4> &o)       {x += o.x; y += o.y; z += o.z; w += o.w; return *this;}
    Vec<T, 4> &operator-=(const Vec<T, 4> &o)       {x -= o.x; y -= o.y; z -= o.z; w -= o.w; return *this;}
};


