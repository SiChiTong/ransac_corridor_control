#include <cmath>
#include <vector>
#include <time.h>
#include <ros/ros.h>

#define PI 3.14159265

/* LineTracking
 *     Fun��o de controle para seguir uma trajet�ria dada por uma reta
 * Entradas: 
 *    line               : vetor contendo os pontos que definem a trajet�ria (x1, x2, y1, y2)
 *    v_linear           : velocidade linear do ve�culo
 *    v_angular          : velocidade angular do veiculo
 *    dt                 : intervalo de tempo desde a ultima chamada da funcao
 *    KPT, KIT, KRT, KVT : ganhos do controlador
 *
 * Sa�da:
 *    velocidade angular em rad/s
 */



class Control{
private:
    static double errori;
public:
    static double LineTracking(const std::vector<double> &line,
                               const double &v_linear, const double &v_angular,
                               const double &dt,
                               const double &KPT, const double &KIT, const double &KRT, const double &KVT);
};

