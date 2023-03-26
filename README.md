#  Лабораторная работа 1. Изучение SIMD(sse+avx) расширения архитектуры x86-64

## Цель
Изучить возможности оптимизации кода с использовванием intrinsic функций и векторной обработки, применить их для отрисовки множества  Мандельброта и наложения изображений, используя alpha blending.

## В работе используются
Язык программирования C\C++, набор  компиляторов GCC

## Экспериментальная установка: 

# Ход работы

### Часть первая. Множество Мандельброта.
Множество Мандельброта — множество точек c на комплексной плоскости, для которых рекуррентное соотношение z -> z^2 + c, при z = 0 задаёт ограниченную последовательность. 
Порядок определения, принадлежит ли точка z множеству (традиционно закрашиваемого чёрным цветом) или нет (закрашивается цветом, зависящим от числа итераций) следующий: на каждой итерации вычисляется текущее расстояние — значение модуля |z|, которое затем сравнивается с «критерием бесконечности» (обычно берётся значение, равное 2). 
Алгоритм в своей основе содержит вышеприведенное соотношение и использует комплексную структуру данных
#### Реализация алгоритма (псевдокод)
    struct complex
    {
        Re = 0.0;
        Im = 0.0;
    };


    void mandelbrot (model, max_iterations)
    {
        for (i = 0; i < model->height; i++)
        {
            for (j = 0; j < model->width; j++)
            {
                x = Mandelbrot_init.Re_min + Mandelbrot_init.Re_scalar * j;
                y = Mandelbrot_init.Im_min + Mandelbrot_init.Im_scalar * i;
        
                complex z_0 = {x, y};
                complex z   = {0, 0};

                counter       = 0;
                current_index = 0;

                while (++counter < max_iterations)
                {
                    if (z^2 >= 4)
                        break;
    
                    z = next_number(z, z0);
                }

                current_index = (j + i*model->width)*4;
                get_color (&model->dots[current_index], counter, max_iterations);
            }
        }
    }
Размер вычисляемого множества: 1200х1000

#### Результаты измерений (время усреднено для трех запусков тестирующей программмы)

| Версия      | Флаги оптимизации | Число запусков | Время исполнения (нс) |
| ------      | :------:          | :------:       | ------: |
| Scalar      | нет               | 16             |21861116592 | 
| Vector(sse) | нет               | 16             | 3085235426 |
| Scalar      | -О3               | 512            | 21802432639 |
| Vector(sse) | -О3               | 512            | 60503953584 |
 
Результаты показывают, что без оптимизаций компилятора векторная обработка оказывается в ~7 раз быстрее скалярной. С использованием флага '-O3' векторная обработка оказывается быстрее скалярной всего в ~3 раза, при этом оптимизированная версия векторной обработки оказывается быстрее неоптимизированной в 22,4 раза. Что в совокупности дает прирост в 140 раз по сравнению с неоптимизированной скалярной обработкой

### Часть вторая. Alpha blending.
Испольузется 32-битный (с альфа каналом) формат bmp без сжатия.
Алгоритм использует только целочисленные операции благодаря битовым сдвигам и разделению цвета на две компоненты (красная+синия и зеленая)

#### Реализация алгоритма (псевдокод)
	alpha = foreground >> 24;

	red_blue  = background & 0xFFFF00FF;
	green     = background & 0xFF00FF00;
	red_blue += ((foreground & 0xFFFF00FF - red_blue) * alpha) >> 8;
	green    += ((foreground & 0xFF00FF00 -    green) * alpha) >> 8;

	out = (red_blue & 0xFFFF00FF)  | (green & 0xFF00FF00);

Размер изображения: 512x512

#### Результаты измерений (время усреднено для трех запусков тестирующей программмы)

| Версия      | Флаги оптимизации | Число запусков | Время исполнения (нс) |
| ------      | :------:          | :------:       | ------: |
| Scalar      | нет               | 2048           | 5139526594 | 
| Vector(sse) | нет               | 2048           | 3297735029 |
| Scalar      | -О3               | 2048           | 1317170120 |
| Vector(sse) | -О3               | 2048           |  398703635 |



### Заключение
- В данной работе мы выяснили, что в среднем векторная обработка работает быстрее скалярной от 1,6 до 140 раз (при использовании оптимизации компилятора)
- Малое ускорение при использовании векторной обработки во второй части работы можно объяснить невыской сложностью алгоритма, при этом с флагом '-O3'ускорение (3,2) близко к теоретическому(4)
- Из первой части можно сделать вывод, что наибольшей эффективности программы можно достигнуть, комбинируя использование векторной обработки и оптимизации компилятора. 

