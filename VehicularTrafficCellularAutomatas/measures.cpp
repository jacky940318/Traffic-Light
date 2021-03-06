#include "measures.h"

float _velocity;
float velocity_total;

float flux;
float flux_total;

int collisions;

int value_intersections;
float value_intersections_total;

float save_velocity;
float save_flux;
float save_value_intersections;

int error_fp = 0;
int error_fn = 0;

int test_error_fp = 0;
int test_error_fn = 0;

FILE *fp_f;
FILE *fp_v;
FILE *fp_i;

FILE *fp_fopt;
FILE *fp_vopt;

float v_optim;
float j_optim;
float j_max = 1.0 / 4.0;

//Measures////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
void Velocity()
{

    int reposition_cell_h;
    int reposition_cell_v;
    int n, m;
    int i, j;

    int cell_changed = 0;

    for (n = 0; n < n_hor_streets; n++){
        for (m = 0; m < m_ver_streets; m++){

            reposition_cell_h = m * d_side_block + m;
            reposition_cell_v = n * d_side_block + n;

            if ((pr_intersections[n][m].value == 0 && pw_intersections[n][m].value == 1))
                cell_changed++;

            for (i = 1; i <= d_side_block; i++){
                reposition_cell_h = m * d_side_block + m + i;
                if (pr_horizontal_streets[n][reposition_cell_h].value == 0 && pw_horizontal_streets[n][reposition_cell_h].value == 1)
                    cell_changed++;
            }

            for (j = 1; j <= d_side_block; j++){
                reposition_cell_v = n * d_side_block + n + j;
                if (pr_vertical_streets[m][reposition_cell_v].value == 0 && pw_vertical_streets[m][reposition_cell_v].value == 1)
                    cell_changed++;
            }
        }
    }

    if (size_vehicles > 0)
      _velocity = ((float) cell_changed / size_vehicles);
    else
      _velocity = 0;


    qDebug() << "------------------------------";

    qDebug() << "HT:" << _velocity << size_vehicles << cell_changed;
    velocity_total+= _velocity;



    //int i;
    int v_total;

    v_total = 0;
    for (i = 0; i < size_vehicles; i++) {
        v_total+= pw_vehicles[i].speed;// GetVelocityVehicle(i);
    }

    //qDebug() << "Measure Size vehicles: " << size_vehicles;
    if (size_vehicles > 0){
      _velocity = ((float) v_total / size_vehicles);
    //  qDebug() << "Velocity: " << velocity;
    }
    else
      _velocity = 0;

    qDebug() << "HV:" <<  _velocity << size_vehicles << v_total;
    qDebug() << "------------------------------";


}
*/

void Velocity()
{
    int i;
    int v_total;

    v_total = 0;
    for (i = 0; i < size_vehicles; i++) {
        v_total+= GetVelocityVehicle(i);
    }

    //qDebug() << "Measure Size vehicles: " << size_vehicles;
    if (size_vehicles > 0){
      _velocity = ((float) v_total / size_vehicles);
    //  qDebug() << "Velocity: " << velocity;
    }
    else
      _velocity = 0;

    //qDebug() <<  _velocity << size_vehicles;
    velocity_total+= _velocity;
}

void Flux(float density)
{
   // qDebug() << "Flujo" << ((float) density * velocity);
    flux = ((float) density * _velocity);
    //qDebug() <<  flux << density;
    flux_total+= flux;
}


void averageValueIntersections()//Probando por que hay un mayor desempenio en el diagrama fundamental respecto al optimo
{
    value_intersections = 0;
    for (int n = 0; n < n_hor_streets; n++)
        for (int m = 0; m < m_ver_streets; m++)
            value_intersections+= pr_intersections[n][m].value;
    value_intersections_total+= value_intersections;
}

void CalculateSaveMeasures()
{
    save_velocity+= (velocity_total / n_ticks);
    save_flux+= (flux_total / n_ticks);
    save_value_intersections+= (value_intersections_total / n_ticks);
}

void SaveMeasures(float density)
{
    save_velocity = save_velocity / n_exp;
    save_flux = save_flux / n_exp;
    save_value_intersections = save_value_intersections / n_exp;

    //qDebug() << n_exp << density;
    v_optim = vOptima(density);
    j_optim = jOptima(density);

    fprintf(fp_v, "%f,%f\n", density, save_velocity);
    fprintf(fp_f, "%f,%f\n", density, save_flux);//con estos datos se grafica el diagrama fundamental del trafico

}



void SaveMeasuresRules(float density_h, float density_v, int rules, FILE *fv, FILE *ff)
{
    save_velocity = save_velocity / n_exp;
    save_flux = save_flux / n_exp;

    //qDebug() << n_exp;
    fprintf(fv, "%f,%f,%f,%d\n", density_h, density_v, save_velocity, rules);//con estos datos se grafica el diagrama fundamental del trafico
    fprintf(ff, "%f,%f,%f,%d\n", density_h, density_v, save_flux, rules);//con estos datos se grafica el diagrama fundamental del trafico


}

float vOptima(float density)
{

    //Es mejor graficar desde R graphics porque se pierden puntos criticos
    if (density <= j_max)
        return 1.0;
    else if (density >= (1.0 - j_max))
        return (1.0 - density) / density;
    else if (density > j_max && density < (1.0 - j_max))
        return j_max / density;

    return 0.0;
}

float jOptima(float density)
{
    //Es mejor graficar desde R graphics porque se pierden puntos criticos
    if (density <= j_max)
        return density;
    else if (density >= (1.0 - j_max))
        return (1.0 - density);
    else if (density > j_max && density < (1.0 - j_max))
        return j_max;

    return 0.0;
}

void SaveMeasuresOpt()
{
    float v_optim, j_optim;

    fp_fopt = fopen("fluopt.csv", "w");   // Abrir archivo para escritura
    fp_vopt = fopen("velopt.csv", "w");   // Abrir archivo para escritura

    fprintf(fp_fopt, "%s,%s\n", "Densidad", "Flujo");
    fprintf(fp_vopt, "%s,%s\n", "Densidad", "Velocidad");

    for (float density = 0.0; density <= 1.0; density+=0.01){

        v_optim = vOptima(density);
        j_optim = jOptima(density);

        fprintf(fp_vopt, "%f,%f\n", density, v_optim);
        fprintf(fp_fopt, "%f,%f\n", density, j_optim);
    }


    fclose(fp_fopt);
    fclose(fp_vopt);

}







