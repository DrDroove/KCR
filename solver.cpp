#include <vector>
#include <cmath>
#include <algorithm>
#include <iomanip>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

using namespace std;

// Константа Пи
const double PI = 3.14159265358979323846;

// Точное решение u*(x, y)
double u_exact(double x, double y) {
    return exp(pow(sin(PI * x * y), 2));
}

// Правая часть f*(x, y) = -Delta u*
double f_exact(double x, double y) {
    double arg = PI * x * y;
    double E = exp(pow(sin(arg), 2));
    double term1 = pow(sin(2.0 * arg), 2);
    double term2 = 2.0 * cos(2.0 * arg);
    return -E * PI * PI * (x * x + y * y) * (term1 + term2);
}


class solution{
    private:
    int N, M;
    const double a_dom, b_dom, c_dom, d_dom;
    double h, k;
    int num_nodes;
    double eps_mem;
    int N_max;
    double max_residual=0.0, eps_1=0.0, max_err_x=0.0, max_err_y=0.0;
    double level_of_significance = 1e-8;
    public:
    solution(int n, int m, double a, double b, double c, double d, double e_m, int n_max)
        : N(n), M(m), a_dom(a), b_dom(b), c_dom(c), d_dom(d), eps_mem(e_m), N_max(n_max){
        h = (b_dom - a_dom) / n;
        k = (d_dom - c_dom) / m;
        num_nodes = (n + 1) * (m + 1);
    }
    ~solution(){}

    bool is_inside(double x, double y) {
    // Проверка на выход за границы объемлющего прямоугольника
        if (x < a_dom || x > b_dom || y < c_dom || y > d_dom) return false;
        
        if (x > 0.75 && y < 1) return false;
           
        if (x > 0.25 && x < 0.5 && y > 1 && y < 1.5) return false;
        
        return true;
    }

    bool is_boundary(double x, double y) {
        if (!is_inside(x, y)) return false;
        // Если хотя бы один сосед находится вне области, точка - граничная
        if (!is_inside(x + h, y) || !is_inside(x - h, y) || 
            !is_inside(x, y + k) || !is_inside(x, y - k)) {
            return true;
        }
        return false;
    }

    vector<double> solve() {
        // Вектор текущего приближения v и вектор невязки R (одномерные массивы)
        vector<double> v(num_nodes, 0.0);
        vector<double> R(num_nodes, 0.0);
        vector<double> f_vec(num_nodes, 0.0);
        
        // Инициализация
        for (int j = 0; j <= M; ++j) {
            for (int i = 0; i <= N; ++i) {
                int idx = j * (N + 1) + i;
                double x = a_dom + i * h;
                double y = c_dom + j * k;
                
                if (is_inside(x, y)) {
                    f_vec[idx] = f_exact(x, y);
                    if (is_boundary(x, y)) {
                        v[idx] = u_exact(x, y); // Граничные условия
                    } else {
                        v[idx] = 0.0; // Начальное нулевое приближение для внутренних узлов
                    }
                }
            }
        }
        
        // Расчет параметра tau для прямоугольника
        double lambda_max = 4.0 / (h * h) + 4.0 / (k * k);
        double lambda_min = (PI * PI) / pow(b_dom - a_dom, 2) + (PI * PI) / pow(d_dom - c_dom, 2);
        double tau = 2.0 / (lambda_max + lambda_min);
        
        int iter = 0;
        max_residual = 0.0;
        eps_1 = 0.0;
        max_err_x = 0.0, max_err_y = 0.0;
        
        // Основной цикл МПИ
        do {
            max_residual = 0.0;
            
            // 1. Вычисление невязки R = A*v - f
            for (int j = 1; j < M; ++j) {
                for (int i = 1; i < N; ++i) {
                    double x = a_dom + i * h;
                    double y = c_dom + j * k;
                    
                    // Считаем невязку только для внутренних узлов области G
                    if (is_inside(x, y) && !is_boundary(x, y)) {
                        int idx = j * (N + 1) + i;
                        double laplacian = (-v[idx - 1] + 2.0 * v[idx] - v[idx + 1]) / (h * h) + 
                                        (-v[idx - (N + 1)] + 2.0 * v[idx] - v[idx + (N + 1)]) / (k * k);
                        
                        R[idx] = laplacian - f_vec[idx];
                        
                        if (abs(R[idx]) > max_residual) {
                            max_residual = abs(R[idx]);
                        }
                    }
                }
            }
            
            // 2. Обновление решения v = v - tau * R
            for (int j = 1; j < M; ++j) {
                for (int i = 1; i < N; ++i) {
                    double x = a_dom + i * h;
                    double y = c_dom + j * k;
                    if (is_inside(x, y) && !is_boundary(x, y)) {
                        int idx = j * (N + 1) + i;
                        v[idx] = v[idx] - tau * R[idx];
                    }
                }
            }

            // Расчет погрешности epsilon_1
        
        eps_1=0; max_err_x =0; max_err_y =0;
        
        for (int j = 0; j <= M; ++j) {
            for (int i = 0; i <= N; ++i) {
                double x = a_dom + i * h;
                double y = c_dom + j * k;
                if (is_inside(x, y)) {
                    int idx = j * (N + 1) + i;
                    double err = abs(u_exact(x, y) - v[idx]);
                    if (err > eps_1) {
                        eps_1 = err;
                        max_err_x = x;
                        max_err_y = y;
                    }
                }
            }
        }
            
            iter++;
        } while (eps_1 > eps_mem && iter < N_max && tau*max_residual >= level_of_significance);
        
        return v;
    }
    double get_eps(){
        return eps_1;
    }
    double get_max_residual(){
        return max_residual;
    }
    std::pair<double, double> get_max_residual_coords(){
        return std::make_pair(max_err_x, max_err_y);
    }

};

PYBIND11_MODULE(solver, m) {
    m.doc() = "C++ PDE Solver wrapped with Pybind11";

    pybind11::class_<solution>(m, "Solution")
        .def(pybind11::init<int, int, double, double, double, double, double, int>(),
             pybind11::arg("n"), pybind11::arg("m"), pybind11::arg("a"), pybind11::arg("b"), 
             pybind11::arg("c"), pybind11::arg("d"), pybind11::arg("e_m"), pybind11::arg("n_max"))
        .def("solve", &solution::solve)
        .def("get_eps", &solution::get_eps)
        .def("get_max_residual", &solution::get_max_residual)
        .def("get_max_residual_coords", &solution::get_max_residual_coords);
}