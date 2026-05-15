#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <iomanip>

using namespace std;

// Параметры области
const double a_dom = 0.0, b_dom = 1.0;
const double c_dom = 0.0, d_dom = 2.0;

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

// Функция, определяющая принадлежность точки (x, y) области G и границе dG.
// ВАЖНО: Ниже представлена аппроксимация вырезов для Области №6.
bool is_inside(double x, double y) {
    // Проверка на выход за границы объемлющего прямоугольника
    if (x < a_dom || x > b_dom || y < c_dom || y > d_dom) return false;
    
   
    if (x > 0.75 && y < 1) return false;
    
    
    if (x > 0.25 && x < 0.5 && y > 1 && y < 1.5) return false;
    
    return true;
}

// Проверка, является ли точка граничной (dG)
bool is_boundary(double x, double y, double h, double k) {
    if (!is_inside(x, y)) return false;
    // Если хотя бы один сосед находится вне области, точка - граничная
    if (!is_inside(x + h, y) || !is_inside(x - h, y) || 
        !is_inside(x, y + k) || !is_inside(x, y - k)) {
        return true;
    }
    return false;
}

int main() {
    // Параметры сетки (выбираем кратно вырезам)
    int n = 80; 
    int m = 160; 
    double h = (b_dom - a_dom) / n;
    double k = (d_dom - c_dom) / m;
    
    int num_nodes = (n + 1) * (m + 1);
    
    // Вектор текущего приближения v и вектор невязки R (одномерные массивы)
    vector<double> v(num_nodes, 0.0);
    vector<double> R(num_nodes, 0.0);
    vector<double> f_vec(num_nodes, 0.0);
    
    // Критерии остановки
    double eps_mem = 1e-7;
    int N_max = 5000;
    
    // Инициализация
    for (int j = 0; j <= m; ++j) {
        for (int i = 0; i <= n; ++i) {
            int idx = j * (n + 1) + i;
            double x = a_dom + i * h;
            double y = c_dom + j * k;
            
            if (is_inside(x, y)) {
                f_vec[idx] = f_exact(x, y);
                if (is_boundary(x, y, h, k)) {
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
    double max_residual = 0.0;
    double eps_1 = 0.0;
    double max_err_x = 0.0, max_err_y = 0.0;
    
    // Основной цикл МПИ
    do {
        max_residual = 0.0;
        
        // 1. Вычисление невязки R = A*v - f
        for (int j = 1; j < m; ++j) {
            for (int i = 1; i < n; ++i) {
                double x = a_dom + i * h;
                double y = c_dom + j * k;
                
                // Считаем невязку только для внутренних узлов области G
                if (is_inside(x, y) && !is_boundary(x, y, h, k)) {
                    int idx = j * (n + 1) + i;
                    double laplacian = (-v[idx - 1] + 2.0 * v[idx] - v[idx + 1]) / (h * h) + 
                                       (-v[idx - (n + 1)] + 2.0 * v[idx] - v[idx + (n + 1)]) / (k * k);
                    
                    R[idx] = laplacian - f_vec[idx];
                    
                    if (abs(R[idx]) > max_residual) {
                        max_residual = abs(R[idx]);
                    }
                }
            }
        }
        
        // 2. Обновление решения v = v - tau * R
        for (int j = 1; j < m; ++j) {
            for (int i = 1; i < n; ++i) {
                double x = a_dom + i * h;
                double y = c_dom + j * k;
                if (is_inside(x, y) && !is_boundary(x, y, h, k)) {
                    int idx = j * (n + 1) + i;
                    v[idx] = v[idx] - tau * R[idx];
                }
            }
        }

         // Расчет погрешности epsilon_1
    
    eps_1=0; max_err_x =0; max_err_y =0;
    
    for (int j = 0; j <= m; ++j) {
        for (int i = 0; i <= n; ++i) {
            double x = a_dom + i * h;
            double y = c_dom + j * k;
            if (is_inside(x, y)) {
                int idx = j * (n + 1) + i;
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
    } while (eps_1 > eps_mem && iter < N_max);
    
   
    
    // Вывод справки (по требованиям)
    cout << "--- Справка для тестовой задачи ---" << endl;
    cout << "Сетка-основа: n = " << n << ", m = " << m << endl;
    cout << "Параметр tau: " << tau << endl;
    cout << "Затрачено итераций N: " << iter << endl;
    cout << "Невязка ||R||_max: " << max_residual << endl;
    cout << "Задача решена с погрешностью eps_1 = " << eps_1 << endl;
    cout << "Максимальное отклонение в узле x = " << max_err_x << ", y = " << max_err_y << endl;
    
    return 0;
}