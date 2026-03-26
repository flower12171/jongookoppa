#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <math.h>
#include <locale.h>

#define N 50

typedef double (*term_func)(double, int);
typedef double (*sum_func)(double*, int);

double term_exp(double x, int n) {
    static double t = 1.0;
    if (n == 0) { t = 1.0; return 1.0; }
    t *= x / n;
    return t;
}

double term_sin(double x, int n) {
    static double t = 1.0;
    if (n == 0) return 0.0;
    if (n == 1) { t = x; return x; }
    t *= -x * x / ((2 * n - 2) * (2 * n - 1));
    return t;
}

double term_cos(double x, int n) {
    static double t = 1.0;
    if (n == 0) return 1.0;
    t *= -x * x / ((2 * n - 1) * (2 * n));
    return t;
}

double term_ln(double x, int n) {
    static double t = 1.0;
    if (n == 0) return 0.0;
    if (n == 1) { t = x; return x; }
    t *= -x * (n - 1) / n;
    return t;
}

double sum_direct(double* a, int n) {
    double s = 0;
    for (int i = 0; i < n; i++) s += a[i];
    return s;
}

double sum_reverse(double* a, int n) {
    double s = 0;
    for (int i = n - 1; i >= 0; i--) s += a[i];
    return s;
}

double sum_kahan(double* a, int n) {
    double s = 0, c = 0;
    for (int i = 0; i < n; i++) {
        double y = a[i] - c;
        double t = s + y;
        c = (t - s) - y;
        s = t;
    }
    return s;
}

// Функция для получения библиотечного значения
double get_library_value(const char* name, double x) {
    if (name[0] == 'e') return exp(x);
    else if (name[0] == 's') return sin(x);
    else if (name[0] == 'c') return cos(x);
    else if (name[0] == 'l') return log(1 + x);
    return 0.0;
}

// Функция для вычисления максимальной погрешности при суммировании
double calculate_max_error(double* terms, int n, sum_func sum_method, double exact_value) {
    double partial_sums[100]; // массив для хранения частичных сумм
    double max_error = 0.0;

    // Вычисляем все частичные суммы
    for (int k = 1; k <= n; k++) {
        // Создаём временный массив для первых k элементов
        double temp[100];
        for (int i = 0; i < k; i++) {
            temp[i] = terms[i];
        }
        partial_sums[k - 1] = sum_method(temp, k);
    }

    // Находим максимальное отклонение от точного значения
    for (int k = 0; k < n; k++) {
        double error = fabs(partial_sums[k] - exact_value);
        if (error > max_error) {
            max_error = error;
        }
    }

    return max_error;
}

void calculate(const char* name, term_func f, double x, sum_func* sums, char** sum_names) {
    double terms[N];
    double exact_value = get_library_value(name, x);

    for (int i = 0; i < N; i++) {
        terms[i] = f(x, i);
    }

    printf("%s:\n", name);
    printf("  Библиотечное значение: %.12f\n\n", exact_value);

    for (int i = 0; i < 3; i++) {
        double res = sums[i](terms, N);
        double max_error = calculate_max_error(terms, N, sums[i], exact_value);

        printf("  %s суммирование:\n", sum_names[i]);
        printf("    Конечный результат: %.12f\n", res);
        printf("    Максимальная погрешность на частичных суммах: %.2e\n", max_error);
        printf("\n");
    }
}

int main() {
    setlocale(LC_ALL, "Russian");
    double x;

    sum_func sums[] = { sum_direct, sum_reverse, sum_kahan };
    char* sum_names[] = { "Прямое", "Обратное", "Кэхан" };

    printf("Введите x: ");
    if (scanf("%lf", &x) != 1) {
        printf("Ошибка ввода!\n");
        return 1;
    }

    printf("\n========================================\n");
    printf("Результаты при x = %.4f\n", x);
    printf("========================================\n\n");

    calculate("exp(x)", term_exp, x, sums, sum_names);
    calculate("sin(x)", term_sin, x, sums, sum_names);
    calculate("cos(x)", term_cos, x, sums, sum_names);

    if (x > -1) {
        calculate("ln(1+x)", term_ln, x, sums, sum_names);
    }
    else {
        printf("ln(1+x):\n");
        printf("  Внимание: ln(1+x) не определен при x <= -1\n");
        printf("  Пропускаем вычисление\n\n");
    }

  

    return 0;
}
