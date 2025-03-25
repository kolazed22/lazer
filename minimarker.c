#include "main.h"

#define Ntr 24       //количество потоков
#define pi 3.14159
#define Nx1 102     //число узлов по оси х локальной сетки
#define Ny 64       //число узлов по оси у
#define Nz 70       //чило узлов по оси z
#define Nx2 200     //число узлов по оси х глобальной сетки

/*функция для вычисления теплопроводности*/
double hcond(double T, int n_m)
{
    if(n_m == 0)  //титан BT6
    {
        double ks,
        Tm = 1944;
        ks = 6 + 0.015 * (T - 300);
        if (T >= Tm)
            ks = 6 + 0.015 * (Tm - 300);
        return ks;
    }
    if (n_m == 1)  //титан BT1
    {
        double ks, Tm = 1944;
        ks = 22.5 + 8e-6 * pow(T - 650, 2);
        if (T > Tm)
            ks = 22.5 + 8e-6 * pow(T - 650, 2);
        return ks;
    }
    
}
/*функция для вычисления теплоемкости*/
double hcap(double T, int n_m)
{
    if (n_m == 0) //титан BT6
    {
        double Cs,
            Tm = 1944, //температура плавления
            Tb = 3560, //температура кипения
            Lm = 1.43e9, //удельная теплота плавления Дж/м^3
            pos = 4500,  //плотность металла, кг/м^3
            po = 4110, //плотнотсь расплава, кг/м^3
            dTm = 60;
        Cs = (600 + 1.07e-5 * pow(fabs(T - 500), 2.5)) * pos;
        if (T > Tm && T <= Tb)
            Cs = 950 * po;
        if (T > Tb)
            Cs = 700 * po;
        Cs = Cs + Lm / (dTm * sqrt(2 * pi)) * exp(-pow(T - Tm, 2) / (2 * pow(dTm, 2)));
        return Cs;
    }
    if (n_m == 1) //титан BT1
    {
        double Cs,
               Tm = 1944,
               po = 4300,
               Lm = 290e3 * po,
               dTm = 40;
               if (T >= Tm)
                Cs = 840 * po;
        Cs = (-1.1e-4 * pow(T - 2000, 2) + 840) * po + Lm / (dTm * sqrt(pi)) * exp(-pow(T - Tm, 2) / pow(dTm, 2));
        return Cs;
    }
}
/*функция для вычисления приращения тепловой энергии*/ //не нужна для основного расчета
double int_energy(double T)
{
    double Eint,
           Tm = 1933,
           po = 4300,
           Lm = 290e3*po,
           dTm = 40,
           T0 = 291;
    Eint = (3.667e-5 * (pow(2000 - T, 3) - pow(2000 - T0, 3)) + 840 * (T - T0))*po + Lm / 2 * erfc((Tm - T) / dTm);
    //Eint = 2.5e6 * (T - T0);
    return Eint;
}
int minimarker(LaserParams params, Material *material,  GtkWidget *progress_bar)
{
    static double T1[Nx1][Ny][Nz], T2[Nx2][Ny][Nz], H1[Nx1][Ny], H2[Nx2][Ny];  //массивы для температуры на локальной и глобальной сетках
    static double ATz1[Nz], BTz1[Nz], ATy1[Ny], BTy1[Ny], ATx1[Nx1], BTx1[Nx1];
    static double ATx2[Nx2], BTx2[Nx2], ATy2[Ny], BTy2[Ny], ATz2[Nz], BTz2[Nz];
    static double q0[Nx1][Ny]; //массив для плотности мощности
    static double z[Nz], y[Ny];  //узлы сетки по осям у и z
    static double sum_z[Nx2][Ny], sum_zy[Nx2];
    double a, b, c, d, g, JT, Jn, Tav, Esum, Eabs=0;
    double T0 = 290,            //начальная температура
           t0,                  //длительность переднего фронта импульса
           tp = params.tp,      // длительнотсь импульса 
           r0 = params.r0,      //радиус пучка по уровню e^-1
           A = material->A,     //поглощательная способность
           m0 = material->m0,   //масса атома металла, кг 
           Tb = material->Tb,   //температура кипения, K
           pol = material->pol, //плотнотсь расплава
           pa = 1e5,            //атмосферное давление, Па
           Lev = material->Lev, // удельная теплота испарения
           kb = 1.3806505e-23,  // постоянная Больцмана
           P = params.P,        //средняя мощность
           Q0,                  //плотность энергии в центре пучка
           f = params.f,        //частота следования импульсов
           v = params.v,        //скорость сканирования
           cs,
           uev,
           ks0,
           ks1;
    double dx1, dx2; //шаги по сетки по координате х
    double dt10, dt1, dt2;
    double time = 0, time1, time2, timestep = 10e-9, counttime;
    double delta = 0, Tmax = 0;
    FILE * f1, * f2, * f3, * f4, * f5, * f6, *f7;
    fopen_s(&f1, g_build_filename(DIR_TEMP,"out_Txy.txt", NULL), "w"); //распределение температуры в плоскости xy  
    fopen_s(&f2, g_build_filename(DIR_TEMP,"out_Tz.txt", NULL), "w");  //зависимость температуры от глубины в центре трека в момент времени достижения максимальной температуры
    fopen_s(&f3, g_build_filename(DIR_TEMP,"out_Tt.txt", NULL), "w");  // зависимость темпеарутры поверхности для точки в центре трека от времени
    fopen_s(&f4, g_build_filename(DIR_TEMP,"out_test.txt", NULL), "w"); 
    fopen_s(&f5, g_build_filename(DIR_TEMP,"out_Txz.txt", NULL), "w");  //распределение температуры в плоскости xz  
    fopen_s(&f6, g_build_filename(DIR_TEMP,"out_Tyz.txt", NULL), "w");  //распределение температуры в плоскости yz
    fopen_s(&f7, g_build_filename(DIR_TEMP,"out_Ty.txt", NULL), "w");  //поперечные профиль температуры
    int i, j, k, t, tmax1, tmax2, np, Np, np0, n_m;
    int i0, ind, i00 = 30;
    int ni;
    /*ввод скорости, частоты и средней мощности с клавиатуры*/
    printf("Choose material (0 - titanuim (BT6), 1 - titanium (BT1)):");
    scanf_s("%i", &n_m);
    // printf("Input tp (ns):");
    // scanf_s("%lf", &tp);
    // printf("Input r0 (um):");
    // scanf_s("%lf", &r0);
    // printf("Input v (mm/s):");
    // scanf_s("%lf", &v);
    // printf("Input f (kHz):");
    // scanf_s("%lf", &f);
    // printf("Input P (W):");
    // scanf_s("%lf", &P);
    // if (n_m == 0)
    // {
    //     // printf("\n\nTitanium BT6\n");
    //     A = 0.4;
    //     m0 = 7.8e-26;
    //     Tb = 3560;
    //     Lev = 4e10;
    //     pol = 4100;
    // }
    // if (n_m == 1)
    // {
    //     // printf("\n\nTitanium BT1\n");
    //     A = 0.38;
    //     m0 = 7.8e-26;
    //     Tb = 3560;
    //     Lev = 4e10;
    //     pol = 4100;
    // }
    // printf("P=%.2f W\nv=%.2f mm/s\nf=%.2f kHz\nr0=%.2f um\ntp=%.3f ns\n\n", P, v, f, r0, tp);
    clock_t timeprog;
    timeprog = clock();
    // printf("executing...  %.2f%%\r", 0);
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), 0.0);
    r0 = r0 * 1e-6;
    t0 = tp / 2.5 * 1e-9;
    f = f * 1e3;
    v = v / 1e3;
    i0 = i00;
    dt10 = 6*t0/40;
    dx1 = r0 / 25;
    dx2 = 3 * dx1;
    Np = 5 * 2 * 1.33 * r0 / v * f;
    Q0 = (P / f) / (pi * pow(r0, 2));
    np0 = (Nx2 / 2 - i0) * dx2 / (v / f);
    int Nzmax1 = 35;
    int Nymax1 = 40;
    int Nxmax2 = i00 + 3 * (int)floor(r0 / dx2);
    int Nymax2 = 40;
    int Nzmax2 = 35;
    /*генерация узлов сетки*/
    y[0] = 0;
    for (j = 0; j < Ny - 1; j++)
    {
        y[j + 1] = y[j] + (30e-9 + pow(j + 1, 2.5) * 0.2e-4 * r0);
        if (y[j + 1]-y[j] > 40e-6)
            y[j + 1] = y[j] + 40e-6;
    }
    z[0] = 0;
    for (k = 0; k < Nz - 1; k++)
    {
        z[k + 1] = z[k] + (30e-9 + pow(k + 1, 2.5) * 0.2e-4 * r0);
        if (z[k + 1]-z[k] >= 40e-6)
            z[k + 1] = z[k] + 40e-6;
    }
    /*задание начальных условий*/
    for (k = 0; k < Nz; k++)
        for (j = 0; j < Ny; j++)
        {
            for (i = 0; i < Nx1; i++)
              T1[i][j][k] = T0;
            for (i = 0; i < Nx2; i++)
              T2[i][j][k] = T0;
        }

   /*цикл по количенству импульсов*/
   for (np = 0; np < Np; np++)
    {
        time1 = 0;
        time2 = 0;
        ind = 1;
        counttime = 0;
        dt1 = dt10;

        /*расчет температуры в течение импульса*/
        while (time1 < 6 * t0)
        {
#pragma omp parallel for shared(T1,Eabs) private(i,j,k,ATz1,BTz1,ks0,ks1,cs,a,b,c,d,g,JT) schedule (dynamic) num_threads(Ntr)
            for (i = 0; i < Nx1; i++)
                for (j = 0; j <= Nymax1; j++)
                {
                   q0[i][j] = A * Q0 / pow(t0, 2) * time1 * exp(-time1 / t0) * exp(-(pow((i - Nx1 / 2) * dx1 - delta, 2) + pow(y[j], 2)) / pow(r0, 2));
                   uev= 0.82 / pol * sqrt(m0 / (2 * pi * kb * T1[i][j][0])) * pa * exp(Lev * m0 / (kb * Tb * pol) * (1 - Tb / T1[i][j][0]));
                   JT = (q0[i][j] - Lev*uev) / (z[1] - z[0]);
                   ks1 = hcond(T1[i][j][0],n_m);
                   cs = hcap(T1[i][j][0],n_m);
                   b = -(ks1 / pow(z[1] - z[0], 2) + cs / dt1);
                   c = ks1 / pow(z[1] - z[0], 2);
                   d = -(T1[i][j][0] * cs / dt1 + JT);
                   ATz1[0] = -c / b;
                   BTz1[0] = d / b;
                   for (k = 1; k < Nzmax1; k++)
                    {
                        JT = 0;
                        Jn = 0;
                        ks0 = hcond(T1[i][j][k], n_m);
                        ks1 = hcond(T1[i][j][k + 1], n_m);
                        cs = hcap(T1[i][j][k], n_m);
                        a = ks0 / pow(z[k] - z[k - 1], 2);
                        b = -(cs / dt1 + ks0 / pow(z[k] - z[k - 1], 2) + ks1 / ((z[k + 1] - z[k]) * (z[k] - z[k - 1])));
                        c = ks1 / ((z[k + 1] - z[k]) * (z[k] - z[k - 1]));
                        d = -(T1[i][j][k] * cs / dt1 + JT);
                        g = a * ATz1[k - 1] + b;
                        ATz1[k] = -c / g;
                        BTz1[k] = (d - a * BTz1[k - 1]) / g;
                    }
                    ks1= hcond(T1[i][j][Nzmax1-1], n_m);
                    cs= hcap(T1[i][j][Nzmax1-1], n_m);
                    a = ks1 / pow(z[Nzmax1 - 1] - z[Nzmax1 - 2], 2);
                    b = -ks1 / pow(z[Nzmax1 - 1] - z[Nzmax1 - 2], 2) - cs / dt1;
                    d = -cs / dt1*T1[i][j][Nzmax1-1];
                    T1[i][j][Nzmax1 - 1] = (d - a * BTz1[Nzmax1 - 2]) / (b + a * ATz1[Nzmax1 - 2]);
                    for (k = Nzmax1 - 2; k >= 0; k--)
                      T1[i][j][k] = ATz1[k] * T1[i][j][k + 1] + BTz1[k];
                }
            if (r0<=10*sqrt(2.5*t0*hcond(T0, n_m)/hcap(T0, n_m))) //уловие расчета теплообмена в радиальных направлениях в течение импульса
            {
                for (i = 0; i < Nx1; i++)
                    for (k = 0; k < Nzmax1; k++)
                    {
                        JT = 0;
                        Jn = 0;
                        ks0 = hcond(T1[i][0][k], n_m);
                        cs = hcap(T1[i][0][k], n_m);
                        b = -(ks0 / pow(y[1] - y[0], 2) + cs / dt1);
                        c = ks0 / pow(y[1] - y[0], 2);
                        d = -(T1[i][0][k] * cs / dt1 + JT);
                        ATy1[0] = -c / b;
                        BTy1[0] = d / b;
                        for (j = 1; j < Nymax1; j++)
                        {
                            JT = 0;
                            Jn = 0;
                            ks0 = hcond(T1[i][j][k], n_m);
                            ks1 = hcond(T1[i][j + 1][k], n_m);
                            cs = hcap(T1[i][j][k], n_m);
                            a = ks0 / pow(y[j] - y[j - 1], 2);
                            b = -(cs / dt1 + ks0 / pow(y[j] - y[j - 1], 2) + ks1 / ((y[j + 1] - y[j]) * (y[j] - y[j - 1])));
                            c = ks1 / ((y[j + 1] - y[j]) * (y[j] - y[j - 1]));
                            d = -(T1[i][j][k] * cs / dt1 + JT);
                            g = a * ATy1[j - 1] + b;
                            ATy1[j] = -c / g;
                            BTy1[j] = (d - a * BTy1[j - 1]) / g;
                        }
                        cs = hcap(T1[i][Nymax1][k], n_m);
                        ks0 = hcond(T1[i][Nymax1][k], n_m);
                        JT = 0;
                        Jn = 0;
                        a = ks0 / pow(y[Nymax1] - y[Nymax1 - 1], 2);
                        b = -(cs / dt1 + ks0 / pow(y[Nymax1] - y[Nymax1 - 1], 2));
                        d = -(T1[i][Nymax1][k] * cs / dt1 + JT);
                        T1[i][Nymax1][k] = (d - a * BTy1[Nymax1 - 1]) / (b + a * ATy1[Nymax1 - 1]);
                        for (j = Nymax1 - 1; j >= 0; j--)
                            T1[i][j][k] = ATy1[j] * T1[i][j + 1][k] + BTy1[j];
                    }
                for (k = 0; k < Nzmax1; k++)
                    for (j = 0; j <= Nymax1; j++)
                    {
                        JT = 0;
                        ks0 = hcond(T1[0][j][k], n_m);
                        cs = hcap(T1[0][j][k], n_m);
                        b = -(ks0 / pow(dx1, 2) + cs / dt1);
                        c = ks0 / pow(dx1, 2);
                        d = -(T1[0][j][k] * cs / dt1 + JT);
                        ATx1[0] = -c / b;
                        BTx1[0] = d / b;
                        for (i = 1; i < Nx1 - 1; i++)
                        {
                            JT = 0;
                            Jn = 0;
                            ks0 = hcond(T1[i][j][k], n_m);
                            ks1 = hcond(T1[i + 1][j][k], n_m);
                            cs = hcap(T1[i][j][k], n_m);
                            a = ks0 / pow(dx1, 2);
                            b = -(cs / dt1 + (ks0 + ks1) / pow(dx1, 2));
                            c = ks1 / pow(dx1, 2);
                            d = -(T1[i][j][k] * cs / dt1 + JT);
                            g = a * ATx1[i - 1] + b;
                            ATx1[i] = -c / g;
                            BTx1[i] = (d - a * BTx1[i - 1]) / g;
                        }
                        cs = hcap(T1[Nx1 - 1][j][k], n_m);
                        ks0 = hcond(T1[Nx1 - 1][j][k], n_m);
                        JT = 0;
                        a = ks0 / pow(dx1, 2);
                        b = -(cs / dt1 + ks0 / pow(dx1, 2));
                        d = -(T1[Nx1 - 1][j][k] * cs / dt1 + JT);
                        T1[Nx1 - 1][j][k] = (d - a * BTx1[Nx1 - 2]) / (b + a * ATx1[Nx1 - 2]);
                        for (i = Nx1 - 2; i >= 0; i--)
                            T1[i][j][k] = ATx1[i] * T1[i + 1][j][k] + BTx1[i];
                    }
            }
            time1 = time1 + dt1;
            time = time + dt1;

            /*вывод данных в файл для зависиомсти температуры точки от времени*/
            if ((i0 * dx2 + Nx1 / 2 * dx1 <= Nx2 / 2 * dx2) || (i0 * dx2 - Nx1 / 2 * dx1 >= Nx2 / 2 * dx2))
            {
                fprintf(f3, "%e  %.3f\n", time, T2[Nx2 / 2][0][0]);
                counttime = counttime + dt1;
            }
            else
            {
                fprintf(f3, "%e  %.3f\n", time, T1[Nx1 / 2 + 3 * (Nx2 / 2 - i0)][0][0]);
                counttime = counttime + dt1;
            }


            /*определение момента времни достижения максимальной температуры в исследуемой точке и вывод данных в файлы*/
            if (np == np0)
                if (time1 > t0)
                    if (ind == 1)
                        if (T1[Nx1 / 2][0][0] < Tmax)
                        {
                            for (i = 0; i < Nx1; i++)
                            {
                                for (j = 0; j < Ny; j++)
                                {
                                    if (i == Nx1 / 2)
                                        fprintf(f7, "%e  %.3f\n", y[j], T1[i][j][0]);
                                    fprintf(f1, "%e   %e   %.3f\n", ((i0 * dx2 - Nx1 / 2 * dx1) + i * dx1), y[j], T1[i][j][0]);
                                    if (j != 0)
                                        fprintf(f1, "%e   %e   %.3f\n", ((i0 * dx2 - Nx1 / 2 * dx1) + i * dx1), -y[j], T1[i][j][0]);
                                }
                                for (k = 0; k < Nz; k++)
                                    fprintf(f5, "%e   %e   %.3f\n", ((i0 * dx2 - Nx1 / 2 * dx1) + i * dx1), z[k], T1[i][0][k]);
                                
                            }
                            for (i = 0; i < Nx2; i++)
                            {
                                for (j = 0; j < Ny; j++)
                                {
                                    if (i * dx2<(i0 * dx2 - Nx1 / 2 * dx1) || i * dx2>(i0 * dx2 + Nx1 / 2 * dx1))
                                    {
                                        fprintf(f1, "%e   %e   %.3f\n", i * dx2, y[j], T2[i][j][0]);
                                        if (j != 0)
                                            fprintf(f1, "%e   %e   %.3f\n", i * dx2, -y[j], T2[i][j][0]);
                                    }
                                }

                                for (k = 0; k < Nz; k++)
                                    if (i * dx2<(i0 * dx2 - Nx1 / 2 * dx1) || i * dx2>(i0 * dx2 + Nx1 / 2 * dx1))
                                     fprintf(f5, "%e   %e   %.3f\n", i * dx2, z[k], T2[i][0][k]);

                            }
                            for(k=0;k<Nz;k++)
                                for (j = 0; j < Ny; j++)
                                {
                                    fprintf(f6, "%e   %e   %.3f\n", y[j], z[k], T1[Nx1/2][j][k]);
                                    if (j != 0)
                                        fprintf(f6, "%e   %e   %.3f\n", -y[j], z[k], T1[Nx1/2][j][k]);
                                }
                                
                            
                                ind = 0;
                        }
         Tmax = T1[Nx1 / 2][0][0];
        }
        

        /*перерасчет температуры на глобальной сетке*/
        for (j = 0; j < Ny; j++)
            for (k = 0; k < Nz; k++)
                for (i = i0; i < Nx2; i++)
                {
                    if (3 * (i - i0) < Nx1/2)
                    {
                        T2[i][j][k] = T1[Nx1 / 2 + 3 * (i - i0)][j][k];
                        T2[2 * i0 - 1 - i][j][k] = T1[Nx1 / 2 - 3 * (i + 1 - i0)][j][k];
                    }

                }
        dt2 = dt1;

        /*расчет на стадии остывания между импульсами*/
        while (time2 < (1 / f - time1))
        {
           
            if (dt2 < 0.03 / f)
                dt2 = dt2 * 1.4;
            else
                dt2 = 0.03 / f;
#pragma omp parallel for shared(T2) private(i,j,k,ATz2,BTz2,ks0,ks1,cs,a,b,c,d,g,JT) schedule (dynamic) num_threads(Ntr)
            for (i = 0; i < Nxmax2; i++)
                for (j = 0; j < Nymax2; j++)
                {
                    JT = 0;
                    ATz2[0] = 1;
                    BTz2[0] = 0;
                    for (k = 1; k < Nzmax2 - 1; k++)
                    {
                        JT = 0;
                        ks0 = hcond(T2[i][j][k], n_m);
                        ks1 = hcond(T2[i][j][k + 1], n_m);
                        cs = hcap(T2[i][j][k], n_m);
                        a = ks0 / pow(z[k] - z[k - 1], 2);
                        b = -(cs / dt2 + ks0 / pow(z[k] - z[k - 1], 2) + ks1 / ((z[k + 1] - z[k]) * (z[k] - z[k - 1])));
                        c = ks1 / ((z[k + 1] - z[k]) * (z[k] - z[k - 1]));
                        d = -(T2[i][j][k] * cs / dt2 + JT);
                        g = a * ATz2[k - 1] + b;
                        ATz2[k] = -c / g;
                        BTz2[k] = (d - a * BTz2[k - 1]) / g;
                    }
                    ks1 = hcond(T2[i][j][Nzmax2 - 1], n_m);
                    cs = hcap(T2[i][j][Nzmax2 - 1], n_m);
                    a = ks1 / pow(z[Nzmax2 - 1] - z[Nzmax2 - 2], 2);
                    b = -ks1 / pow(z[Nzmax2 - 1] - z[Nzmax2 - 2], 2) - cs / dt2;
                    d = -cs / dt2 * T2[i][j][Nzmax2 - 1];
                    T2[i][j][Nzmax2 - 1] = (d - a * BTz2[Nzmax2 - 2]) / (b + a * ATz2[Nzmax2 - 2]);
                    for (k = Nzmax2 - 2; k >= 0; k--)
                      T2[i][j][k] = ATz2[k] * T2[i][j][k + 1] + BTz2[k];
                }
#pragma omp parallel for shared(T2) private(i,j,k,ATy2,BTy2,ks0,ks1,cs,a,b,c,d,g,JT) schedule (dynamic) num_threads(Ntr)
                for (i = 0; i < Nxmax2; i++)
                 for (k = 0; k < Nzmax2; k++)
                 {
                    JT = 0;
                    Jn = 0;
                    ATy2[0] = 1;
                    BTy2[0] = 0;
                    for (j = 1; j < Nymax2 - 1; j++)
                    {
                        JT = 0;
                        Jn = 0;
                        ks0 = hcond(T2[i][j][k], n_m);
                        ks1 = hcond(T2[i][j + 1][k], n_m);
                        cs = hcap(T2[i][j][k], n_m);
                        a = ks0 / pow(y[j] - y[j - 1], 2);
                        b = -(cs / dt2 + ks0 / pow(y[j] - y[j - 1], 2) + ks1 / ((y[j + 1] - y[j]) * (y[j] - y[j - 1])));
                        c = ks1 / ((y[j + 1] - y[j]) * (y[j] - y[j - 1]));
                        d = -(T2[i][j][k] * cs / dt2 + JT);
                        g = a * ATy2[j - 1] + b;
                        ATy2[j] = -c / g;
                        BTy2[j] = (d - a * BTy2[j - 1]) / g;
                      
                    }
                    ks1 = hcond(T2[i][Nymax2 - 1][k], n_m);
                    cs = hcap(T2[i][Nymax2 - 1][k], n_m);
                    a = ks1 / pow(y[Nymax2 - 1] - y[Nymax2 - 2], 2);
                    b = -ks1 / pow(y[Nymax2 - 1] - y[Nymax2 - 2], 2) - cs / dt2;
                    d = -cs / dt2 * T2[i][Nymax2 - 1][k];
                    T2[i][Nymax2 - 1][k] = (d - a * BTy2[Nymax2 - 2]) / (b + a * ATy2[Nymax2 - 2]);
                    for (j = Nymax2 - 2; j >= 0; j--)
                     T2[i][j][k] = ATy2[j] * T2[i][j + 1][k] + BTy2[j];
                      
                 }
#pragma omp parallel for shared(T2) private(i,j,k,ATx2,BTx2,ks0,ks1,cs,a,b,c,d,g,JT) schedule (dynamic) num_threads(Ntr)
            for (j = 0; j < Nymax2; j++)
                for (k = 0; k < Nzmax2; k++)
                {
                    JT = 0;
                    ATx2[0] = 1;
                    BTx2[0] = 0;
                    for (i = 1; i < Nxmax2 - 1; i++)
                    {
                        JT = 0;
                       
                        ks0 = hcond(T2[i][j][k], n_m);
                        ks1 = hcond(T2[i + 1][j][k], n_m);
                        cs = hcap(T2[i][j][k], n_m);
                        a = ks0 / pow(dx2, 2);
                        b = -(cs / dt2 + (ks0 + ks1) / pow(dx2, 2));
                        c = ks1 / pow(dx2, 2);
                        d = -(T2[i][j][k] * cs / dt2 + JT);
                        g = a * ATx2[i - 1] + b;
                        ATx2[i] = -c / g;
                        BTx2[i] = (d - a * BTx2[i - 1]) / g;
                      
                    }
                    ks1 = hcond(T2[Nxmax2 - 1][j][k], n_m);
                    cs = hcap(T2[Nxmax2 - 1][j][k], n_m);
                    a = ks1 / pow(dx2, 2);
                    b = -ks1 / pow(dx2, 2) - cs / dt2;
                    d = -cs / dt2 * T2[Nxmax2 - 1][j][k];
                    T2[Nxmax2 - 1][j][k] = (d - a * BTx2[Nxmax2 - 2]) / (b + a * ATx2[Nxmax2 - 2]);
                    for (i = Nxmax2 - 2; i >= 0; i--)
                      T2[i][j][k] = ATx2[i] * T2[i + 1][j][k] + BTx2[i];
                }
            time2 = time2 + dt2;
            time = time + dt2;
            fprintf(f3, "%e  %.3f  %e  %e\n", time, T2[Nx2 / 2][0][0]);
            
          /*проверка сохранения энергии*/
           /*Esum = 0;
             for(i=0;i<Nx2-1;i++)
                for(j=0;j<Ny-1;j++)
                    for (k = 0; k < Nz - 1; k++)
                    {
                        Tav = (T2[i][j][k] + T2[i][j][k + 1] + T2[i][j + 1][k] + T2[i][j + 1][k + 1] + T2[i + 1][j][k] + T2[i + 1][j][k + 1] + T2[i + 1][j + 1][k] + T2[i + 1][j + 1][k + 1])/8;
                        Esum = Esum + int_energy(Tav) * dx2 * (y[j + 1] - y[j]) * (z[k + 1] - z[k]);
                    }*/
           // printf("%e  %e  %e  %e\n",time, A * P / f * (np + 1), 2 * Esum, 2*Eabs);
           //fprintf(f4, "%e  %e  %e  %e  %e  %e\n", time, A* P / f * (np + 1), 2 * Esum,fabs(2 * Esum - A * P / f * (np + 1)) / (A * P / f * (np + 1)));
           if (T2[Nxmax2 - 1][0][0] > T0 + 1)
                Nxmax2 = Nxmax2 + 1;
            if (T2[i00][Nymax2 - 1][0] > T0 + 1)
                Nymax2 = Nymax2 + 1;
            if (T2[i00][0][Nzmax2-1] > T0 + 1)
                Nzmax2 = Nzmax2 + 1;
            if (Nxmax2 > Nx2)
                Nxmax2 = Nx2;
            if (Nymax2 > Ny)
                Nymax2 = Ny;
            if (Nzmax2 > Nz)
                Nzmax2 = Nz;
        }
        i0 = i00 + (np + 1) * v / f / dx2;
        delta = i00 * dx2 + (np + 1) * v / f - i0 * dx2;
        if ((i0 * dx2 + 3 * r0) > Nxmax2 * dx2)
            Nxmax2 = i0 + 3 * (int)floor(r0 / dx2);
        if (Nxmax2 > Nx2)
            Nxmax2 = Nx2;

        /*перерасчет температуры на локальную сетку*/
        for (i = Nx1 / 2; i < Nx1; i++)

            for (j = 0; j < Ny; j++)
                for (k = 0; k < Nz; k++)
                {
                    ni = i0 + (i - Nx1 / 2) / 3;
                    T1[i][j][k] = T2[ni][j][k] + (dx1 * (i - Nx1 / 2) - (ni - i0) * dx2) * (T2[ni + 1][j][k] - T2[ni][j][k]) / dx2;
                    ni = i0 - (i - Nx1 / 2) / 3 - 1;
                    T1[Nx1 - i - 1][j][k] = T2[ni][j][k] + ((i0 - ni) * dx2 - (i - Nx1 / 2 + 1) * dx1) * (T2[ni + 1][j][k] - T2[ni][j][k]) / dx2;
                }
        // printf("executing...  %.2f%%\r", 100 * (double)(np + 1) / Np);
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), (double)(np + 1) / Np);

    }
    fclose(f1);
    fclose(f2);
    fclose(f3);
    fclose(f4);
    fclose(f5);
    fclose(f6);
    fclose(f7);
    timeprog = clock() - timeprog;
    update_plots(NULL, NULL);
    // printf("\nProgram execution time=%f s\n", (double)timeprog / CLOCKS_PER_SEC);
    // system("PAUSE");
    return 0;

}

