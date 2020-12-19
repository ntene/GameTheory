/*
 * \file io.hpp
 * \brief io.hpp
 * \author agent for playing the game
 * \course Theory of Computer Game (TCG)
 */

#ifndef IO_HPP
#define IO_HPP

#include <unistd.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <cassert>

#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <stdexcept>
#include <chrono>
#include <algorithm>
#include <string.h>

#include "einstein.hpp"
#include "getch.hpp"

class _com_io {
  public:
    _com_io(_com_io const&) = default;
    // don't allow = operator to copy
    _com_io &operator=(_com_io const&) = delete;
    _com_io &operator=(_com_io const&&a) {
        if (pid != a.pid) {
            suicide();
            pid = a.pid;
            rfd = a.rfd;
            wfd = a.wfd;
            a.pid = -1; // remove process from pointer a
        }
        return (*this);
    }

    _com_io(_com_io &&a) {
        *this = std::move(a);
    }

    ~_com_io() {
        suicide();
    }

    explicit _com_io(std::string const &exec_name) {
        if (init) {
            init = 0;
            struct sigaction sig_catcher;
            sig_catcher.sa_handler = catch_sigpipe;
            sig_catcher.sa_flags = 0;
            if (sigemptyset(&sig_catcher.sa_mask) == -1) {
                throw std::runtime_error("AGENT(exec_name)::sigemptyset error\n");
            }
            if (sigaction(SIGPIPE, &sig_catcher, nullptr) == -1) {
                throw std::runtime_error("AGENT(exec_name)::sigaction error\n");
            }
        }

        // double-end pipe
        int _rfd[2], _wfd[2];
        if (pipe(_rfd) == -1 || pipe(_wfd) == -1) {
            throw std::runtime_error("AGENT(exec_name)::pipe() error\n");
        }
        int _pid = fork();
        if (_pid == -1) {
            throw std::runtime_error("AGENT(exec_name)::fork() error\n");
        }

        // 0 = stdin, 1 = stdout
        if (!_pid) { // child process
            // child reads from wfd[0] writes to rfd[1]
            if (close(_rfd[0]) == -1 || close(_wfd[1]) == -1) {
                throw std::runtime_error("AGENT(exec_name)::child close() error\n");
            }
            // give to stdin/stdout
            if (dup2(_wfd[0], 0) == -1 || dup2(_rfd[1], 1) == -1) {
                throw std::runtime_error("AGENT(exec_name)::child dup2() error\n");
            }
            if (close(_rfd[1]) == -1 || close(_wfd[0]) == -1) {
                throw std::runtime_error("AGENT(exec_name)::child close() error - 1 \n");
            }
            if (execlp(exec_name.c_str(), nullptr) == -1) {
                throw std::runtime_error("AGENT(exec_name)::child execlp() error\n");
            }
        }
        // parent process (judge)
        // parent reads from rfd[0], writes to wfd[1]
        if (close(_rfd[1]) == -1 || close(_wfd[0]) == -1) {
            throw std::runtime_error("AGENT(exec_name)::parent close() error\n");
        }
        pid = _pid;
        rfd = _rfd[0];
        wfd = _wfd[1];
    }

    // -1 if fail
    // does not allow over estimated sz
    std::string recv(std::size_t sz, int timeout_sec) const {
        // int done = 0
        if (pid == -1) {
            assert(false);
            return "";
        }

        std::string res;
        timeout_sec = std::max(timeout_sec, 0);
        int available = timeout_sec * 1e6 + 1e5;
        struct timeval tv;
        tv.tv_sec = timeout_sec;
        tv.tv_usec = 0;

        std::vector<char> buf(sz + 1);
        while (sz) {
#ifdef LINUX
            ::fd_set fds;
#else
            struct fd_set fds;
#endif
            FD_ZERO(&fds);
            FD_SET(rfd, &fds);

            auto t0 = std::chrono::steady_clock::now();
            int ready = select(rfd + 1, &fds, nullptr, nullptr, &tv);
            if (ready < 0) {
                throw std::runtime_error("AGENT::recv::select() error\n");
            } else if (ready == 0) {
                throw std::runtime_error("AGENT::recv::select() timeout\n");
            }

            assert(FD_ISSET(rfd, &fds));

            auto t1 = std::chrono::steady_clock::now();
            int elapsed = 1e6 * std::chrono::duration_cast<std::chrono::duration<double>>(t1 - t0).count();
            if (available - elapsed < 1e5) {
                assert(false);
                return "";
            }
            int buf_read = read(rfd, buf.data(), sz);
            switch (buf_read) {
                case -1:
                    throw std::runtime_error("AGENT::recv::read() error\n");
                case 0:
                    std::cout << "EOF" << ready << " rfd: " << rfd << std::endl;
                    return "";
                default:
                    buf[buf_read] = 0;
                    res += buf.data();
                    std::fill(buf.data(), buf.data() + buf_read, 0);
                    sz -= buf_read;
            }
        }
        return res;
    }

    // return false when write fail
    bool send(char c) const {
        if (pid == -1) {
            return false;
        }
        int buf_write = write(wfd, &c, 1);
        return buf_write;
    }

    // return false when write fail
    bool send(std::string str) {
        if (pid == -1) {
            return false;
        }
        unsigned int offset = 0;
        while (offset < str.size()) {
            int buf_write = write(wfd, str.c_str() + offset, str.size() - offset);
            if (buf_write <= 0) {
                return false;
            }
            offset += buf_write;
        }
        return true;
    }

    // kill self (allow timeout before kill)
    void suicide(int timeout_sec = 0) {
        if (pid == -1) {
            return;
        }
        if (timeout_sec > 0) {
            sleep(timeout_sec);
        }
        if (close(rfd) == -1 || close(wfd) == -1) {
            throw std::runtime_error("AGENT::suicide::close() error\n");
        }
        kill(pid, SIGKILL);
        if (waitpid(pid, nullptr, 0) == -1) {
            throw std::runtime_error("AGENT::suicide::waitpid() error\n");
        }
        std::cerr << "killed " << pid << std::endl;
        pid = -1;
    }

  private:
    static bool init;
    mutable int pid = -1;
    int rfd, wfd; // read/write fd;

    static void catch_sigpipe (int sig) {}
};

bool _com_io::init = 1;

std::string do_human_move(Board &b) {
    std::cout << Ansi::BOLD << Ansi::FG_YELLOW << "Change: Arrow Keys\n";
    std::cout << "Confirm: Z\n" << Ansi::CLEAR;
    MoveList mL;
    int mL_size;
    std::string res;

    if (b.side_to_move() == RED) {
        mL_size = b.legal_actions<RED>(mL);
    } else {
        mL_size = b.legal_actions<BLUE>(mL);
    }
    if (mL_size == 1 && mL[0] == MOVE_PASS) {
        b.do_move(MOVE_PASS);
        return "--";
    }
    b.set_hl(mL[0]);
    std::cout << b << "\n\n";
    
    int now_move = 0;
    bool stay = 1;
    while (stay) {
        char c = getch();
        switch (c) {
            case 'Z': case 'z':
                stay = 0;
                res = b.move_to_str(mL[now_move]);
                b.do_move(mL[now_move]);
                break;
            case '\033':
                c = getch();
                if (c == 0) {
                    std::cout << "\a"; continue;
                }
                c = getch();
                if (!strchr("ABCD", c)) {
                    std::cout << "\a"; continue;
                }
                now_move = (now_move + ((c == 'B' || c == 'C') ? 1 : mL_size - 1)) % mL_size;
                b.set_hl(mL[now_move]);
                break;
        }
        CLR_LINES(FILE_NB + 7);
        std::cout << b << "\n\n";
    }

    return res;
}

class Agent {
  public:
    using IO = _com_io;

    Board *b;
    IO *io = nullptr;
    int time_limit = 10;
    
    Agent() noexcept = default;
    Agent(std::string exec_name, int limit=10) noexcept {
        time_limit = limit;
        io = new IO(exec_name);
    }

    void init(std::string topLeft, std::string bottomRight, char start) noexcept {
        if (io != nullptr) {
            io->send(topLeft);
            io->send(bottomRight);
            io->send(start);
        } else {
            b = new Board();
            b->init(topLeft, bottomRight);
        }
    }

    void restart(char c) noexcept {
        if (io != nullptr) {
            io->send(c);
        }
    }

    void give_move(std::string m) noexcept {
        if (io != nullptr) {
            io->send(m);
        } else {
            b->do_move(m[0], m[1]);
        }
    }

    std::string get_move() {
        if (io != nullptr) {
            return io->recv(2, time_limit);
        } else {
            return do_human_move(*b);
        }
    }
};

#endif // #ifndef IO_HPP
