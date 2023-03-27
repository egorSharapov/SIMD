# Лабораторная работа 1. Изучение SIMD(SSE+AVX) расширения архитектуры x86-64

## Цель
Изучить возможности оптимизации кода с использовванием intrinsic функций и векторной обработки, применить их для отрисовки множества  Мандельброта и наложения изображений, используя alpha blending.

## В работе используются
Язык программирования C\C++, набор  компиляторов GCC

## Экспериментальная установка

## Ход работы

### Часть первая. Множество Мандельброта.
Множество Мандельброта — множество точек c на комплексной плоскости, для которых рекуррентное соотношение z -> z^2 + c, при z = 0 задаёт ограниченную последовательность. 
Порядок определения, принадлежит ли точка z множеству (традиционно закрашиваемого чёрным цветом) или нет (закрашивается цветом, зависящим от числа итераций) следующий: на каждой итерации вычисляется текущее расстояние — значение модуля |z|, которое затем сравнивается с «критерием бесконечности» (обычно берётся значение, равное 2). 
Алгоритм в своей основе содержит вышеприведенное соотношение и использует комплексную структуру данных
#### Реализация алгоритма (псевдокод)
```C++
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
    
                z = next_number (z, z0);
            }

            current_index = (j + i*model->width)*4;
            get_color (&model->dots[current_index], counter, max_iterations);
        }
    }
}
```
Размер вычисляемого множества: 1200х1000 пикселей

#### Результаты измерений (время усреднено для трех запусков тестирующей программмы)

| Версия      | Флаги оптимизации | Число запусков | Время исполнения (us) |
| ------      | :---------------: | :------------: | --------------------: |
| Scalar      | нет               | 128            |       175 273 827 884 | 
| Scalar      | -О3               | 512            |        60 503 953 584 | 
| Scalar      | -О3 -fprofile-use | 512            |        59 579 548 759 |
| Scalar      | -О3 -ffast-math   | 512            |        64 843 286 189 |
| Vector(sse) | нет               | 128            |        28 040 940 225 | 
| Vector(sse) | -О3               | 512            |        21 802 432 639 |
| Vector(sse) | -О3 -fprofile-use | 512            |        21 059 302 425 |
| Vector(sse) | -О3 -ffast-math   | 512            |        20 119 211 444 |
#### Графики
<img src="/plots/plot1.png" style="height: 370px;"/>
Результаты показывают, что без оптимизаций компилятора векторная обработка оказывается в ~6.25 раз быстрее скалярной. С использованием флага `-O3` векторная обработка оказывается быстрее скалярной всего в ~3 раза, при этом оптимизированная версия векторной обработки оказывается быстрее неоптимизированной в 22.4 раза. Что в совокупности дает прирост в 140 раз по сравнению с неоптимизированной скалярной обработкой

### Часть вторая. Alpha blending.
Испольузется 32-битный (с альфа каналом) формат bmp без сжатия.
Алгоритм использует только целочисленные операции благодаря битовым сдвигам и разделению цвета на две компоненты (красная+синия и зеленая)

#### Реализация алгоритма (псевдокод)
```C++
alpha = foreground >> 24;
    
red_blue  = background & 0x00FF00FF;
green     = background & 0xFF00FF00;

red_blue += ((foreground & 0x00FF00FF - red_blue) * alpha) >> 8;
green    += ((foreground & 0xFF00FF00 -    green) >> 8) * alpha;
   
out = (red_blue & 0x00FF00FF)  | (green & 0xFF00FF00);
```
Размер изображения: 512x512

#### Результаты измерений (время усреднено для трех запусков тестирующей программмы)

| Версия      | Флаги оптимизации | Число запусков | Время исполнения (us) |
| ------      | :---------------: | :------:       | ------: |
| Scalar      | нет               | 2048           | 3 238 683 753 | 
| Vector(sse) | нет               | 2048           | 2 580 843 021 |
| Vector(avx) | нет               | 2048           | 1 406 541 603 |
| Scalar      | -О3               | 2048           |   802 411 367 |
| Vector(sse) | -О3               | 2048           |   240 692 377 |
| Vector(avx) | -O3               | 2048           |   134 546 940 |
| Scalar      | -О3 -ffast-math   | 2048           |   800 456 005 |
| Vector(sse) | -О3 -ffast-math   | 2048           |   257 381 083 |
| Vector(avx) | -О3 -ffast-math   | 2048           |   119 893 228 |
#### Графики
<img src="/plots/plot2.png" style="height: 370px;"/>
Замеры показывают, что без оптимизаций компилятора векторная обработка (sse) быстрее скалярной всего в 
1.28 раза или в 2.28 раза в случае avx2. При этом с использованием оптимизаций gcc прирост при использовании векторной обработки по сравнению со скалярной составляет 6.7 порядков. Также можно отметить, что не всегда использование агрессивной оптимизации математических выражений позволяет получить прирост скорости работы. Например, в случае sse с опцией `-ffast-math` программа стала медленнее на 6.67%.

### Заключение
- В данной работе мы выяснили, что в среднем векторная обработка работает быстрее скалярной от 1.3 до 140 раз (при использовании оптимизации компилятора).
- Малое ускорение при использовании векторной обработки во второй части работы можно объяснить невыской сложностью алгоритма, при этом с флагами `-O3 -ffast-math` и использованием avx2 инструкци можно добиться ускорения до 31 раза. 
- Из первой части можно сделать вывод, что наибольшей эффективности программы можно достигнуть, комбинируя использование векторной обработки и оптимизации компилятора. 
- Также важно отметить, что не всегда более агрессивные оптимизации компилятора ведут к ускорению работы программы. 


