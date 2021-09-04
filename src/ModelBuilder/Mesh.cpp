#include "Mesh.h"
#include "Model.h" // TODO TOMER!! Y?
#include "Decimate.h"
#include "ProgressBar.h"
#include "boost/algorithm/string/predicate.hpp"
//#include <stdio.h>


using namespace vivid;
using namespace boost::algorithm;


// TODO: to place the methods in order and in substructure

CMesh::~CMesh() {}

/*-------------------------------------------------- Public Methods --------------------------------------------------*/

void CMesh::Reduce(quan_t aVerticlePercent, quan_t aMaxError)
{
    //check input valdilty
    if( aVerticlePercent < 0 || aVerticlePercent > 1){
//        if (mLogFile) (mLogFile)(CLogFile::ELogCode::LOG_ERROR, CLogFile::ELogMessage::INVALID_ALPHA_VALUE);
    }
    if( aMaxError < 0 || aMaxError > 1){
//        if (mLogFile) (mLogFile)(CLogFile::ELogCode::LOG_ERROR, CLogFile::ELogMessage::INVALID_ALPHA_VALUE);
    }

	//call decimation from External
	int targetVerticesN = int(aVerticlePercent * mPoints.size());
	int targetTrianglesN = int(aVerticlePercent * mFaces.size());
	auto temp = DecimateMesh(mPoints, GetFacesAsTriangles(), targetVerticesN, targetTrianglesN, aMaxError);
	mPoints = get<0>(temp);
	mFaces = get<1>(temp);
}



void CMesh::LaplacianSmooth(size_t aNumIterations, double aAlphaFactor, double aBetaFactor)
{
    vector<CPoint> curr_points = mPoints;

    for (int i = 0; i < aNumIterations; i++) {
        vector<CPoint> next_points (mPoints.size());
        vector<CPoint> curr_beta_points (mPoints.size());
        // Laplacian Smooth
        for (size_t j = 0; j < mPoints.size(); j++) {
            if (mPointNeighbours[j].size() != 0) {
                for (auto it = mPointNeighbours[j].begin(); it != mPointNeighbours[j].end(); it++) {
                    next_points[j] += curr_points[*it];
                }
                next_points[j] = next_points[j] / (mPointNeighbours[j].size());

                curr_beta_points[j] = next_points[j] - (mPoints[j] * aAlphaFactor + curr_points[j] * (1. - aAlphaFactor));
            } else {
                next_points[j] = mPoints [j];
            }
        }
        // HC Algorithm
        if (aBetaFactor > 0) {
            for (size_t j = 0; j < mPoints.size(); j++) {
                if (mPointNeighbours[j].size() != 0) {
                    CPoint next_beta;
                    for (auto it = mPointNeighbours[j].begin(); it != mPointNeighbours[j].end(); it++) {
                        next_beta += curr_beta_points[*it];
                    }
                    next_points[j] -= curr_beta_points[j] * aBetaFactor + next_beta*((1. - aBetaFactor) / mPointNeighbours[j].size());
                }
            }
        }

        curr_points = next_points;
    }
    mPoints = curr_points;
}

void CMesh::ExportToObj(string aOutput, bool WithTexture){
    CModel(*this).ExportToObj(aOutput, WithTexture); //TODO NAFTALI This is how it done.
}

void CMesh::CalculatePointsNeighbours() {
    for (int i = 0; i < mPoints.size(); i++) {
        printProgress(static_cast<double>(i)/mPoints.size());
        for (auto it = mFaces.begin(); it != mFaces.end(); it++) {
            vector<size_t> v = it->GetPoints();
            for (int j = 0; j < v.size();j++) {
                if (v[j] == i) {
                    // Face contains point i, so get one before and one after,
                    size_t prev, next;
                    if (j == 0) {
                        prev = v[v.size()-1];
                        next = v[j+1];
                    } else if (j == (v.size() -1)) {
                        prev = v[j-1];
                        next = v[0];
                    } else {
                        prev = v[j-1];
                        next = v[j+1];
                    }
                    if (!(std::find(mPointNeighbours[i].begin(), mPointNeighbours[i].end(), prev) != mPointNeighbours[i].end())) {
                        mPointNeighbours[i].push_back(prev);
                    }
                    if (!(std::find(mPointNeighbours[i].begin(), mPointNeighbours[i].end(), next) != mPointNeighbours[i].end())) {
                        mPointNeighbours[i].push_back(next);
                    }
                    break;
                }
            }
        }
    }
}
// TODO: SHOULD BE MOVED TO MODELCOMPONENT.CPP
/*--------------------------------------------- Transformation Methods -----------------------------------------------*/

void CMesh::TransformMesh(FTrans_t const aTrans){
    for (auto it = mPoints.begin(); it != mPoints.end(); it++)
    {
        *it = aTrans(*it);
    }
}

void CMesh::TransformMesh(quan_t const aTrans[3][3]){

    double px,py,pz;
    for (auto it = mPoints.begin(); it != mPoints.end(); it++)
    {

        px= it->X(); py= it->Y(); pz= it->Z();

        it->X(aTrans[0][0] * px + aTrans[0][1] * py + aTrans[0][2] * pz);
        it->Y(aTrans[1][0] * px + aTrans[1][1] * py + aTrans[1][2] * pz);
        it->Z(aTrans[2][0] * px + aTrans[2][1] * py + aTrans[2][2] * pz);

        // We should change all to vectorized operators.
        // May the god of compilers forgive us all for our sins.
    }
}

void CMesh::RotateMesh(CPoint aNormVec, double aRadAngel){
    // Trig operations are expensive
    auto cos_a = cos(aRadAngel);
    auto sin_a = sin(aRadAngel);
    // auto one_min_cos_a = 1-cos_a; for optimization it's better but it's less readable...
    auto nx = aNormVec.X();
    auto ny = aNormVec.Y();
    auto nz = aNormVec.Z();

    quan_t const rotation_mat[3][3] = {
            cos_a + nx*nx*(1-cos_a),        nx*ny*(1-cos_a) - nz*sin_a,     nx*nz*(1-cos_a) + ny*sin_a,

            ny*nx*(1-cos_a) + nz*sin_a,     cos_a + ny*ny*(1-cos_a),        ny*nz*(1-cos_a) - nx*sin_a,

            nz*nx*(1-cos_a) - ny*sin_a,     nz*ny*(1-cos_a) + nx*sin_a,     cos_a + nz*nz*(1-cos_a),
    };
    TransformMesh(rotation_mat);
}

void CMesh::MoveMesh(CPoint aDirectionVec){
//    auto x_movement = aDirectionVec.X();
//    auto y_movement = aDirectionVec.Y();
//    auto z_movement = aDirectionVec.Z();
    for (auto it = mPoints.begin(); it != mPoints.end(); it++)
    {
        *it += aDirectionVec;
//        it->X(x_movement + it->X());
//        it->Y(y_movement + it->Y());
//        it->Z(z_movement + it->Z());
        // The only reason I let it pass like that, is because this is an inline code.
        // We should change all to vectorized operators.
        // May the god of compilers forgive us all for our sins.
    }
}

void CMesh::ScaleMesh(CPoint aScaleVec){
    auto x_scale = aScaleVec.X();
    auto y_scale = aScaleVec.Y();
    auto z_scale = aScaleVec.Z();
    for (auto it = mPoints.begin(); it != mPoints.end(); it++)
    {
        //(*it).Scale(aScaleVec); // TODO: Why not this?
        it->X(x_scale * it->X());
        it->Y(y_scale * it->Y());
        it->Z(z_scale * it->Z());
        // The only reason I let it pass like that, is because this is an inline code.
        // We should change all to vectorized operators.
        // May the god of compilers forgive us all for our sins.
    }
}

/*------------------------------------------------- Private Methods --------------------------------------------------*/

vector<CIndexedFace> CMesh::GetFacesAsTriangles() {
    vector<CIndexedFace> aTriangles = vector<CIndexedFace>();
    for (auto fIt = mFaces.begin(); fIt != mFaces.end(); fIt++) {
        for (size_t i = 1; i < fIt->GetPoints().size()-1; i++) { // go over all the vertices from 1 to n-1 and connect them with vertice 0 to create triangles
            aTriangles.push_back(CIndexedFace((*fIt)[0], (*fIt)[i], (*fIt)[i + 1], fIt->GetColor()));
        }
    }
    return aTriangles;
}
