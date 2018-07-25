
template<typename Function, typename... Args>
void gl_exec(Function func, Args... args) {
    func(args...);
#ifndef NDEBUG    
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) 
    {
       	std::cerr << "GL error " << err << std::endl;
        __debugbreak();
    }
#endif    
}