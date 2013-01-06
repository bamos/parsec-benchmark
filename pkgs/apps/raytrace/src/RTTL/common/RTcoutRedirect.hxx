/// \file RTcoutRedirect.hxx
/// Redirects all cout output to "Output" window of MS Visual Studio IDE.
/// This is define only for Windows running in _DEBUG mode in IDE, it has no
/// effect on other modes and OS.
/// This file is supposed to be included once in the project for which
/// redirection is desired.

#if defined(_WIN32) && defined(_DEBUG)
#include <streambuf>
/// Statically created object to allow cout redirection.
class RTcoutRedirect: public std::streambuf {
public:
    RTcoutRedirect() {
        m_cout_buffer = std::cout.rdbuf();
        std::cout.rdbuf(this);
    }

private:
    int_type overflow(int_type c) {
        if (!traits_type::eq_int_type(c, traits_type::eof())) {
            char s[] = {c,0};
            OutputDebugStringA(s);
            c = m_cout_buffer->sputc(c);
            return c;
        } else {
            return traits_type::not_eof(c);
        }
    }
    std::streambuf* m_cout_buffer;
} redirected_cout;
#endif
