#include <algorithm>
#include "SurfByFunc.h"

#define SUBDIVISION_FACTOR 10.

namespace vivid
{

CMesh SurfByFunc(const F3D_t &func, int aBoxSize, quan_t aAlpha, const std::string &arLabel, bool RemoveFlatSurfaces) {
    // Creating Wireframe
    aBoxSize *= SUBDIVISION_FACTOR;
    vector<CPoint> points = {};
//    for (int i = -aBoxSize; i < aBoxSize; i += 1) {
//        for (int j = -aBoxSize; j < aBoxSize; j += 1) {
//            points.push_back(CPoint(i,i,0.));
//        }
//    }
    vector<quan_t> quan = {};
    quan_t temp_x, temp_y;

    for (int x = -aBoxSize; x <= aBoxSize; x += 1) {
        for (int y = -aBoxSize; y <= aBoxSize; y += 1) {
            temp_x = x/SUBDIVISION_FACTOR; temp_y = y/SUBDIVISION_FACTOR;
            quan_t z = func(temp_x, temp_y);
            if (z > 2*aBoxSize/SUBDIVISION_FACTOR) {
                z = 2*aBoxSize/SUBDIVISION_FACTOR;
            }
            else if (z < -2*aBoxSize/SUBDIVISION_FACTOR) {
                z = -2*aBoxSize/SUBDIVISION_FACTOR;
            }
            else if (isnan(z)) {
                z = 0;
            }
            CPoint point = {temp_x,z,temp_y};
            points.push_back(point);
            quan.push_back(z);
        }
    }

    for (int i = 0; i < points.size(); i++) {
        if (isnan(points[i].Y())){
            points[i] = CPoint(points[i].X(), 0, points[i].Z());
        }
    }
    quan_t v_max = *max_element(quan.begin(), quan.end());
    quan_t v_min = *min_element(quan.begin(), quan.end());
    if (v_max <= 0) {
        v_max = v_min;
        v_min = *max_element(quan.begin(), quan.end());
    }
    quan_t divide_by = 1./(v_max - v_min);

    for (int i = 0; i < points.size(); i++) {
        quan[i] = (quan[i] - v_min) * divide_by;
    }
    vector<CIndexedFace> faces;
    for (size_t i = 0; i < (2*aBoxSize)*(2*aBoxSize); i += aBoxSize*2+1) {
        for (size_t j = 0; j < 2*(aBoxSize); j += 1) {
            quan_t color = (quan[i+j] + quan[i+j+1] + quan[i+2*aBoxSize+j+1] + quan[i+2*aBoxSize+j]) / 4.;
            //            if (!(isinf(points[i+j].Y()) & isinf(points[i+j+1].Y()) & isinf(points[i+j+2+2*aBoxSize].Y()) & isinf(points[i+j+1+2*aBoxSize].Y()))) {
            //                faces.push_back(CIndexedFace({i + j, i + (j + 1), i + 2*aBoxSize + (j + 2), i + 2*aBoxSize + j + 1}, color));
            //                cout << i + j << " " << i + (j + 1) << " " << i + 2*aBoxSize + (j + 2) << " " << i + 2*aBoxSize + j + 1<< endl;
            //            }
            if (!(isnan(points[i+j].Y()) & isnan(points[i+j+1].Y()) & isnan(points[i+j+2+2*aBoxSize].Y()) & isnan(points[i+j+1+2*aBoxSize].Y()))) {
                if (RemoveFlatSurfaces) {
                    if (!(iszero(points[i+j].Y()) & iszero(points[i+j+1].Y()) & iszero(points[i+j+2+2*aBoxSize].Y()) & iszero(points[i+j+1+2*aBoxSize].Y()))) {
                        faces.push_back(CIndexedFace({i + j, i + (j + 1), i + 2*aBoxSize + (j + 2), i + 2*aBoxSize + j + 1}, color));
                    }
                } else {
                    faces.push_back(CIndexedFace({i + j, i + (j + 1), i + 2*aBoxSize + (j + 2), i + 2*aBoxSize + j + 1}, color));
                }
            }
        }
    }

    CMesh mesh = CMesh(points, faces, arLabel, aAlpha);
    return mesh;
}

CMesh ParametricSurface(const FParametric_t &func, int aNumberOfSteps, quan_t aThetaMin, quan_t aThetaMax,
                        quan_t aPhiMin, quan_t aPhiMax, quan_t aAlpha, const std::string &arLabel)
{
    vector<CPoint> points = {};
    vector<quan_t> quan = {};

    double phi_step = aPhiMax / aNumberOfSteps;
    double theta_step = aThetaMax / aNumberOfSteps;
    for (int i = 0; i <= aNumberOfSteps; i += 1) {
        quan_t theta = aThetaMin + i * theta_step;
        for (int j = 0; j <= aNumberOfSteps; j += 1) {
            quan_t phi = aPhiMin + j * phi_step;
            CPoint point = func(phi, theta);
            points.push_back(point);
            quan.push_back(point.Magnitude());
        }
    }

    quan_t v_max = *max_element(quan.begin(), quan.end());
    quan_t v_min = *min_element(quan.begin(), quan.end());

    quan_t divide_by = 1./(v_max - v_min);

    for (int i = 0; i < points.size(); i++) {
        quan[i] = (quan[i] - v_min) * divide_by;
    }
    vector<CIndexedFace> faces;

    /* Add rectangular faces around center */
    for (size_t i = 0; i < aNumberOfSteps; i++)
    {
        for (size_t j = 0; j < aNumberOfSteps; j++)
        {
            quan_t color = (quan[i + j * aNumberOfSteps] + quan[i + (j + 1) * aNumberOfSteps] +
                    quan[ i + 1 + (j + 1) * aNumberOfSteps] + quan[i + 1 + j * aNumberOfSteps]) / 4.;

            faces.push_back(CIndexedFace({i + j * aNumberOfSteps, i + (j + 1) * aNumberOfSteps,
                                          i + 1 + (j + 1) * aNumberOfSteps, i + 1 + j * aNumberOfSteps}, color));
        }
//        faces.push_back(CIndexedFace({i + (aNumberOfSteps - 1) * aNumberOfSteps,
//                                                    i, (i + 1),
//                                                    i + 1 + (aNumberOfSteps - 1) * aNumberOfSteps}, 0));
    }

    CMesh mesh = CMesh(points, faces, arLabel, aAlpha);
    return mesh;
}

} // namespace vivid